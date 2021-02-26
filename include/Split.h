#pragma once

#include <bits/stdc++.h>

struct Split {
    std::vector<std::vector<Split*>> branches = std::vector<std::vector<Split*>>(2);
    float pt;
    bool axis;
};
