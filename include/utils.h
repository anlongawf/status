#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
#include <deque>

// ANSI Color Codes
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"

std::string colorize(double value, double warn, double danger, bool invert = false);
std::string drawBar(double percent, int width = 20);

int getTerminalWidth();
std::string drawSparkline(const std::deque<double>& values, double max_val = 100.0);

#endif // UTILS_H
