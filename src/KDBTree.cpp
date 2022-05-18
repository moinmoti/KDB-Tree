#include "KDBTree.h"

void printRect(string str, Rect r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(uint _pageCap, uint _fanout, Rect _boundary, SplitType _splitType) {
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

    double sqrDist;
    if (k == 32) {
        while (!knnEnts.empty()) {
            Entry entry = knnEnts.top().entry;
            sqrDist = knnEnts.top().dist;
            knnEnts.pop();
            trace(entry.id, sqrDist);
        }
        cerr << endl;
    }
    return info;
}

Info KDBTree::rangeQuery(Rect query) {
    Info info;
    info.cost = root->range(info.output, query);
    uint pointCount = info.output;
    trace(pointCount);
    return info;
}

uint KDBTree::size(map<string, double> &stats) const {
    uint totalSize = 2 * sizeof(uint);
    uint pageSize = 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
    uint directorySize = 4 * sizeof(float) + sizeof(uint) + sizeof(void *);
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : dir->contents) {
            Directory *dcn = dynamic_cast<Directory *>(cn);
            if (dcn) {
                toVisit.push(dcn);
            } else
                stats["pages"]++;
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"];
    return totalSize;
}

void KDBTree::snapshot() const {
    string splitStr;
    SplitType usedType = Node::Split::type;
    if (usedType == Cyclic)
        splitStr = "Cyclic";
    else if (usedType == Orientation)
        splitStr = "Orientation";
    else if (usedType == Spread)
        splitStr = "Spread";
    else
        splitStr = "Invalid";

    ofstream log(splitStr + "-KDBTree.csv");
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        // log << dir->height << "," << dir->contents.size();
        for (auto p : dir->rect)
            log << "," << p;
        log << endl;
        for (auto cn : dir->contents) {
            Directory *dcn = dynamic_cast<Directory *>(cn);
            if (dcn) {
                toVisit.push(dcn);
            } else {
                log << 0 << "," << static_cast<Page *>(cn)->entries.size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl;
            }
        }
    }
    log.close();
}
