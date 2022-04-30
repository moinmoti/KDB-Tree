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
    void deleteQuery(Record, map<string, double> &);
    void insertQuery(Record, map<string, double> &);
    void kNNQuery(array<float, 2>, map<string, double> &, int);
    int insertPoint(Node *, Node *, Record);
    int size(map<string, double> &) const;
};
