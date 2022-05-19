#include "Node.h"

SplitType Node::Split::type;

bool overlaps(Rect r, Point p) {
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

vector<Entry> Directory::getEntries(uint &writes) const {
    vector<Entry> allEntries;
    for (auto cn : contents) {
        vector<Entry> tempEntries = cn->getEntries(writes);
        allEntries.insert(allEntries.end(), all(tempEntries));
    }
    return allEntries;
}

uint Directory::insert(Node *pn, Entry e) {
    auto cn = contents.begin();
    while (!(*cn)->containsPt(e.pt))
        cn++;
    uint writes = (*cn)->insert(this, e);
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
                          max_heap<knnEntry> &knnEnts) const {
    double minDist = knnEnts.top().dist;
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

array<Node *, 2> Directory::partition(uint &writes, Split &split) {
    array<Node *, 2> dirs = {new Directory(), new Directory()};
    for (uint i = 0; i < dirs.size(); i++) {
        dirs[i]->rect = rect;
        dirs[i]->rect[split.axis + !i * D] = split.pt;
        dirs[i]->splitDim = split.axis;
    }

    // Splitting contents
    while (!contents.empty()) {
        Node *cn = contents.front();
        if (cn->rect[split.axis + D] <= split.pt)
            static_cast<Directory *>(dirs[0])->contents.emplace_back(cn);
        else if (cn->rect[split.axis] >= split.pt)
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

array<Node *, 2> Directory::partition(uint &writes) {
    Split split = getSplit(writes);
    return partition(writes, split);
}

uint Directory::range(uint &pointCount, Rect query) const {
    uint reads = 0;
    for (auto cn : contents) {
        if (cn->overlap(query))
            reads += cn->range(pointCount, query);
    }
    return reads;
}

uint Directory::size(array<uint, 2> &info) const {
    info[0]++;
    uint totalSize = 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
    for (auto cn : contents)
        totalSize += cn->size(info);
    return totalSize;
}

uint Directory::snapshot(ofstream &ofs) const {
    uint height = 0;
    for (auto cn : contents)
        height = cn->snapshot(ofs) + 1;
    ofs << height << "," << contents.size();
    for (auto c : rect)
        ofs << "," << c;
    ofs << endl;
    return height;
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
    uint N = ceil(entries.size() / 2);
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

vector<Entry> Page::getEntries(uint &writes) const {
    writes++;
    return entries;
}

uint Page::insert(Node *pn, Entry e) {
    uint writes = 2;
    entries.emplace_back(e);
    if (entries.size() > capacity) {
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

uint Page::knnSearch(Rect query, min_heap<knnNode> &unseenNodes,
                     max_heap<knnEntry> &knnEnts) const {
    auto calcSqrDist = [](Rect x, Point y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };
    for (auto e : entries) {
        double minDist = knnEnts.top().dist;
        double dist = calcSqrDist(query, e.pt);
        if (dist < minDist) {
            knnEntry kEnt;
            kEnt.entry = e;
            kEnt.dist = dist;
            knnEnts.pop();
            knnEnts.push(kEnt);
        }
    }
    return 1;
}

array<Node *, 2> Page::partition(uint &writes, Split &split) {
    array<Node *, 2> pages = {new Page(), new Page()};
    for (uint i = 0; i < pages.size(); i++) {
        pages[i]->rect = rect;
        pages[i]->rect[split.axis + !i * D] = split.pt;
        pages[i]->splitDim = split.axis;
    }

    // Splitting entries
    Page *firstPage = static_cast<Page *>(pages[0]);
    Page *secondPage = static_cast<Page *>(pages[1]);
    for (auto e : entries) {
        if (e.pt[split.axis] < split.pt)
            firstPage->entries.emplace_back(e);
        else if (e.pt[split.axis] > split.pt)
            secondPage->entries.emplace_back(e);
        else {
            if (firstPage->entries.size() < secondPage->entries.size())
                firstPage->entries.emplace_back(e);
            else
                secondPage->entries.emplace_back(e);
        }
    }

    entries.clear();
    writes += 2; // One write is already added in getSplit either in this call or before.
    return pages;
}

array<Node *, 2> Page::partition(uint &writes) {
    Split split = getSplit(writes);
    return partition(writes, split);
}

uint Page::range(uint &pointCount, Rect query) const {
    if (inside(query))
        pointCount += entries.size();
    else {
        for (auto e : entries)
            if (overlaps(query, e.pt))
                pointCount++;
    }
    return 1;
}

uint Page::size(array<uint, 2> &info) const {
    info[1]++;
    return 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
}

uint Page::snapshot(ofstream &ofs) const {
    ofs << 0 << "," << entries.size();
    for (auto c : rect)
        ofs << "," << c;
    ofs << endl;
    return 0;
}

Page::~Page() { entries.clear(); }
