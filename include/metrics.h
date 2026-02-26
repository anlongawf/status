#ifndef METRICS_H
#define METRICS_H

#include <string>

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

#endif // METRICS_H
