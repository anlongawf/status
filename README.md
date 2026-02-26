# CSMON (Cloudcheap Status Monitor)

A lightweight, real-time C++ terminal UI monitoring tool designed specifically for game server nodes. It provides critical hardware statistics directly in the terminal without relying on heavy web panels or intermediate agents.

## Features

- **Extreme Performance**: Written entirely in C++ reading directly from Linux Kernel (`/proc` and `/sys`), consuming almost 0% CPU and `< 1MB` RAM.
- **CPU Monitoring**: Tracks real-time usage (%), Load Average (1m/5m/15m) to prevent overselling, and thermal sensor reading (°C).
- **Power Consumption (Watts)**: Unique feature tracking live hardware energy usage by parsing Intel RAPL and AMD Ryzen `hwmon` microjoule sensors.
- **Memory & Swap**: Visualizes RAM usage and swap depletion (crucial for game server crash prevention).
- **Disk & Inodes**: Tracks not just disk space availability, but also Inode health.
- **Network Stats**: Accurate inbound/outbound traffic calculation (Mbps) and packet drop detection (great for basic DDoS awareness).

## Directory Structure

```text
status/ (or csmon/)
├── include/                 # Header files for variables and structs
│   ├── metrics.h            # Structures (CPUData, NetData, DiskData)
│   └── utils.h              # Terminal ANSI color and UI drawing functions
├── src/                     # Source implementations
│   ├── main.cpp             # Refresh loop and TUI rendering core
│   ├── metrics.cpp          # Linux kernel extraction logic
│   └── utils.cpp            # Aesthetic helpers
├── Makefile                 # Easy 1-click build mechanism
└── README.md
```

## Setup & Installation

**Prerequisites:** Ubuntu/CentOS/Debian with `g++` installed.

1. Clone the repository to your node:
   ```bash
   git clone git@github.com:anlongawf/status.git csmon
   cd csmon
   ```

2. Compile the source code using the provided Makefile:
   ```bash
   make
   ```

3. Run the monitor (updates every 1 second):
   ```bash
   ./csmon
   ```

> **Note on Power Sensor (Watts)**: In order to fetch accurate server wattage, `csmon` must be run on physical hardware or a hypervisor that passes through RAPL/hwmon architectures (usually bare-metal nodes or dedicated VMs with exposed powercaps). If unsupported, it will gracefully show `N/A`.

## Contributing
Feel free to open an issue or submit a Pull Request if you'd like to extend the monitor's capabilities (e.g., adding GPU stats or custom process tracking loops).
