#include "../include/metrics.h"
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>
#include <unistd.h>
#include <iostream>

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
    // Reads from intel RAPL interface (Works for both Intel Xeon and AMD Ryzen on modern kernels)
    ifstream fp("/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj");
    long long uj = 0;
    if (fp.is_open()) {
        fp >> uj;
    } else {
        // Fallback for some AMD systems using hwmon
        for (int i = 0; i < 5; ++i) {
            ifstream fp_hwmon("/sys/class/hwmon/hwmon" + to_string(i) + "/energy1_input");
            if (fp_hwmon.is_open()) {
                if (fp_hwmon >> uj) {
                    return uj; // Found hwmon energy_uj
                }
            }
        }
    }
    return uj;
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
