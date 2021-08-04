#include "OrientSplit.h"

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

array<float, 2> getMedian(vector<array<float, 2>> pts) {
    if (pts.size() % 2)
        return pts[pts.size() / 2];
    array<float, 2> l, r;
    l = pts[pts.size() / 2 - 1];
    r = pts[pts.size() / 2];
    return array{(l[0] + r[0]) / 2, (l[1] + r[1]) / 2};
}

vector<SuperNode *> OrientNode::splitLeaf(SuperNode *pn, Split *newSplit, vector<SuperNode *> sns) {
    sns = {new OrientNode(), new OrientNode()};

    bool axis = (rect[2] - rect[0]) < (rect[3] - rect[1]);
    sort(all(points.value()),
         [axis](const array<float, 2> &l, const array<float, 2> &r) { return l[axis] < r[axis]; });
    array median = getMedian(points.value());

    newSplit = new Split();
    newSplit->axis = axis;
    newSplit->pt[axis] = median[axis];
    newSplit->pt[!axis] = getCenter()[!axis];
    return SuperNode::splitLeaf(pn, newSplit, sns);
}

vector<SuperNode *> OrientNode::splitBranch(SuperNode *pn, vector<SuperNode *> sns) {
    sns = {new OrientNode(), new OrientNode()};
    return SuperNode::splitBranch(pn, sns);
}
