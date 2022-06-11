#include "Node.h"

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

uint Directory::getHeight() const { return contents.front()->getHeight() + 1; }

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

void Directory::snapshot(ofstream &ofs) const {
    uint height = getHeight();
    ofs << height << "," << contents.size();
    for (auto c : rect)
        ofs << "," << c;
    ofs << endl;
    for (auto cn : contents)
        cn->snapshot(ofs);
}

Directory::~Directory() {
    for (auto cn : contents) {
        delete cn;
    }
    contents.clear();
}
