#include "utils.h"

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

std::string drawBar(double percent) {
    int width = 20;
    int filled = (percent / 100.0) * width;
    std::string bar = "[";
    for (int i = 0; i < width; ++i) {
        if (i < filled) bar += "|";
        else bar += ".";
    }
    bar += "]";
    return bar;
}
