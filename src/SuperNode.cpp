#include "SuperNode.h"

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

/////////////////////////////////////////////////////////////////////////////////////////
// Rectangle Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, array<float, 4> r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

array<float, 4> SuperNode::combineRect(array<float, 4> r) {
    array<float, 4> newRect;
    for (int i = 0; i < NUMDIMS; i++) {
        newRect[i] = min(rect[i], r[i]);
        newRect[i + NUMDIMS] = max(rect[i + NUMDIMS], r[i + NUMDIMS]);
    }
    return newRect;
}

bool SuperNode::overlap(array<float, 4> r) const {
    for (int i = 0; i < NUMDIMS; i++)
        if (rect[i] > r[i + NUMDIMS] || r[i] > rect[i + NUMDIMS])
            return false;
    return true;
}

float SuperNode::edgeOverlap(int d, array<float, 4> r) const {
    int i = !(d % 2);
    return min(rect[i + NUMDIMS], r[i + NUMDIMS]) - max(rect[i], r[i]);
}

float SuperNode::overlapArea(array<float, 4> r) const {
    float overlap = 1.0;
    for (int i = 0; i < NUMDIMS; i++) {
        if (rect[i] >= r[i + NUMDIMS] || r[i] >= rect[i + NUMDIMS])
            return 0;
        else
            overlap *= min(rect[i + NUMDIMS], r[i + NUMDIMS]) - max(rect[i], r[i]);
    }
    return overlap;
}

bool contains(array<float, 4> r, array<float, 2> p) {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (r[i] <= p[i]) & (r[i + NUMDIMS] >= p[i]);
    return result;
}

bool SuperNode::containsPt(array<float, 2> p) const {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + NUMDIMS] >= p[i]);
    return result;
}

bool SuperNode::contained(array<float, 4> r) const {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + NUMDIMS] <= r[i + NUMDIMS]);
    return result;
}

array<float, 2> SuperNode::getCenter() const {
    return array<float, 2>{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2};
}

array<float, 2> SuperNode::getOverlapCenter(array<float, 4> r) const {
    return array<float, 2>{(max(r[0], rect[0]) + min(r[2], rect[2])) / 2,
                           (max(r[1], rect[1]) + min(r[3], rect[3])) / 2};
}

