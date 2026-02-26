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
    cout << "\033[2J\033[H";
    bool is_first_run = true;
    int previous_lines = 0;
    
    // Sparkline history buffers
    std::deque<double> cpu_history;
    std::deque<double> rx_history;
    std::deque<double> tx_history;
    const int HISTORY_LIMIT = 20;
    
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
        
        cpu_history.push_back(cpuPercent);
        if (cpu_history.size() > HISTORY_LIMIT) cpu_history.pop_front();
        
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
        
        rx_history.push_back(mbpsIn);
        if (rx_history.size() > HISTORY_LIMIT) rx_history.pop_front();
        tx_history.push_back(mbpsOut);
        if (tx_history.size() > HISTORY_LIMIT) tx_history.pop_front();

        // SysInfo
        SysInfo sys = getSysInfo();
        
        // Terminal width for layout
        int termWidth = getTerminalWidth();
        if (termWidth < 60) termWidth = 60; // Minimum reasonable size
        int barWidth = termWidth / 3; // Allocate 33% of screen to progress bars
        
        std::string separator = "";
        for (int i=0; i < termWidth - 4; ++i) separator += "=";
        std::string thin_separator = "";
        for (int i=0; i < termWidth - 4; ++i) thin_separator += "-";

        // ------------------
        // RENDER UI TO BUFFER
        // ------------------
        stringstream ss;
        if (is_first_run) {
            is_first_run = false;
        } else {
            // Move cursor Up cleanly based on the previous frame size
            ss << "\033[" << previous_lines << "A";
        }
        
        ss << BOLD << CYAN << separator << "\033[K\n";
        ss << " 🚀 CLOUDCHEAP STATUS MONITOR | AUTO REFRESH: 1S\033[K\n";
        ss << separator << "\033[K\n" << RESET;
        
        // SYSTEM INFO BLOCK
        ss << BOLD << " [ SYSTEM ]\033[K\n" << RESET;
        ss << " OS:     " << sys.osName << " (" << sys.kernel << ")\033[K\n";
        ss << " Model:  " << (sys.machineModel.empty() || sys.machineModel == " " ? "Generic System" : sys.machineModel) << "\033[K\n";
        ss << " CPU:    " << sys.cpuModel << "\033[K\n";
        ss << " Uptime: " << sys.uptime << "\033[K\n";
        
        ss << CYAN << thin_separator << "\033[K\n" << RESET;

        // CPU BLOCK
        double temp = getCPUTemp();
        ss << BOLD << " [ CPU ]\033[K\n" << RESET;
        ss << " Usage: " << colorize(cpuPercent, 70, 90) << drawBar(cpuPercent, barWidth) << " ";
        ss << fixed << setprecision(1) << cpuPercent << "%  " << RESET << "| Cores: " << getCoreCount();
        ss << "  [ " << drawSparkline(cpu_history, 100.0) << " ]\033[K\n";
        
        ss << " Temp:  ";
        if (temp > 0) ss << colorize(temp, 75, 85) << temp << "°C\t\t\t" << RESET;
        else ss << "N/A\t\t\t";
        
        ss << "| Load:  " << getLoadAvg() << "\033[K\n";

        ss << " Power: ";
        if (watts > 0) ss << colorize(watts, 100, 200) << watts << " W\033[K\n" << RESET;
        else ss << "N/A (No RAPL Kernel Module / IPMI Required)\033[K\n";
        
        ss << CYAN << thin_separator << "\033[K\n" << RESET;

        // MEM BLOCK
        ss << BOLD << " [ RAM & SWAP ]\033[K\n" << RESET;
        ss << " RAM:  " << colorize(ramPercent, 80, 95) << drawBar(ramPercent, barWidth) << " ";
        ss << (usedRAM/1024) << "MB / " << (mem.totalRAM/1024) << "MB (" << (int)ramPercent << "%)\033[K\n" << RESET;
        
        ss << " Swap: " << colorize(swapPercent, 1, 50) << drawBar(swapPercent, barWidth) << " ";
        if (mem.totalSwap > 0)
            ss << ((mem.totalSwap - mem.freeSwap)/1024) << "MB / " << (mem.totalSwap/1024) << "MB (" << (int)swapPercent << "%)\033[K\n" << RESET;
        else
            ss << "0MB (Disabled)\033[K\n" << RESET;
            
        ss << CYAN << thin_separator << "\033[K\n" << RESET;

        // DISK BLOCK
        ss << BOLD << " [ DISK / ]\033[K\n" << RESET;
        ss << " Space: " << colorize(disk.percentUsed, 80, 95) << drawBar(disk.percentUsed, barWidth) << " ";
        ss << disk.usedGB << "GB / " << disk.totalGB << "GB (" << (int)disk.percentUsed << "%)\033[K\n" << RESET;
        ss << " Inode: " << colorize(disk.inodePercentUsed, 80, 90) << (int)disk.inodePercentUsed << "% Used\033[K\n" << RESET;

        ss << CYAN << thin_separator << "\033[K\n" << RESET;

        // NET BLOCK
        ss << BOLD << " [ NETWORK (" << curNet.interface << ") ]\033[K\n" << RESET;
        ss << " RX In:  " << mbpsIn << " Mbps\t[ " << drawSparkline(rx_history, -1.0) << " ]\t[ Loss: " << (curNet.rxDrop > 0 ? RED : GREEN) << curNet.rxDrop << RESET << " ]\033[K\n";
        ss << " TX Out: " << mbpsOut << " Mbps\t[ " << drawSparkline(tx_history, -1.0) << " ]\t[ Err:  " << (curNet.rxErr > 0 ? RED : GREEN) << curNet.rxErr << RESET << " ]\033[K\n";

        ss << CYAN << separator << "\033[K\n" << RESET;
        
        // Count number of lines printed to correctly backtrack next frame
        std::string bufferStr = ss.str();
        previous_lines = std::count(bufferStr.begin(), bufferStr.end(), '\n');
        
        // Print everything instantly and flush to trigger visual update
        cout << bufferStr << flush;
    }
    
    return 0;
}
