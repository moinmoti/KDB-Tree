#pragma once

#include "Node.h"

struct KDBTree {

    Node *root;

    KDBTree(int, int, array<float, 4>, SplitType);
    ~KDBTree();

    void bulkload(string, long);
    Info deleteQuery(Record);
    Info insertQuery(Record);
    Info kNNQuery(array<float, 2>, int);
    Info rangeQuery(array<float, 4>);
    int size(map<string, double> &) const;
    void snapshot() const;
};
