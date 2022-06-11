#include "Node.h"

uint Page::capacity;

bool overlaps(Rect r, Point p) {
    for (uint i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

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

uint Page::getHeight() const { return 0; }

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
    if constexpr (DEBUG) {
        if (inside(query))
            pointCount += entries.size();
        else {
            for (auto e : entries)
                if (overlaps(query, e.pt))
                    pointCount++;
        }
    }
    return 1;
}

uint Page::size(array<uint, 2> &info) const {
    info[1]++;
    return 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
}

void Page::snapshot(ofstream &ofs) const {
    ofs << 0 << "," << entries.size();
    for (auto c : rect)
        ofs << "," << c;
    ofs << endl;
}

Page::~Page() { entries.clear(); }
