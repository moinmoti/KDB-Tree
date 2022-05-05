#pragma once

#include "common.h"

struct Node {

    struct Split {
        static SplitType type;
        float pt;
        bool axis;
    };

    int height;
    bool splitDim;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Directory node specific members
    optional<vector<Node*>> contents;

    // Page node specific members
    optional<vector<Record>> points;

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    bool overlap(array<float, 4>) const;

    vector<Record> getPoints() const;
    Split* getSplit() const;
    int scan(array<float, 4>) const;
    int size() const;
    vector<Node*> splitDirectory(int &, Split * = NULL);
    vector<Node*> splitPage(Split * = NULL);

    ~Node();
};
