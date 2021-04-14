#pragma once

#include <bits/stdc++.h>
#include "OrientSplit.h"
#include <chrono>

using namespace std;

class KDBTree {

public:
    int branchCap;
    int leafCap;
    int splitCount;
    SuperNode* root;

    KDBTree(int, int, vector<float>, string);
    ~KDBTree();

    void snapshot();
    void fullSnapshot(vector<float>);
    void load(string, long);
    void rangeQuery(float*, float*, map<string, double>&);
    void deleteQuery(vector<float>, map<string, double>&);
    void insertQuery(vector<float>, map<string, double>&);
    void kNNQuery(vector<float>, map<string, double>&, int);
    void insertPoint(SuperNode*, SuperNode*, vector<float>);
    int size() const;
};
