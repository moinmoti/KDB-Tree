#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <math.h>
#include <map>
#include <queue>
#include <sstream>
#include <stack>
#include <string.h>
#include <vector>

using namespace std;
using namespace chrono;

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1> void __f(const char *name, Arg1 &&arg1) {
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char *names, Arg1 &&arg1, Args &&... args) {
    const char *comma = strchr(names + 1, ',');
    cerr.write(names, comma - names) << " : " << setw(9) << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

#define all(c) c.begin(), c.end()
#define remove(container, element) container.erase(find(all(container), element))
constexpr uint D = 2;
constexpr double dist(float x1, float y1, float x2, float y2) {
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}
constexpr double distManhattan(float x1, float y1, float x2, float y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}
constexpr uint oppDir(uint d) {return (d + D) % (D * 2);};

enum SplitType {Cyclic, Spread};

struct Record {
    int id;
    array<float, 2>  data;
};

struct Info {
    int cost = 0;
    int output = 0;

    Info operator+(const Info &i) {
        return Info{cost+i.cost, output+i.output};
    }
    void operator+=(const Info &i) {
        *this = *this + i;
    }
};
