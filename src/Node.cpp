#include "Node.h"

SplitType Node::Split::type;
int Node::fanout;
int Node::pageCap;

int overlaps(array<float, 4> r, array<float, 2> p) {
    for (int i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, array<float, 4> r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::overlap(array<float, 4> r) const {
    for (int i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

bool Node::containsPt(array<float, 2> p) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(array<float, 4> r) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

array<float, 2> Node::getCenter() const {
    return array<float, 2>{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2};
}

double Node::minSqrDist(array<float, 4> r) const {
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

Node::Split *Node::getSplit() const {
    bool axis;
    vector<Record> allPoints = getPoints();
    if (Split::type == Cyclic) {
        axis = !splitDim;
    } else if (Split::type == Spread) {
        array<float, 2> low({180, 90}), high({-180, -90});
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

/////////////////////////////////////////////////////////////////////////////////////////
// Directory Methods
/////////////////////////////////////////////////////////////////////////////////////////

Directory::Directory(Node *pg) {
    rect = pg->rect;
    splitDim = pg->splitDim;
    height = 1;
    delete pg;
}

vector<Record> Directory::getPoints() const {
    vector<Record> allPoints;
    for (auto cn : contents) {
        vector<Record> tempPoints = cn->getPoints();
        allPoints.insert(allPoints.end(), all(tempPoints));
    }
    return allPoints;
}

int Directory::insert(Node *pn, Record p) {
    vector<Node *> newNodes;
    auto cn = contents.begin();
    while (!(*cn)->containsPt(p.data))
        cn++;
    int writes = (*cn)->insert(this, p);
    if (contents.size() > fanout) {
        Directory *dpn = dynamic_cast<Directory *>(pn);
        array<Node *, 2> newDirs = partition(writes);
        if (pn->height == height) {
            height++;
        } else {
            dpn->contents.erase(find(all(dpn->contents), this));
            delete this;
        }
        dpn->contents.emplace_back(newDirs);
    }
    return writes;
}

int Directory::range(int &pointCount, array<float, 4> query) const {
    int reads = 0;
    for (auto cn : contents) {
        if (cn->overlap(query))
            reads += cn->range(pointCount, query);
    }
    return reads;
}

int Directory::size() const {
    int rectSize = sizeof(float) * 4;
    int typeSize = sizeof(vector<Node *>) + contents.size() * sizeof(void *);
    int totalSize = typeSize + rectSize;
    return totalSize;
}

array<Node *, 2> Directory::partition(int &writes, Split *split) {
    array<Node *, 2> dirs = {new Directory(), new Directory()};
    if (split == NULL)
        split = getSplit();
    for (int i = 0; i < dirs.size(); i++) {
        Directory *dir = dynamic_cast<Directory *>(dirs[i]);
        dir->height = height;
        dir->rect = rect;
        dir->rect[split->axis + !i * D] = split->pt;
        dir->contents = vector<Node *>();
        dir->splitDim = split->axis;
    }

    // Splitting contents
    while (!contents.empty()) {
        Node *cn = contents.front();
        if (cn->rect[split->axis + D] <= split->pt)
            dynamic_cast<Directory *>(dirs[0])->contents.emplace_back(cn);
        else if (cn->rect[split->axis] >= split->pt)
            dynamic_cast<Directory *>(dirs[1])->contents.emplace_back(cn);
        else {
            contents.erase(find(all(contents), cn));
            contents.emplace_back(cn->partition(writes, split));
            delete cn;
        }
        contents.erase(contents.begin());
    }

    return dirs;
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

Page::Page(Node *dir) {
    rect = dir->rect;
    splitDim = dir->splitDim;
    height = 0;
    delete dir;
}

vector<Record> Page::getPoints() const { return points; }

Node *Page::fission() {
    int writes = 0;
    Node *node = (Directory *)this;
    node->height = log(points.size()) / log(pageCap);
    uint N = ceil(points.size() / 2);
    Directory *dir = dynamic_cast<Directory *>(node);
    dir->contents.emplace_back(partition(writes));
    for (; N > pageCap && dir->contents.size() <= fanout / 2; N = ceil(N / 2)) {
        vector<Node *> newPages;
        for (auto cn : dir->contents) {
            newPages.emplace_back(cn->partition(writes));
            delete cn;
        }
        dir->contents = newPages;
    }
    if (N > pageCap) {
        for (auto &cn : dir->contents)
            cn = dynamic_cast<Page *>(cn)->fission();
    }
    delete this;
    return node;
}

int Page::insert(Node *pn, Record p) {
    int writes = 2;
    points.emplace_back(p);
    if (points.size() > pageCap) {
        Directory *dpn = dynamic_cast<Directory *>(pn);
        dpn->contents.erase(find(all(dpn->contents), this));
        dpn->contents.emplace_back(partition(writes));
        delete this;
    }
    return writes;
}

int Page::range(int &pointCount, array<float, 4> query) const {
    if (inside(query))
        pointCount += points.size();
    else {
        for (auto p : points)
            if (overlaps(query, p.data))
                pointCount++;
    }
    return 1;
}

int Page::size() const {
    int rectSize = sizeof(float) * 4;
    int typeSize = sizeof(vector<array<float, 2>>);
    int totalSize = typeSize + rectSize;
    return totalSize;
}

array<Node *, 2> Page::partition(int &writes, Split *split) {
    array<Node *, 2> pages = {new Page(), new Page()};
    if (split == NULL)
        split = getSplit();
    for (int i = 0; i < pages.size(); i++) {
        Page *page = dynamic_cast<Page *>(pages[i]);
        page->height = 0;
        page->rect = rect;
        page->rect[split->axis + !i * D] = split->pt;
        page->points = vector<Record>();
        page->splitDim = split->axis;
    }

    // Splitting points
    Page *firstPage = dynamic_cast<Page *>(pages[0]);
    Page *secondPage = dynamic_cast<Page *>(pages[1]);
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
    writes = 3;
    return pages;
}

Page::~Page() { points.clear(); }