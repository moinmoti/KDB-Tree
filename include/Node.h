#pragma once

#include "common.h"

struct Node {

    struct knnEntry {
        Entry entry;
        double dist = numeric_limits<double>::max();
        bool operator<(const knnEntry &second) const { return dist < second.dist; }
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
    bool containsPt(Point p) const;
    Point getCenter() const;
    bool inside(Rect) const;
    double minSqrDist(Rect) const;
    bool overlap(Rect) const;

    virtual vector<Entry> getEntries(uint &) const = 0;
    Split* getSplit(uint &) const;
    virtual uint insert(Node *, Entry) = 0;
    virtual uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const = 0;
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

    vector<Entry> getEntries(uint &) const;
    uint insert(Node *, Entry);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const;
    array<Node*, 2> partition(uint &, Split * = NULL);
    uint range(uint &, Rect query) const;
    uint size() const;

    ~Directory();
};

struct Page: Node {
    static uint capacity;
    vector<Entry> entries;

    Page();
    explicit Page(Node *, bool = true);

    Node* fission();
    vector<Entry> getEntries(uint &) const;
    uint insert(Node *, Entry);
    uint knnSearch(Rect, min_heap<knnNode> &, max_heap<knnEntry> &) const;
    array<Node *, 2> partition(uint &, Split * = NULL);
    uint range(uint &, Rect) const;
    uint size() const;
    uint snapshot(ofstream &) const;

    ~Page();
};
