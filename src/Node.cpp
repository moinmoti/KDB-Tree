#include "Node.h"

SplitType Node::Split::type;

/////////////////////////////////////////////////////////////////////////////////////////
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::overlap(Rect r) const {
    for (uint i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

bool Node::containsPt(Point p) const {
    bool result = true;
    for (uint i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(Rect r) const {
    bool result = true;
    for (uint i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

Point Node::getCenter() const { return Point{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2}; }

double Node::minSqrDist(Rect r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top) {
        if (left)
            return dist(rect[0], rect[3], r[2], r[1]);
        if (right)
            return dist(rect[2], rect[3], r[0], r[1]);
        return (r[1] - rect[3]) * (r[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return dist(rect[0], rect[1], r[2], r[3]);
        if (right)
            return dist(rect[2], rect[1], r[0], r[3]);
        return (rect[1] - r[3]) * (rect[1] - r[3]);
    }
    if (left)
        return (rect[0] - r[2]) * (rect[0] - r[2]);
    if (right)
        return (r[0] - rect[2]) * (r[0] - rect[2]);
    return 0;
}

Node::Split Node::getSplit(uint &writes) const {
    bool axis;
    vector<Entry> allEntries = getEntries(writes);
    if (Split::type == Cyclic) {
        axis = !splitDim;
    } else if (Split::type == Spread) {
        Point low({180, 90}), high({-180, -90});
        for (auto e : allEntries) {
            if (e.pt[0] < low[0])
                low[0] = e.pt[0];
            if (e.pt[1] < low[1])
                low[1] = e.pt[1];
            if (e.pt[0] > high[0])
                high[0] = e.pt[0];
            if (e.pt[1] > high[1])
                high[1] = e.pt[1];
        }
        axis = (high[0] - low[0]) < (high[1] - low[1]);
    } else if (Split::type == Orientation) {
        axis = (rect[2] - rect[0]) < (rect[3] - rect[1]);
    } else {
        cerr << "Error: Invalid TYPE!!!" << endl;
        abort();
    }
    sort(all(allEntries),
        [axis](const Entry &l, const Entry &r) { return l.pt[axis] < r.pt[axis]; });
    Split split{.axis = axis, .pt = allEntries[allEntries.size() / 2].pt[axis]};
    return split;
}

Node::~Node() {}
