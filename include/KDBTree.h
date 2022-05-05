#pragma once

#include "Node.h"

class KDBTree {

public:
    int fanout;
    int pageCap;
    SplitType splitType;
    Node *root;

    KDBTree(int, int, array<float, 4>, SplitType);
    ~KDBTree();

    void snapshot() const;
    void load(string, long);
    void fission(Node *);
    void bulkload(string, long);
    void rangeQuery(array<float, 4>, map<string, double> &);
    void deleteQuery(Record, map<string, double> &);
    void insertQuery(Record, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    int insertPoint(Node *, Node *, Record);
    int size(map<string, double> &) const;
};
