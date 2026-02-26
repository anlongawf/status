#include "../include/metrics.h"
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <iostream>
#include <vector>

using namespace std;

SysInfo getSysInfo() {
    SysInfo info = {"Unknown OS", "Unknown Kernel", "0m", "Unknown CPU", "Unknown Model"};
    
    // 1. Get OS Name
    ifstream fpos("/etc/os-release");
    if (fpos.is_open()) {
        string line;
        while (getline(fpos, line)) {
            if (line.find("PRETTY_NAME=") == 0) {
                info.osName = line.substr(12);
                if (info.osName.front() == '"' && info.osName.back() == '"') {
                     info.osName = info.osName.substr(1, info.osName.size() - 2);
                }
                break;
            }
        }
    }
    
    // 2. Get Kernel
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        info.kernel = buffer.release;
    }
    
    // 3. Get Uptime
    ifstream fput("/proc/uptime");
    if (fput.is_open()) {
        double uptimeSec;
        if (fput >> uptimeSec) {
            int days = uptimeSec / 86400;
            int hours = ((int)uptimeSec % 86400) / 3600;
            int mins = ((int)uptimeSec % 3600) / 60;
            info.uptime = to_string(days) + "d " + to_string(hours) + "h " + to_string(mins) + "m";
        }
    }
    
    // 4. Get CPU Model
    ifstream fpcpu("/proc/cpuinfo");
    if (fpcpu.is_open()) {
        string line;
        while (getline(fpcpu, line)) {
            if (line.find("model name") == 0) {
                size_t pos = line.find(":");
                if (pos != string::npos) {
                    info.cpuModel = line.substr(pos + 2);
                    break;
                }
            }
        }
    }
    
    // 5. Get Machine Model (DMI)
    ifstream fpvendor("/sys/class/dmi/id/sys_vendor");
    ifstream fpproduct("/sys/class/dmi/id/product_name");
    string vendor = "", product = "";
    if (fpvendor.is_open()) getline(fpvendor, vendor);
    if (fpproduct.is_open()) getline(fpproduct, product);
    
    if (!vendor.empty() || !product.empty()) {
        info.machineModel = vendor + (vendor.empty() ? "" : " ") + product;
    }

    return info;
}

using namespace std;

CPUData getSysCPU() {
    ifstream fpstat("/proc/stat");
    string line;
    getline(fpstat, line);
    istringstream iss(line);
    string cpuLabel;
    long long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;
    iss >> cpuLabel >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal >> guest >> guest_nice;
    
    long long idleTime = idle + iowait;
    long long nonIdleTime = user + nice + system + irq + softirq + steal;
    long long totalTime = idleTime + nonIdleTime;
    
    return {nonIdleTime, totalTime};
}

int getCoreCount() {
    return sysconf(_SC_NPROCESSORS_ONLN);
}

string getLoadAvg() {
    ifstream fpload("/proc/loadavg");
    string load1, load5, load15;
    if (fpload.is_open()) {
        fpload >> load1 >> load5 >> load15;
        return load1 + " " + load5 + " " + load15;
    }
    return "N/A";
}

double getCPUTemp() {
    // Attempt to read from thermal zones
    for (int i = 0; i < 5; ++i) {
        ifstream fp("/sys/class/thermal/thermal_zone" + to_string(i) + "/temp");
        if (fp.is_open()) {
            long long tempMillic;
            if (fp >> tempMillic) {
                if (tempMillic > 1000) return tempMillic / 1000.0;
                return tempMillic;
            }
        }
    }
    return -1.0; // Not available
}

