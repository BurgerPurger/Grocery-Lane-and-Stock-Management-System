#ifndef COMMON_H
#define COMMON_H

#include <cctype>
#include <climits>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>
#include <vector>


namespace util {

using Money = long long;

struct Result {
    bool ok = true;
    std::string message;
    int id = 0;            
};
inline Result success(std::string msg = "", int id = 0) { return Result{true, std::move(msg), id}; }
inline Result failure(std::string msg) { return Result{false, std::move(msg), 0}; }

std::string trim(const std::string& s);
std::string toLower(std::string s);
std::vector<std::string> split(const std::string& s, char delim, size_t maxParts = 0);
std::string padRight(const std::string& s, size_t width);  
std::string padLeft(const std::string& s, size_t width);
bool parseInt(const std::string& s, long long& out);

std::string formatMoney(Money centavos);            
bool parseMoney(const std::string& s, Money& out);  

std::mutex& ioMutex();

bool inputClosed();
bool readLine(const std::string& prompt, std::string& out, bool allowEmpty = false);
bool readInt(const std::string& prompt, long long lo, long long hi, long long& out);
bool readMoney(const std::string& prompt, Money& out);
bool confirm(const std::string& prompt);

} 

namespace util {

static bool g_closed = false;

std::mutex& ioMutex() {
    static std::mutex m;
    return m;
}

std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::string toLower(std::string s) {
    for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::vector<std::string> split(const std::string& s, char delim, size_t maxParts) {
    std::vector<std::string> out;
    size_t start = 0;
    while (true) {
        if (maxParts != 0 && out.size() + 1 == maxParts) {  
            out.push_back(s.substr(start));
            break;
        }
        size_t p = s.find(delim, start);
        if (p == std::string::npos) {
            out.push_back(s.substr(start));
            break;
        }
        out.push_back(s.substr(start, p - start));
        start = p + 1;
    }
    return out;
}

std::string padRight(const std::string& s, size_t width) {
    if (s.size() > width) return width >= 2 ? s.substr(0, width - 2) + ".." : s.substr(0, width);
    return s + std::string(width - s.size(), ' ');
}

std::string padLeft(const std::string& s, size_t width) {
    if (s.size() >= width) return s;
    return std::string(width - s.size(), ' ') + s;
}

bool parseInt(const std::string& s, long long& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    try {
        size_t pos = 0;
        long long v = std::stoll(t, &pos);
        if (pos != t.size()) return false;
        out = v;
        return true;
    } catch (...) {
        return false;
    }
}

std::string formatMoney(Money c) {
    bool neg = c < 0;
    unsigned long long a = neg ? static_cast<unsigned long long>(-(c + 1)) + 1ULL
                               : static_cast<unsigned long long>(c);
    std::string whole = std::to_string(a / 100);
    std::string grouped;
    int count = 0;
    for (int i = static_cast<int>(whole.size()) - 1; i >= 0; --i) {
        grouped.insert(grouped.begin(), whole[static_cast<size_t>(i)]);
        if (++count % 3 == 0 && i > 0) grouped.insert(grouped.begin(), ',');
    }
    char buf[8];
    std::snprintf(buf, sizeof buf, ".%02d", static_cast<int>(a % 100));
    return (neg ? "-" : "") + grouped + buf;
}

bool parseMoney(const std::string& in, Money& out) {
    std::string s = trim(in);
    if (s.empty()) return false;
    size_t dot = s.find('.');
    std::string whole = s.substr(0, dot);
    std::string frac = dot == std::string::npos ? "" : s.substr(dot + 1);
    if (whole.empty() && frac.empty()) return false;
    if (whole.size() > 9 || frac.size() > 2) return false;
    for (char c : whole) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    for (char c : frac) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    while (frac.size() < 2) frac += '0';
    Money w = whole.empty() ? 0 : std::stoll(whole);
    out = w * 100 + std::stoll(frac);
    return true;
}

bool inputClosed() { return g_closed; }

static bool isCancel(const std::string& s) {
    std::string l = toLower(s);
    return l == "/c" || l == "cancel";
}

bool readLine(const std::string& prompt, std::string& out, bool allowEmpty) {
    while (true) {
        std::cout << prompt << std::flush;
        std::string line;
        if (!std::getline(std::cin, line)) {
            g_closed = true;
            std::cout << "\n";
            return false;
        }
        line = trim(line);
        if (isCancel(line)) {
            std::cout << "  (cancelled)\n";
            return false;
        }
        if (line.empty() && !allowEmpty) {
            std::cout << "  Input cannot be empty.\n";
            continue;
        }
        out = line;
        return true;
    }
}

bool readInt(const std::string& prompt, long long lo, long long hi, long long& out) {
    while (true) {
        std::string s;
        if (!readLine(prompt, s)) return false;
        long long v;
        if (!parseInt(s, v) || v < lo || v > hi) {
            std::cout << "  Please enter a whole number from " << lo << " to " << hi << ".\n";
            continue;
        }
        out = v;
        return true;
    }
}

bool readMoney(const std::string& prompt, Money& out) {
    while (true) {
        std::string s;
        if (!readLine(prompt, s)) return false;
        if (!parseMoney(s, out)) {
            std::cout << "  Enter an amount such as 25 or 25.50 (at most 2 decimals).\n";
            continue;
        }
        return true;
    }
}

bool confirm(const std::string& prompt) {
    std::string s;
    if (!readLine(prompt + " (y/n): ", s)) return false;
    s = toLower(s);
    return s == "y" || s == "yes";
}

} 

#endif
