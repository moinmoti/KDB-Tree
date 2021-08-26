#include "KDBTree.h"

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(int _pageCap, int _fanout, array<float, 4> _boundary, string type) {
    pageCap = _pageCap;
    fanout = _fanout;

    root = new Node();
    root->rect = _boundary;
    root->height = 0;
    root->splitDim = 1;
}

KDBTree::~KDBTree() {}

void KDBTree::snapshot() const {
    string splitType = (TYPE) ? "Spread" : "Cyclic";
    ofstream log(splitType + "-KDBTree.csv");
    stack<Node *> toVisit({root});
    Node *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        log << dir->height << "," << dir->contents->size();
        for (auto p : dir->rect)
            log << "," << p;
        log << endl;
        for (auto cn : dir->contents.value()) {
            if (cn->points) {
                log << cn->height << "," << cn->points->size();
                for (auto p : cn->rect)
                    log << "," << p;
                log << endl;
            } else {
                toVisit.push(cn);
            }
        }
    }
    log.close();
}

void KDBTree::fission(Node *node) {
    node->height = log(node->points->size()) / log(pageCap);
    uint N = ceil(node->points->size() / 2);
    node->contents = node->splitPage();
    node->points->clear();
    node->points.reset();
    for (; N > pageCap && node->contents->size() <= fanout / 2; N = ceil(N / 2)) {
        vector<Node *> newPages;
        for (auto cn : node->contents.value()) {
            vector<Node *> pages = cn->splitPage();
            for (auto page : pages)
                newPages.emplace_back(page);
            delete cn;
        }
        node->contents = newPages;
    }
    if (N > pageCap) {
        for (auto cn : node->contents.value())
            fission(cn);
    }
}

void KDBTree::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    vector<array<float, 2>> Points;
    Points.reserve(limit);
    if (file.is_open()) {
        // getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lon >> lat;
            array pt{lon, lat};
            Points.emplace_back(pt);
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    cout << "Initiate fission" << endl;
    root->points = Points;
    fission(root);
}

void KDBTree::insertPoint(Node *pn, Node *node, array<float, 2> p) {
    vector<Node *> newNodes;
    if (node->points) {
        node->points->emplace_back(p);
        if (node->points->size() > pageCap)
            newNodes = node->splitPage();
    } else {
        auto cn = node->contents->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        insertPoint(node, *cn, p);
        if (node->contents->size() > fanout)
            newNodes = node->splitDirectory();
    }
    if (!newNodes.empty()) {
        if (node == root) {
            root->contents->clear();
            root->height++;
        } else {
            pn->contents->erase(find(all(pn->contents.value()), node));
            delete node;
        }
        for (auto cn : newNodes)
            pn->contents->emplace_back(cn);
    }
}

void KDBTree::insertQuery(array<float, 2> p, map<string, double> &stats) {
    insertPoint(root, root, p);
}

void KDBTree::deleteQuery(array<float, 2> p, map<string, double> &stats) {
    Node *node = root;
    while (node->contents) {
        auto cn = node->contents->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        node = *cn;
    }
    auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end())
        node->points->erase(pt);
}

void rangeSearch(Node *node, int &pointCount, array<float, 4> query, map<string, double> &stats) {
    if (node->points) {
        stats["io"]++;
        // pointCount += node->scan(query);
    } else {
        for (auto cn : node->contents.value())
            if (cn->overlap(query))
                rangeSearch(cn, pointCount, query, stats);
    }
}

void KDBTree::rangeQuery(array<float, 4> query, map<string, double> &stats) {
    int pointCount = 0;
    rangeSearch(root, pointCount, query, stats);
    // trace(pointCount);
}

typedef struct knnPoint {
    array<float, 2> pt;
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
            if (node->points) {
                for (auto p : node->points.value()) {
                    minDist = knnPts.top().dist;
                    dist = sqrDist(query, p);
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
                minDist = knnPts.top().dist;
                for (auto cn : node->contents.value()) {
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
    Node *foundNode;
    array query{p[0], p[1], p[0], p[1]};

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    kNNSearch(root, query, knnPts, stats);

    /* double sqrDist;
    while (!knnPts.empty()) {
        p = knnPts.top().pt;
        sqrDist = knnPts.top().dist;
        knnPts.pop();
        trace(p[0], p[1], sqrDist);
    } */
}

int KDBTree::size(map<string, double> &stats) const {
    int totalSize = 2 * sizeof(int);
    int pageSize = 4 * sizeof(float) + sizeof(int) + sizeof(Node *);
    int directorySize = 4 * sizeof(float) + 2 * sizeof(int) + sizeof(Node *);
    stack<Node *> toVisit({root});
    Node *dir;
    while (!toVisit.empty()) {
        dir = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : dir->contents.value()) {
            if (cn->contents) {
                stats["pages"] += cn->contents->size();
                toVisit.push(cn);
            }
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"];
    return totalSize;
}
