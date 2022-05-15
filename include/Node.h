#pragma once

#include "common.h"

struct Node {

    struct knnPoint {
        Record pt;
        double dist = numeric_limits<double>::max();
        bool operator<(const knnPoint &second) const { return dist < second.dist; }
    };

    struct knnNode {
        Node *sn;
        double dist = numeric_limits<double>::max();
        bool operator>(const knnNode &second) const { return dist > second.dist; }
    };

    struct Split {
        static SplitType type;
        float pt;
        bool axis;
    };

    bool splitDim;
    Rect rect; // xlow, ylow, xhigh, yhigh

    // Rect methods
    bool containsPt(Data p) const;
    Data getCenter() const;
    bool inside(Rect) const;
    double minSqrDist(Rect) const;
    bool overlap(Rect) const;

    virtual vector<Record> getPoints(uint &) const = 0;
    Split* getSplit(uint &) const;
    virtual uint insert(Node *, Record) = 0;
    virtual uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const = 0;
    virtual array<Node*, 2> partition(uint &, Split * = NULL) = 0;
    virtual uint range(uint &, Rect query) const = 0;
    virtual uint size() const = 0;

    virtual ~Node() = 0;
};

struct Directory: Node {
    static uint capacity;
    vector<Node*> contents;

    Directory();
    explicit Directory(Node *, bool = true);

    vector<Record> getPoints(uint &) const;
    uint insert(Node *, Record);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const;
    array<Node*, 2> partition(uint &, Split * = NULL);
    uint range(uint &, Rect query) const;
    uint size() const;

    ~Directory();
};

struct Page: Node {
    static uint capacity;
    vector<Record> points;

    Page();
    explicit Page(Node *, bool = true);

    Node* fission();
    vector<Record> getPoints(uint &) const;
    uint insert(Node *, Record);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnPoint> &) const;
    array<Node *, 2> partition(uint &, Split * = NULL);
    uint range(uint &, Rect) const;
    uint size() const;

    ~Page();
};
