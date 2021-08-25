#pragma once

#include "Node.h"
#include "common.h"

class KDBTree {

public:
    int fanout;
    int pageCap;
    Node *root;

    KDBTree(int, int, array<float, 4>, string);
    ~KDBTree();

    void snapshot() const;
    void load(string, long);
    void fission(Node *);
    void bulkload(string, long);
    void rangeQuery(array<float, 4>, map<string, double> &);
    void deleteQuery(array<float, 2>, map<string, double> &);
    void insertQuery(array<float, 2>, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    void insertPoint(Node *, Node *, array<float, 2>);
    int size(map<string, double> &) const;
};
