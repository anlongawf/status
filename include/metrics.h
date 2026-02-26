#ifndef METRICS_H
#define METRICS_H

#include <string>

// System Info Data Structure
struct SysInfo {
    std::string osName;
    std::string kernel;
    std::string uptime;
    std::string cpuModel;
    std::string machineModel;
};

// CPU Data Structure
struct CPUData {
    long long activeTime;
    long long totalTime;
};

// Memory Data Structure
struct MemData {
    long long totalRAM, freeRAM, buffers, cached, sReclaimable;
    long long totalSwap, freeSwap;
};

// Disk Data Structure
struct DiskData {
    double totalGB, usedGB;
    double percentUsed;
    double inodePercentUsed;
};

// Network Data Structure
struct NetData {
    std::string interface;
    long long rxBytes, txBytes, rxDrop, rxErr;
};

// Metric Functions
CPUData getSysCPU();
int getCoreCount();
std::string getLoadAvg();
double getCPUTemp();
long long getCPUEnergyUj();

MemData getMemInfo();
DiskData getRootDisk();
NetData getPrimaryNet();
SysInfo getSysInfo();

#endif // METRICS_H
