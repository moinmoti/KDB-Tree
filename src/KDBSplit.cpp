#include "KDBSplit.h"

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1> void __f(const char *name, Arg1 &&arg1) {
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char *names, Arg1 &&arg1, Args &&... args) {
    const char *comma = strchr(names + 1, ',');
    cerr.write(names, comma - names) << " : " << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

#define all(c) c.begin(), c.end()
#define NUMDIMS 2
#define dist(x1, y1, x2, y2) (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)
#define distManhattan(x1, y1, x2, y2) std::abs(x1 - x2) + std::abs(y1 - y2)
#define oppDir(d) (d + NUMDIMS) % (NUMDIMS * 2)

#define V 0
#define H 1

vector<SuperNode *> KDBNode::splitLeaf(SuperNode *pn, Split *newSplit, vector<SuperNode *> sns) {
    sns = {new KDBNode(), new KDBNode()};

    bool axis = !splitDim;
    for (auto sn : sns)
        sn->splitDim = axis;
    sort(all(points.value()),
         [axis](const array<float, 2> &l, const array<float, 2> &r) { return l[axis] < r[axis]; });
    array median = getMedian();

    newSplit = new Split();
    newSplit->axis = axis;
    newSplit->pt[axis] = median[axis];
    newSplit->pt[!axis] = getCenter()[!axis];
    return SuperNode::splitLeaf(pn, newSplit, sns);
}

vector<SuperNode *> KDBNode::splitBranch(SuperNode *pn, vector<SuperNode *> sns) {
    sns = {new KDBNode(), new KDBNode()};
    return SuperNode::splitBranch(pn, sns);
}
