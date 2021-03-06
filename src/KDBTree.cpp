#include "KDBTree.h"

void printRect(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(uint _fanout, uint _pageCap, Rect _boundary, SplitType _splitType) {
    Directory::capacity = _fanout;
    Page::capacity = _pageCap;
    Node::Split::type = _splitType;

    root = new Directory();
    Directory *dRoot = static_cast<Directory *>(root);
    dRoot->rect = _boundary;
    dRoot->splitDim = 1;

    Node *firstPage = new Page();
    firstPage->rect = root->rect;
    firstPage->splitDim = 1;

    dRoot->contents.emplace_back(firstPage);
}

KDBTree::~KDBTree() {}

void KDBTree::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    uint i = 0;
    vector<Entry> entries;
    entries.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            uint id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            array pt{lon, lat};
            entries.emplace_back(Entry{.id = id, .pt = pt});
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    root = new Page(root);
    Page *pRoot = static_cast<Page *>(root);

    pRoot->entries = entries;
    cout << "Initiate fission" << endl;
    if (pRoot->entries.size() > Page::capacity)
        root = pRoot->fission();
}

Info KDBTree::deleteQuery(Entry e) {
    Info info;
    /* Node *node = root;
    while (node->height) {
        auto cn = static_cast<Directory *>(node)->contents.begin();
        while (!(*cn)->containsPt(e.pt))
            cn++;
        node = *cn;
    } */
    /* auto pt = find(all(node->entries.value()), e);
    if (e != node->entries->end())
        node->entries->erase(e); */
    return info;
}

Info KDBTree::insertQuery(Entry e) {
    Info info;
    info.cost = root->insert(root, e);
    return info;
}

Info KDBTree::kNNQuery(Point p, uint k) {
    Info info;
    array query{p[0], p[1], p[0], p[1]};

    min_heap<Node::knnNode> unseenNodes;
    vector<Node::knnEntry> tempEnts(k);
    max_heap<Node::knnEntry> knnEnts(all(tempEnts));
    Node *node = root;
    unseenNodes.emplace(Node::knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnEnts.top().dist;
        if (dist < minDist)
            info.cost += node->knnSearch(query, unseenNodes, knnEnts);
        else
            break;
    }

    if constexpr (DEBUG) {
        if (k == 32) {
            while (!knnEnts.empty()) {
                Entry entry = knnEnts.top().entry;
                double sqrDist = knnEnts.top().dist;
                knnEnts.pop();
                trace(entry.id, sqrDist);
            }
            cerr << endl;
        }
    }
    return info;
}

Info KDBTree::rangeQuery(Rect query) {
    Info info;
    info.cost = root->range(info.output, query);
    if constexpr (DEBUG) {
        uint numEntries = info.output;
        trace(numEntries);
        cerr << endl;
    }
    return info;
}

uint KDBTree::size(array<uint, 2> &info) const {
    info = {0, 0};
    return root->size(info);
}

void KDBTree::snapshot() const {
    ofstream ofs("Index.csv");
    root->snapshot(ofs);
    ofs.close();
}
