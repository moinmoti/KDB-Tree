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
    SuperNode *root;

    KDBTree(int, int, array<float, 4>, string);
    ~KDBTree();

    void snapshot() const;
    void load(string, long);
    void leafFission(SuperNode *);
    void branchFission(SuperNode *);
    void bulkload(string, long);
    void rangeQuery(array<float, 4>, map<string, double> &);
    void deleteQuery(array<float, 2>, map<string, double> &);
    void insertQuery(array<float, 2>, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    void insertPoint(SuperNode *, SuperNode *, array<float, 2>);
    int size(map<string, double> &) const;
};
