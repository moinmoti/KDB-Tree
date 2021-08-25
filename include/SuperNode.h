#pragma once

#include <bits/stdc++.h>
#include "Split.h"
#include <chrono>

using namespace std;
using namespace chrono;

class SuperNode {

public:
    int height;
    bool splitDim;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Branch node specific members
    optional<vector<SuperNode*>> childNodes;
    optional<vector<Split *>> splits;

    // Leaf node specific members
    optional<vector<array<float, 2>>> points;

    // Rect methods
    array<float, 4> combineRect(array<float, 4>);
    bool overlap(array<float, 4>) const;
    float edgeOverlap(int d, array<float, 4>) const;
    float overlapArea(array<float, 4>) const;
    bool containsPt(array<float, 2> p) const;
    bool contained(array<float, 4>) const;
    array<float, 2> getCenter() const;
    array<float, 2> getOverlapCenter(array<float, 4>) const;
    double minManhattanDist(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;

    // Common supernode methods
    void createRect(array<float, 4>, Split *, int);
    array<float, 2> getMedian() const;
    int size() const;
    //void mergeNode(int, SuperNode*);
    //int deleteNode(SuperNode*);

    // Branch specific methods
    virtual vector<SuperNode*> splitLeaf(SuperNode *, Split * = NULL, vector<SuperNode*> = vector<SuperNode*>());

    // Leaf specific methods
    virtual vector<SuperNode*> splitBranch(SuperNode*, vector<SuperNode*> = vector<SuperNode*>());
    int scan(array<float, 4>) const;

    ~SuperNode();
};
