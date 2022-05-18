#pragma once

#include "Node.h"

struct KDBTree {

    Node *root;

    KDBTree(uint, uint, Rect, SplitType);
    ~KDBTree();

    void bulkload(string, long);
    Info deleteQuery(Entry);
    Info insertQuery(Entry);
    Info kNNQuery(Point, uint);
    Info rangeQuery(Rect);
    uint size(map<string, double> &) const;
    void snapshot() const;
};
