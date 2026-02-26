#include "../include/metrics.h"
#include "../include/utils.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "\033[?25l"; // Hide cursor
    
    CPUData prevCpu = getSysCPU();
    NetData prevNet = getPrimaryNet();
    long long prevEnergy = getCPUEnergyUj();
    
    // Refresh interval: 1s
    const int INTERVAL_MS = 1000; 
    
    while (true) {
        // Clear screen and cursor home
        cout << "\033[2J\033[H";
        
        // ------------------
        // CALCULATE METRICS
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
        // RENDER UI
        // ------------------
        cout << BOLD << CYAN << "=========================================================\n";
        cout << " 🚀 CLOUDCHEAP STATUS MONITOR | AUTO REFRESH: 1S\n";
        cout << "=========================================================\n" << RESET;
        
        // SYSTEM INFO BLOCK
        cout << BOLD << " [ SYSTEM ]\n" << RESET;
        cout << " OS:     " << sys.osName << " (" << sys.kernel << ")\n";
        cout << " Model:  " << (sys.machineModel.empty() || sys.machineModel == " " ? "Generic System" : sys.machineModel) << "\n";
        cout << " CPU:    " << sys.cpuModel << "\n";
        cout << " Uptime: " << sys.uptime << "\n";
        
        cout << CYAN << "---------------------------------------------------------\n" << RESET;

        // CPU BLOCK
        double temp = getCPUTemp();
        cout << BOLD << " [ CPU ]\n" << RESET;
        cout << " Usage: " << colorize(cpuPercent, 70, 90) << drawBar(cpuPercent) << " ";
        cout << fixed << setprecision(1) << cpuPercent << "%\t" << RESET << "| Cores: " << getCoreCount() << "\n";
        
        cout << " Temp:  ";
        if (temp > 0) cout << colorize(temp, 75, 85) << temp << "°C\t\t\t" << RESET;
        else cout << "N/A\t\t\t";
        
        cout << "| Load:  " << getLoadAvg() << "\n";

        cout << " Power: ";
        if (watts > 0) cout << colorize(watts, 100, 200) << watts << " W\n" << RESET;
        else cout << "N/A (RAPL/hwmon unsupported)\n";
        
        cout << CYAN << "---------------------------------------------------------\n" << RESET;

        // MEM BLOCK
        cout << BOLD << " [ RAM & SWAP ]\n" << RESET;
        cout << " RAM:  " << colorize(ramPercent, 80, 95) << drawBar(ramPercent) << " ";
        cout << (usedRAM/1024) << "MB / " << (mem.totalRAM/1024) << "MB (" << (int)ramPercent << "%)\n" << RESET;
        
        cout << " Swap: " << colorize(swapPercent, 1, 50) << drawBar(swapPercent) << " ";
        if (mem.totalSwap > 0)
            cout << ((mem.totalSwap - mem.freeSwap)/1024) << "MB / " << (mem.totalSwap/1024) << "MB (" << (int)swapPercent << "%)\n" << RESET;
        else
            cout << "0MB (Disabled)\n" << RESET;
            
        cout << CYAN << "---------------------------------------------------------\n" << RESET;

        // DISK BLOCK
        cout << BOLD << " [ DISK / ]\n" << RESET;
        cout << " Space: " << colorize(disk.percentUsed, 80, 95) << drawBar(disk.percentUsed) << " ";
        cout << disk.usedGB << "GB / " << disk.totalGB << "GB (" << (int)disk.percentUsed << "%)\n" << RESET;
        cout << " Inode: " << colorize(disk.inodePercentUsed, 80, 90) << (int)disk.inodePercentUsed << "% Used\n" << RESET;

        cout << CYAN << "---------------------------------------------------------\n" << RESET;

        // NET BLOCK
        cout << BOLD << " [ NETWORK (" << curNet.interface << ") ]\n" << RESET;
        cout << " RX In:  " << mbpsIn << " Mbps\t\t[ Loss: " << (curNet.rxDrop > 0 ? RED : GREEN) << curNet.rxDrop << RESET << " ]\n";
        cout << " TX Out: " << mbpsOut << " Mbps\t\t[ Err:  " << (curNet.rxErr > 0 ? RED : GREEN) << curNet.rxErr << RESET << " ]\n";

        cout << CYAN << "=========================================================\n" << RESET;
    }
    
    return 0;
}