double SuperNode::minManhattanDist(array<float, 4> r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top) {
        if (left)
            return distManhattan(rect[0], rect[3], r[2], r[1]);
        if (right)
            return distManhattan(rect[2], rect[3], r[0], r[1]);
        return abs(r[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return distManhattan(rect[0], rect[1], r[2], r[3]);
        if (right)
            return distManhattan(rect[2], rect[1], r[0], r[3]);
        return abs(rect[1] - r[3]);
    }
    if (left)
        return abs(rect[0] - r[2]);
    if (right)
        return abs(r[0] - rect[2]);
    return 0;
}

double SuperNode::minSqrDist(array<float, 4> r) const {
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

void SuperNode::createRect(array<float, 4> r, Split *_split, int side) {
    rect = r;
    rect[_split->axis + !side * NUMDIMS] = _split->pt[_split->axis];
}

array<float, 2> SuperNode::getMedian() const {
    if (points->size() % 2)
        return points.value()[points->size() / 2];
    array<float, 2> l, r;
    l = points.value()[points->size() / 2 - 1];
    r = points.value()[points->size() / 2];
    return array{(l[0] + r[0]) / 2, (l[1] + r[1]) / 2};
}

// void SuperNode::mergeNode(int mergeDir, SuperNode *expiredNode) {
// for (int dir = 0; dir < neighbors.size(); dir++) {
// int oppDir = (dir + NUMDIMS) % (NUMDIMS*2);
// if (dir == mergeDir) {
// neighbors[oppDir].erase(expiredNode);
// continue;
//}
// for (auto nb: expiredNode->neighbors[dir]) {
// if (nb->neighbors[oppDir].find(expiredNode) == nb->neighbors[oppDir].end())
// cerr << "Node not found!!!" << endl;
// nb->neighbors[oppDir].erase(expiredNode);
// nb->neighbors[oppDir].insert(this);
// neighbors[dir].insert(nb);
//}
//}
// rect = combineRect(expiredNode->rect);
//}

// int SuperNode::deleteNode(SuperNode *pn) {
// for (int dir = 0; dir < neighbors.size(); dir++) {
// if (neighbors[dir].size() > 1) continue;
// int oppDir = (dir + NUMDIMS) % (NUMDIMS*2);
// for (auto nb: neighbors[dir]) {
// if (rect[dir] == nb->rect[oppDir]
//&& rect[(dir+1)%2] == nb->rect[(dir+1)%2]
//&& rect[(dir+1)%2 + 2] == nb->rect[(dir+1)%2 + 2]) {
// nb->mergeNode(dir, this);
// pn->children.erase(find(all(pn->children), this));
// return 1;
//}
//}
//}
// return 0;
//}

vector<SuperNode *> SuperNode::splitLeaf(SuperNode *pn, Split *newSplit, vector<SuperNode *> sns) {
    for (int i = 0; i < sns.size(); i++) {
        sns[i]->height = 0;
        sns[i]->createRect(rect, newSplit, i);
        sns[i]->points = vector<array<float, 2>>();
    }

    // Splitting points
    for (auto p : (points).value()) {
        if (p[newSplit->axis] < newSplit->pt[newSplit->axis])
            sns[0]->points->emplace_back(p);
        else if (p[newSplit->axis] > newSplit->pt[newSplit->axis])
            sns[1]->points->emplace_back(p);
        else {
            if (sns[0]->points->size() < sns[1]->points->size())
                sns[0]->points->emplace_back(p);
            else
                sns[1]->points->emplace_back(p);
        }
    }

    pn->splits->emplace_back(newSplit);
    return sns;
}

vector<SuperNode *> SuperNode::splitBranch(SuperNode *pn, vector<SuperNode *> sns) {
    Split *bestSplit = splits.value()[0];
    for (int i = 0; i < sns.size(); i++) {
        sns[i]->height = height;
        sns[i]->createRect(rect, bestSplit, i);
        sns[i]->childNodes = vector<SuperNode *>();
        sns[i]->childNodes->reserve(childNodes->size());
        sns[i]->splits = vector<Split *>();
    }
    for (auto cn : childNodes.value())
        sns[cn->getCenter()[bestSplit->axis] > bestSplit->pt[bestSplit->axis]]
            ->childNodes->emplace_back(cn);
    for (auto isplit = next(splits->begin()); isplit != splits->end(); isplit++)
        sns[(*isplit)->pt[bestSplit->axis] > bestSplit->pt[bestSplit->axis]]->splits->emplace_back(
            *isplit);

    childNodes->clear();
    splits->clear();
    pn->splits->emplace_back(bestSplit);
    return sns;
}

int overlaps(array<float, 4> r, array<float, 2> p) {
    for (int i = 0; i < NUMDIMS; i++) {
        if (r[i] > p[i] || p[i] > r[i + NUMDIMS])
            return false;
    }
    return true;
}

int SuperNode::scan(array<float, 4> query) const {
    int totalPoints = 0;
    if (contained(query))
        return points->size();
    for (auto p : points.value())
        if (overlaps(query, p))
            totalPoints++;
    return totalPoints;
}

int SuperNode::size() const {
    int rectSize = sizeof(float) * 4;
    int typeSize = 0;
    if (points)
        typeSize = sizeof(vector<array<float, 2>>);
    else
        typeSize = sizeof(vector<SuperNode *>) + childNodes->size() * sizeof(SuperNode *) +
                   sizeof(vector<Split *>) + splits->size() * (sizeof(Split *) + sizeof(Split));
    int totalSize = typeSize + rectSize;
    return totalSize;
}

SuperNode::~SuperNode() {
    if (points) {
        points->clear();
        points.reset();
    } else {
        childNodes->clear();
        childNodes.reset();
    }
}
