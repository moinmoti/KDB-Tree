#include "KDBTree.h"

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(int _pageCap, int _fanout, array<float, 4> _boundary, SplitType _splitType) {
    Node::fanout = _fanout;
    Node::pageCap = _pageCap;
    Node::Split::type = _splitType;

    root = new Directory();
    Directory *dRoot = static_cast<Directory *>(root);
    dRoot->rect = _boundary;
    dRoot->height = 1;
    dRoot->splitDim = 1;
    dRoot->contents = vector<Node *>();

    Node *firstPage = new Page();
    firstPage->rect = root->rect;
    firstPage->height = 0;
    firstPage->splitDim = 1;
    static_cast<Page *>(firstPage)->points = vector<Record>();

    dRoot->contents.emplace_back(firstPage);
}

KDBTree::~KDBTree() {}

void KDBTree::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    vector<Record> Points;
    Points.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            array pt{lon, lat};
            Points.emplace_back(Record{.id = id, .data = pt});
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    root = (Page *)root;
    Page *pRoot = static_cast<Page *>(root);

    pRoot->points = Points;
    cout << "Initiate fission" << endl;
    if (pRoot->points.size() > Node::pageCap)
        root = pRoot->fission();
}

void KDBTree::deleteQuery(Record p, map<string, double> &stats) {
    Node *node = root;
    while (node->height) {
        auto cn = static_cast<Directory *>(node)->contents.begin();
        while (!(*cn)->containsPt(p.data))
            cn++;
        node = *cn;
    }
    /* auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end())
        node->points->erase(pt); */
}

void KDBTree::insertQuery(Record p, map<string, double> &stats) {
    stats["io"] = root->insert(root, p);
}

typedef struct knnPoint {
    Record pt;
    double dist = numeric_limits<double>::max();
    bool operator<(const knnPoint &second) const { return dist < second.dist; }
} knnPoint;

typedef struct knnNode {
    Node *sn;
    double dist = numeric_limits<double>::max();
    bool operator<(const knnNode &second) const { return dist > second.dist; }
} knnNode;

void kNNSearch(Node *node, array<float, 4> query,
               priority_queue<knnPoint, vector<knnPoint>> &knnPts, map<string, double> &stats) {
    auto sqrDist = [](array<float, 4> x, array<float, 2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->height == 0) {
                Page *pg = static_cast<Page *>(node);
                for (auto p : pg->points) {
                    minDist = knnPts.top().dist;
                    dist = sqrDist(query, p.data);
                    if (dist < minDist) {
                        knnPoint kPt;
                        kPt.pt = p;
                        kPt.dist = dist;
                        knnPts.pop();
                        knnPts.push(kPt);
                    }
                }
                stats["io"]++;
            } else {
                Directory *dir = static_cast<Directory *>(node);
                minDist = knnPts.top().dist;
                for (auto cn : dir->contents) {
                    dist = cn->minSqrDist(query);
                    if (dist < minDist) {
                        knnNode kn;
                        kn.sn = cn;
                        kn.dist = dist;
                        unseenNodes.push(kn);
                    }
                }
            }
        } else
            break;
    }
}

void KDBTree::kNNQuery(array<float, 2> p, map<string, double> &stats, int k) {
    array query{p[0], p[1], p[0], p[1]};

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    kNNSearch(root, query, knnPts, stats);

    /* double sqrDist;
    if (k == 32) {
        while (!knnPts.empty()) {
            Record pt = knnPts.top().pt;
            sqrDist = knnPts.top().dist;
            knnPts.pop();
            trace(pt.id);
        }
    } */
}

void KDBTree::rangeQuery(array<float, 4> query, map<string, double> &stats) {
    int pointCount;
    stats["io"] = root->range(pointCount, query);
    // trace(pointCount);
}

int KDBTree::size(map<string, double> &stats) const {
    int totalSize = 2 * sizeof(int);
    int pageSize = 4 * sizeof(float) + sizeof(int) + sizeof(void *);
    int directorySize = 4 * sizeof(float) + sizeof(int) + sizeof(void *);
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : dir->contents) {
            if (cn->height) {
                toVisit.push(static_cast<Directory *>(cn));
            } else
                stats["pages"]++;
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"];
    return totalSize;
}

void KDBTree::snapshot() const {
    string splitStr = (Node::Split::type == Cyclic)   ? "Cyclic"
                      : (Node::Split::type == Spread) ? "Spread"
                                                      : "Invalid";
    ofstream log(splitStr + "-KDBTree.csv");
    stack<Directory *> toVisit({static_cast<Directory *>(root)});
    Directory *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        log << dir->height << "," << dir->contents.size();
        for (auto p : dir->rect)
            log << "," << p;
        log << endl;
        for (auto cn : dir->contents) {
            if (cn->height) {
                toVisit.push(static_cast<Directory *>(cn));
            } else {
                log << cn->height << "," << static_cast<Page *>(cn)->points.size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl;
            }
        }
    }
    log.close();
}
