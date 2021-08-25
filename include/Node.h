#pragma once

#include "Split.h"
#include "common.h"

class Node {

public:
    int height;
    bool splitDim;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Directory node specific members
    optional<vector<Node*>> contents;

    // Page node specific members
    optional<vector<array<float, 2>>> points;

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    bool overlap(array<float, 4>) const;

    vector<Node*> cascadingSplit(Split*, vector<Node*> = vector<Node*>());
    vector<array<float, 2>> getPoints() const;
    Split* getSplit() const;
    int scan(array<float, 4>) const;
    int size() const;
    vector<Node*> splitDirectory(Split * = NULL);
    vector<Node*> splitPage(Split * = NULL);

    ~Node();
};
