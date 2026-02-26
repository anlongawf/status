# CSMON: Cloudcheap Status Monitor

`csmon` is a high-performance, ultra-lightweight Terminal UI (TUI) monitoring tool designed specifically for game servers and bare-metal nodes. 

Instead of relying on heavy web stacks (like Node.js, Python, or Web Panels) that consume precious resources, `csmon` is written entirely in **C++11**. It directly reads hardware statistics from the Linux Kernel (`/proc` and `/sys`), guaranteeing an almost **0% CPU footprint** and consuming **less than 1MB of RAM**.

---

## 🚀 Key Features

*   **Real-Time Terminal UI**: Features a clean, color-coded, auto-refreshing interface drawn using standard ANSI escape codes, removing the need for `ncurses` dependencies.
*   **CPU Usage & Load Average**: Parses `/proc/stat` and `/proc/loadavg` to calculate live active time versus idle time, crucial for identifying game server overselling and preventing lag.
*   **Thermal Sensors**: Reads CPU temperatures directly from `/sys/class/thermal/thermal_zone*`.
*   **Power Consumption (Watts) ⚡**: Integrates deep hardware tracking by reading the `intel-rapl` interface (for Intel Xeon/Core) and `hwmon` energy inputs (for AMD Ryzen). It calculates power usage in real-time by interpreting microjoule differentials per second.
*   **RAM & Swap**: Parses `/proc/meminfo` to display available RAM and Swap depletion to proactively prevent Out-Of-Memory (OOM) crashes.
*   **Disk & Inode Health**: Uses `<sys/statvfs.h>` to monitor disk space on the root partition (`/`) and tracks Inode exhaustion—a common fault in environments with millions of small log files.
*   **Network Intelligence**: Reads `/proc/net/dev` to calculate live inbound (RX) and outbound (TX) bandwidth in Mbps. More importantly, it highlights `Drop` and `Err` packets to help warn administrators of potential network disruption or volumetric DDoS attacks.

---

## 📁 Source Code Structure

The project maintains a clean, scalable C++ architecture to allow for easy maintenance and future sensor additions (e.g., GPU tracking).

```text
status/
├── include/
│   ├── metrics.h         # Defines Data Structures (CPUData, MemData, DiskData, NetData)
│   └── utils.h           # ANSI color definitions and UI helper declarations
├── src/
│   ├── main.cpp          # Core loop (1-second sleep), orchestrator, and TUI renderer
│   ├── metrics.cpp       # Linux Kernel extraction logic (Filesystem, RAPL, procfs)
│   └── utils.cpp         # Implementations for the dynamic progress bars & colors
├── Makefile              # Build automation script
└── README.md             # This documentation
```

---

## 🛠️ Build & Installation

`csmon` is designed to be fully self-contained on any modern Linux system. 

### Prerequisites
*   A Linux environment (Ubuntu, CentOS, Debian, etc.)
*   `g++` (GCC Compiler)
*   `make`

### Installation Steps

1. **Clone the repository** to your server:
   ```bash
   git clone https://github.com/anlongawf/status.git
   cd status
   ```

2. **Compile the program** using the included Makefile. The Makefile applies the `-O3` optimization flag to ensure maximum runtime speed:
   ```bash
   make
   ```

3. **Global Installation (Optional but Recommended)**:
   You can install `csmon` globally into your Linux `/usr/local/bin` directory. This allows you to type `status` from anywhere to launch the monitor, similar to `htop`:
   ```bash
   sudo make install
   ```

4. **Run the Monitor**:
   If installed globally, simply type:
   ```bash
   status
   ```
   If not installed locally, run:
   ```bash
   ./csmon
   ```

*(Press `Ctrl+C` to cleanly exit the monitor).*

### How to Update
If a new version is released and you already have the repository cloned, you can update it by running:
```bash
cd status
git reset --hard HEAD
git pull origin main
sudo make clean
make
sudo make install
status
```

---

## 🧠 Under The Hood (Metrics Explained)

If you intend to modify `src/metrics.cpp`, here is how `csmon` retrieves data:

1. **Power (RAPL/hwmon)**: `getCPUEnergyUj()` loops through `/sys/class/powercap/intel-rapl/intel-rapl:0/energy_uj`. If unavailable, it falls back to `/sys/class/hwmon/hwmon*/energy1_input` to support newer AMD architectures.
2. **CPU Percentage**: `getSysCPU()` calculates the derivative of CPU ticks (`user + nice + system + irq + softirq + steal`) between 1-second intervals.
3. **Network Drops**: `getPrimaryNet()` scans all interfaces, dynamically selects the interface with the highest traffic (ignoring `lo` loopback), and parses the `rx_drop` column to visualize networking health.

---

## 🤝 Contributing

Contributions are welcome! If you want to add support for NVIDIA GPU tracking (`nvidia-smi` wrappers) or specific game-server process counts (e.g., matching PIDs for Minecraft/Rust servers), feel free to fork the repository and submit a Pull Request.