long long getCPUEnergyUj() {
    long long totalUj = 0;
    bool foundIntel = false;
    
    // Support Dual/Quad CPU Sockets: Loop through intel-rapl:0, intel-rapl:1, etc.
    for (int i = 0; i < 4; ++i) {
        ifstream fp("/sys/class/powercap/intel-rapl/intel-rapl:" + to_string(i) + "/energy_uj");
        if (fp.is_open()) {
            long long uj = 0;
            if (fp >> uj) {
                totalUj += uj;
                foundIntel = true;
            }
        }
    }
    
    if (foundIntel) {
        return totalUj;
    }
    
    // Deeper Fallback for AMD EPYC / Ryzen using hwmon
    // Some systems expose power in hwmonX/energy1_input, energy2_input, etc.
    long long amdTotalUj = 0;
    bool foundAmd = false;
    for (int i = 0; i < 10; ++i) { // Check up to 10 hwmon devices
        for (int j = 1; j <= 4; ++j) { // Check energy1_input through energy4_input
            string path = "/sys/class/hwmon/hwmon" + to_string(i) + "/energy" + to_string(j) + "_input";
            ifstream fp_hwmon(path);
            if (fp_hwmon.is_open()) {
                long long uj = 0;
                if (fp_hwmon >> uj) {
                    amdTotalUj += uj;
                    foundAmd = true;
                }
            }
        }
    }
    
    if (foundAmd) return amdTotalUj;
    
    // Some EPYC systems put it directly in powercap/amd_energy
    for (int i = 0; i < 4; ++i) {
        string path = "/sys/class/powercap/amd_energy/amd_energy:" + to_string(i) + "/energy_uj";
        ifstream fp_amd(path);
        if (fp_amd.is_open()) {
            long long uj = 0;
            if (fp_amd >> uj) {
                amdTotalUj += uj;
                foundAmd = true;
            }
        }
    }
    
    if (foundAmd) return amdTotalUj;

    return 0; // Unsupported
}

MemData getMemInfo() {
    MemData data = {0};
    ifstream fpmem("/proc/meminfo");
    string name;
    long long value;
    string unit;
    
    while (fpmem >> name >> value >> unit) {
        if (name == "MemTotal:") data.totalRAM = value;
        else if (name == "MemFree:") data.freeRAM = value;
        else if (name == "Buffers:") data.buffers = value;
        else if (name == "Cached:") data.cached = value;
        else if (name == "SReclaimable:") data.sReclaimable = value;
        else if (name == "SwapTotal:") data.totalSwap = value;
        else if (name == "SwapFree:") data.freeSwap = value;
    }
    return data;
}

DiskData getRootDisk() {
    DiskData data = {0};
    struct statvfs stat;
    if (statvfs("/", &stat) == 0) {
        data.totalGB = (double)(stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024);
        double freeGB = (double)(stat.f_bfree * stat.f_frsize) / (1024 * 1024 * 1024);
        data.usedGB = data.totalGB - freeGB;
        if (data.totalGB > 0) {
            data.percentUsed = (data.usedGB / data.totalGB) * 100.0;
        }
        
        double totalInodes = stat.f_files;
        double freeInodes = stat.f_ffree;
        if (totalInodes > 0) {
            data.inodePercentUsed = ((totalInodes - freeInodes) / totalInodes) * 100.0;
        }
    }
    return data;
}

NetData getPrimaryNet() {
    NetData data = {"N/A", 0, 0, 0, 0};
    ifstream fpnet("/proc/net/dev");
    string line;
    // Skip 2 header lines
    getline(fpnet, line); getline(fpnet, line);
    
    long long maxRx = -1;
    while (getline(fpnet, line)) {
        istringstream iss(line);
        string iface;
        iss >> iface;
        if (iface == "lo:") continue; // Skip loopback
        
        long long rxB, rxP, rxE, rxD, rxF, rxFrame, rxComp, rxMulti;
        long long txB, txP, txE, txD, txCol, txCarr, txComp;
        
        if (iss >> rxB >> rxP >> rxE >> rxD >> rxF >> rxFrame >> rxComp >> rxMulti
                >> txB >> txP >> txE >> txD >> txCol >> txCarr >> txComp) {
            if (rxB > maxRx) { // Simple heuristic: choose interface with most traffic
                maxRx = rxB;
                data.interface = iface.substr(0, iface.size()-1);
                data.rxBytes = rxB;
                data.txBytes = txB;
                data.rxDrop = rxD;
                data.rxErr = rxE;
            }
        }
    }
    return data;
}
