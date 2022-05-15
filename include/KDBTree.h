#pragma once

#include "Node.h"

struct KDBTree {

    Node *root;

    KDBTree(uint, uint, Rect, SplitType);
    ~KDBTree();

    void bulkload(string, long);
    Info deleteQuery(Record);
    Info insertQuery(Record);
    Info kNNQuery(Data, uint);
    Info rangeQuery(Rect);
    uint size(map<string, double> &) const;
    void snapshot() const;
};
