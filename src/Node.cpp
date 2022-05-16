#include "Node.h"

SplitType Node::Split::type;

bool overlaps(Rect r, Data p) {
    for (uint i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

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

bool Node::containsPt(Data p) const {
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

Data Node::getCenter() const { return Data{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2}; }

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

Node::Split *Node::getSplit(uint &writes) const {
    bool axis;
    vector<Record> allPoints = getPoints(writes);
    if (Split::type == Cyclic) {
        axis = !splitDim;
    } else if (Split::type == Spread) {
        Data low({180, 90}), high({-180, -90});
        for (auto p : allPoints) {
            if (p.data[0] < low[0])
                low[0] = p.data[0];
            if (p.data[1] < low[1])
                low[1] = p.data[1];
            if (p.data[0] > high[0])
                high[0] = p.data[0];
            if (p.data[1] > high[1])
                high[1] = p.data[1];
        }
        axis = (high[0] - low[0]) < (high[1] - low[1]);
    } else if (Split::type == Orientation) {
        axis = (rect[2] - rect[0]) < (rect[3] - rect[1]);
    } else {
        cerr << "Error: Invalid TYPE!!!" << endl;
        abort();
    }
    sort(all(allPoints),
         [axis](const Record &l, const Record &r) { return l.data[axis] < r.data[axis]; });
    Split *split = new Split();
    split->axis = axis;
    split->pt = allPoints[allPoints.size() / 2].data[axis];
    return split;
}

Node::~Node() {}

/////////////////////////////////////////////////////////////////////////////////////////
// Directory Methods
/////////////////////////////////////////////////////////////////////////////////////////

uint Directory::capacity;

Directory::Directory() {}

Directory::Directory(Node *pg, bool canDel) {
    rect = pg->rect;
    splitDim = pg->splitDim;
    if (canDel)
        delete pg;
}

vector<Record> Directory::getPoints(uint &writes) const {
    vector<Record> allPoints;
    for (auto cn : contents) {
        vector<Record> tempPoints = cn->getPoints(writes);
        allPoints.insert(allPoints.end(), all(tempPoints));
    }
    return allPoints;
}

uint Directory::insert(Node *pn, Record p) {
    auto cn = contents.begin();
    while (!(*cn)->containsPt(p.data))
        cn++;
    uint writes = (*cn)->insert(this, p);
    if (contents.size() > capacity) {
        Directory *dpn = static_cast<Directory *>(pn);
        array<Node *, 2> newDirs = partition(writes);
        if (pn != this) {
            dpn->contents.erase(find(all(dpn->contents), this));
            delete this;
        }
        for (auto dir : newDirs)
            dpn->contents.emplace_back(dir);
    }
    return writes;
}

uint Directory::knnSearch(Rect query, min_heap<knnNode> &unseenNodes,
                          max_heap<knnPoint> &knnPts) const {
    double minDist = knnPts.top().dist;
    for (auto cn : contents) {
        double dist = cn->minSqrDist(query);
        if (dist < minDist) {
            knnNode kn;
            kn.sn = cn;
            kn.dist = dist;
            unseenNodes.push(kn);
        }
    }
    return 0;
}

array<Node *, 2> Directory::partition(uint &writes, Split *split) {
    array<Node *, 2> dirs = {new Directory(), new Directory()};
    if (split == NULL)
        split = getSplit(writes);
    for (uint i = 0; i < dirs.size(); i++) {
        dirs[i]->rect = rect;
        dirs[i]->rect[split->axis + !i * D] = split->pt;
        dirs[i]->splitDim = split->axis;
    }

    // Splitting contents
    while (!contents.empty()) {
        Node *cn = contents.front();
        if (cn->rect[split->axis + D] <= split->pt)
            static_cast<Directory *>(dirs[0])->contents.emplace_back(cn);
        else if (cn->rect[split->axis] >= split->pt)
            static_cast<Directory *>(dirs[1])->contents.emplace_back(cn);
        else {
            array<Node *, 2> newNodes = cn->partition(writes, split);
            for (auto nd : newNodes)
                contents.emplace_back(nd);
            delete cn;
        }
        contents.erase(contents.begin());
    }

    return dirs;
}

uint Directory::range(uint &pointCount, Rect query) const {
    uint reads = 0;
    for (auto cn : contents) {
        if (cn->overlap(query))
            reads += cn->range(pointCount, query);
    }
    return reads;
}

uint Directory::size() const {
    uint rectSize = sizeof(float) * 4;
    uint typeSize = sizeof(vector<Node *>) + contents.size() * sizeof(void *);
    uint totalSize = typeSize + rectSize;
    return totalSize;
}

Directory::~Directory() {
    for (auto cn : contents) {
        delete cn;
    }
    contents.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////
// Page Methods
/////////////////////////////////////////////////////////////////////////////////////////

uint Page::capacity;

Page::Page() {}

Page::Page(Node *dir, bool canDel) {
    rect = dir->rect;
    splitDim = dir->splitDim;
    if (canDel)
        delete dir;
}

Node *Page::fission() {
    uint writes = 0;
    Node *node = new Directory(this, false);
    uint N = ceil(points.size() / 2);
    Directory *dir = static_cast<Directory *>(node);
    array<Node *, 2> newPages = partition(writes);
    for (auto pg : newPages)
        dir->contents.emplace_back(pg);
    for (; N > capacity && dir->contents.size() <= dir->capacity / 2; N = ceil(N / 2)) {
        vector<Node *> pages;
        for (auto cn : dir->contents) {
            array<Node *, 2> newPages = cn->partition(writes);
            for (auto pg : newPages)
                pages.emplace_back(pg);
            delete cn;
        }
        dir->contents = pages;
    }
    if (N > capacity) {
        for (auto &cn : dir->contents)
            cn = static_cast<Page *>(cn)->fission();
    }
    delete this;
    return node;
}

vector<Record> Page::getPoints(uint &writes) const {
    writes++;
    return points;
}

uint Page::insert(Node *pn, Record p) {
    uint writes = 2;
    points.emplace_back(p);
    if (points.size() > capacity) {
        Directory *dpn = static_cast<Directory *>(pn);
        dpn->contents.erase(find(all(dpn->contents), this));
        writes = 0;
        array<Node *, 2> newPages = partition(writes);
        for (auto pg : newPages)
            dpn->contents.emplace_back(pg);
        delete this;
    }
    return writes;
}

uint Page::knnSearch(Rect query, min_heap<knnNode> &unseenNodes, max_heap<knnPoint> &knnPts) const {
    auto calcSqrDist = [](Rect x, Data y) { return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2); };
    for (auto p : points) {
        double minDist = knnPts.top().dist;
        double dist = calcSqrDist(query, p.data);
        if (dist < minDist) {
            knnPoint kPt;
            kPt.pt = p;
            kPt.dist = dist;
            knnPts.pop();
            knnPts.push(kPt);
        }
    }
    return 1;
}

array<Node *, 2> Page::partition(uint &writes, Split *split) {
    array<Node *, 2> pages = {new Page(), new Page()};
    if (split == NULL)
        split = getSplit(writes);
    for (uint i = 0; i < pages.size(); i++) {
        pages[i]->rect = rect;
        pages[i]->rect[split->axis + !i * D] = split->pt;
        pages[i]->splitDim = split->axis;
    }

    // Splitting points
    Page *firstPage = static_cast<Page *>(pages[0]);
    Page *secondPage = static_cast<Page *>(pages[1]);
    for (auto p : points) {
        if (p.data[split->axis] < split->pt)
            firstPage->points.emplace_back(p);
        else if (p.data[split->axis] > split->pt)
            secondPage->points.emplace_back(p);
        else {
            if (firstPage->points.size() < secondPage->points.size())
                firstPage->points.emplace_back(p);
            else
                secondPage->points.emplace_back(p);
        }
    }

    points.clear();
    writes += 2; // One write is already added in getSplit either in this call or before.
    return pages;
}

uint Page::range(uint &pointCount, Rect query) const {
    if (inside(query))
        pointCount += points.size();
    else {
        for (auto p : points)
            if (overlaps(query, p.data))
                pointCount++;
    }
    return 1;
}

uint Page::size() const {
    uint rectSize = sizeof(float) * 4;
    uint typeSize = sizeof(vector<Data>);
    uint totalSize = typeSize + rectSize;
    return totalSize;
}

Page::~Page() { points.clear(); }
