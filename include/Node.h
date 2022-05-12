#pragma once

#include "common.h"

struct Node {

    struct Split {
        static SplitType type;
        float pt;
        bool axis;
    };

    static int fanout;
    static int pageCap;

    int height;
    bool splitDim;
    array<float, 4> rect; // xlow, ylow, xhigh, yhigh

    // Rect methods
    bool containsPt(array<float, 2> p) const;
    array<float, 2> getCenter() const;
    bool inside(array<float, 4>) const;
    double minSqrDist(array<float, 4>) const;
    bool overlap(array<float, 4>) const;

    virtual vector<Record> getPoints() const = 0;
    Split* getSplit() const;
    virtual int insert(Node *, Record) = 0;
    virtual array<Node*, 2> partition(int &, Split * = NULL) = 0;
    virtual int range(int &, array<float, 4> query) const = 0;
    virtual int size() const = 0;

    virtual ~Node() = 0;
};

struct Directory: Node {
    vector<Node*> contents;

    Directory();
    explicit Directory(Node *, bool = true);

    vector<Record> getPoints() const;
    int insert(Node *, Record);
    array<Node*, 2> partition(int &, Split * = NULL);
    int range(int &, array<float, 4> query) const;
    int size() const;

    ~Directory();
};

struct Page: Node {
    vector<Record> points;

    Page();
    explicit Page(Node *, bool = true);

    Node* fission();
    vector<Record> getPoints() const;
    int insert(Node *, Record);
    array<Node *, 2> partition(int &, Split * = NULL);
    int range(int &, array<float, 4>) const;
    int size() const;

    ~Page();
};
