#include "utils.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

std::string colorize(double value, double warn, double danger, bool invert) {
    if (invert) {
        if (value >= danger) return GREEN;
        if (value >= warn) return YELLOW;
        return RED;
    } else {
        if (value >= danger) return RED;
        if (value >= warn) return YELLOW;
        return GREEN;
    }
}

std::string drawBar(double percent, int width) {
    if (width < 5) width = 5; // Minimum viable width
    int filled = (percent / 100.0) * width;
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) bar += "|";
        else bar += ".";
    }
    bar += "]";
    return bar;
}

int getTerminalWidth() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80; // Fallback width
}

std::string drawSparkline(const std::deque<double>& values, double max_val) {
    if (values.empty()) return "";
    
    // Unicode blocks:  _,  , ▂, ▃, ▄, ▅, ▆, ▇, █
    const std::vector<std::string> blocks = {" ", " ", "▂", "▃", "▄", "▅", "▆", "▇", "█"};
    
    // If input max_val is 0 or negative, automatically scale based on current values
    if (max_val <= 0.0) {
        max_val = 0.001; // Avoid div by zero
        for (double v : values) {
            if (v > max_val) max_val = v;
        }
    }

    std::string sparkline = "";
    for (double v : values) {
        if (v < 0) v = 0;
        int idx = (int)((v / max_val) * (blocks.size() - 1));
        if (idx < 0) idx = 0;
        if (idx >= blocks.size()) idx = blocks.size() - 1;
        
        // Add minimal colorizing to sparkline based on relative height
        if (idx >= 6) sparkline += RED + blocks[idx] + RESET;
        else if (idx >= 4) sparkline += YELLOW + blocks[idx] + RESET;
        else sparkline += GREEN + blocks[idx] + RESET;
    }
    
    return sparkline;
}

