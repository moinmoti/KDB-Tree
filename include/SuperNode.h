#pragma once

#include <bits/stdc++.h>
#include "Split.h"
#include <chrono>

using namespace std;
using namespace chrono;

class SuperNode {

public:
    int id;
    int height;
    vector<float> rect = vector<float>(4); // xlow, ylow, xhigh, yhigh
    Split* guide; // Origin Splits

    // Branch node specific members
    optional<vector<SuperNode*>> childNodes;

    // Leaf node specific members
    optional<vector<vector<float>>> points;

    // Rect methods
    vector<float> combineRect(vector<float>);
    bool overlap(vector<float>) const;
    float edgeOverlap(int d, vector<float>) const;
    float overlapArea(vector<float>) const;
    bool containsPt(vector<float> p) const;
    bool contained(vector<float>) const;
    vector<float> getCenter() const;
    vector<float> getOverlapCenter(vector<float>) const;
    double minManhattanDist(vector<float>) const;
    double minSqrDist(vector<float>) const;

    // Common supernode methods
    void setGuide(Split*, Split*);
    void createRect(vector<float>, Split*, int);
    int size() const;
    //void mergeNode(int, SuperNode*);
    //int deleteNode(SuperNode*);

    // Branch specific methods
    virtual vector<SuperNode*> splitLeaf(Split* = NULL, vector<SuperNode*> = vector<SuperNode*>());

    // Leaf specific methods
    virtual vector<SuperNode*> splitBranch(Split* = NULL, vector<SuperNode*> = vector<SuperNode*>());
    int scan(vector<float>) const;

    ~SuperNode();
};
