#pragma once

#include "Node.h"

struct KDBTree {

    Node *root;

    KDBTree(int, int, array<float, 4>, SplitType);
    ~KDBTree();

    void bulkload(string, long);
    void deleteQuery(Record, map<string, double> &);
    void insertQuery(Record, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    void rangeQuery(array<float, 4>, map<string, double> &);
    int size(map<string, double> &) const;
    void snapshot() const;
};
