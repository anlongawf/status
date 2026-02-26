#include "../include/metrics.h"
#include "../include/utils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <sstream>

using namespace std;

int main() {
    cout << "\033[?25l"; // Hide cursor
    
    CPUData prevCpu = getSysCPU();
    NetData prevNet = getPrimaryNet();
    long long prevEnergy = getCPUEnergyUj();
    
    // Refresh interval: 1s
    const int INTERVAL_MS = 1000; 
    
    // Clear screen ONCE at the start
    cout << "\033[2J";
    
    while (true) {
        // ------------------
        // WAIT & CALCULATE
        // ------------------
        this_thread::sleep_for(chrono::milliseconds(INTERVAL_MS));
        
        // CPU Usage
        CPUData curCpu = getSysCPU();
        double activeDelta = curCpu.activeTime - prevCpu.activeTime;
        double totalDelta = curCpu.totalTime - prevCpu.totalTime;
        double cpuPercent = 100.0 * (activeDelta / totalDelta);
        prevCpu = curCpu;
        
        // Power (Watts)
        long long curEnergy = getCPUEnergyUj();
        double watts = 0.0;
        if (prevEnergy > 0 && curEnergy >= prevEnergy) {
            watts = ((curEnergy - prevEnergy) / 1000000.0) / (INTERVAL_MS / 1000.0);
        }
        prevEnergy = curEnergy;

        // RAM
        MemData mem = getMemInfo();
        long long usedRAM = mem.totalRAM - mem.freeRAM - mem.buffers - mem.cached - mem.sReclaimable;
        double ramPercent = 0.0;
        if (mem.totalRAM > 0) ramPercent = (double)usedRAM / mem.totalRAM * 100.0;
        
        double swapPercent = (mem.totalSwap > 0) ? (double)(mem.totalSwap - mem.freeSwap) / mem.totalSwap * 100.0 : 0.0;

        // Disk
        DiskData disk = getRootDisk();

        // Network speed
        NetData curNet = getPrimaryNet();
        double mbpsIn = ((curNet.rxBytes - prevNet.rxBytes) * 8.0 / 1000000.0) / (INTERVAL_MS / 1000.0);
        double mbpsOut = ((curNet.txBytes - prevNet.txBytes) * 8.0 / 1000000.0) / (INTERVAL_MS / 1000.0);
        prevNet = curNet;

        // SysInfo
        SysInfo sys = getSysInfo();

        // ------------------
        // RENDER UI TO BUFFER
        // ------------------
        stringstream ss;
        // Move cursor top-left
        ss << "\033[H";
        
        ss << BOLD << CYAN << "=========================================================\033[K\n";
        ss << " 🚀 CLOUDCHEAP STATUS MONITOR | AUTO REFRESH: 1S\033[K\n";
        ss << "=========================================================\033[K\n" << RESET;
        
        // SYSTEM INFO BLOCK
        ss << BOLD << " [ SYSTEM ]\033[K\n" << RESET;
        ss << " OS:     " << sys.osName << " (" << sys.kernel << ")\033[K\n";
        ss << " Model:  " << (sys.machineModel.empty() || sys.machineModel == " " ? "Generic System" : sys.machineModel) << "\033[K\n";
        ss << " CPU:    " << sys.cpuModel << "\033[K\n";
        ss << " Uptime: " << sys.uptime << "\033[K\n";
        
        ss << CYAN << "---------------------------------------------------------\033[K\n" << RESET;

        // CPU BLOCK
        double temp = getCPUTemp();
        ss << BOLD << " [ CPU ]\033[K\n" << RESET;
        ss << " Usage: " << colorize(cpuPercent, 70, 90) << drawBar(cpuPercent) << " ";
        ss << fixed << setprecision(1) << cpuPercent << "%\t" << RESET << "| Cores: " << getCoreCount() << "\033[K\n";
        
        ss << " Temp:  ";
        if (temp > 0) ss << colorize(temp, 75, 85) << temp << "°C\t\t\t" << RESET;
        else ss << "N/A\t\t\t";
        
        ss << "| Load:  " << getLoadAvg() << "\033[K\n";

        ss << " Power: ";
        if (watts > 0) ss << colorize(watts, 100, 200) << watts << " W\033[K\n" << RESET;
        else ss << "N/A (RAPL/hwmon unsupported)\033[K\n";
        
        ss << CYAN << "---------------------------------------------------------\033[K\n" << RESET;

        // MEM BLOCK
        ss << BOLD << " [ RAM & SWAP ]\033[K\n" << RESET;
        ss << " RAM:  " << colorize(ramPercent, 80, 95) << drawBar(ramPercent) << " ";
        ss << (usedRAM/1024) << "MB / " << (mem.totalRAM/1024) << "MB (" << (int)ramPercent << "%)\033[K\n" << RESET;
        
        ss << " Swap: " << colorize(swapPercent, 1, 50) << drawBar(swapPercent) << " ";
        if (mem.totalSwap > 0)
            ss << ((mem.totalSwap - mem.freeSwap)/1024) << "MB / " << (mem.totalSwap/1024) << "MB (" << (int)swapPercent << "%)\033[K\n" << RESET;
        else
            ss << "0MB (Disabled)\033[K\n" << RESET;
            
        ss << CYAN << "---------------------------------------------------------\033[K\n" << RESET;

        // DISK BLOCK
        ss << BOLD << " [ DISK / ]\033[K\n" << RESET;
        ss << " Space: " << colorize(disk.percentUsed, 80, 95) << drawBar(disk.percentUsed) << " ";
        ss << disk.usedGB << "GB / " << disk.totalGB << "GB (" << (int)disk.percentUsed << "%)\033[K\n" << RESET;
        ss << " Inode: " << colorize(disk.inodePercentUsed, 80, 90) << (int)disk.inodePercentUsed << "% Used\033[K\n" << RESET;

        ss << CYAN << "---------------------------------------------------------\033[K\n" << RESET;

        // NET BLOCK
        ss << BOLD << " [ NETWORK (" << curNet.interface << ") ]\033[K\n" << RESET;
        ss << " RX In:  " << mbpsIn << " Mbps\t\t[ Loss: " << (curNet.rxDrop > 0 ? RED : GREEN) << curNet.rxDrop << RESET << " ]\033[K\n";
        ss << " TX Out: " << mbpsOut << " Mbps\t\t[ Err:  " << (curNet.rxErr > 0 ? RED : GREEN) << curNet.rxErr << RESET << " ]\033[K\n";

        ss << CYAN << "=========================================================\033[K\n" << RESET;
        
        // Print everything instantly and flush to trigger visual update
        cout << ss.str() << flush;
    }
    
    return 0;
}
