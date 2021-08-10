#include "KDBTree.h"
#include "OrientSplit.h"

using namespace std;
using namespace std::chrono;

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1> void __f(const char *name, Arg1 &&arg1) {
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char *names, Arg1 &&arg1, Args &&... args) {
    const char *comma = strchr(names + 1, ',');
    cerr.write(names, comma - names) << " : " << setw(9) << arg1 << " | ";
    __f(comma + 1, args...);
}
#else
#define trace(...)
#endif

#define NUMDIMS 2
#define all(c) c.begin(), c.end()

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(int _leafCap, int _branchCap, array<float, 4> _boundary, string type) {
    leafCap = _leafCap;
    branchCap = _branchCap;

    if (type == "Orient")
        root = new OrientNode();
    else
        cerr << "Invalid Partition Scheme" << endl;
    root->rect = _boundary;
    root->height = 0;
}

KDBTree::~KDBTree() {}

void KDBTree::snapshot() const {
    ofstream log("KDBTree.csv");
    stack<SuperNode *> toVisit({root});
    SuperNode *branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        log << branch->height << "," << branch->childNodes->size();
        for (auto p : branch->rect)
            log << "," << p;
        log << endl;
        for (auto cn : branch->childNodes.value()) {
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

void KDBTree::branchFission(SuperNode *node) {
    vector<SuperNode *> branches = node->splitBranch(root);
    for (auto branch : branches) {
        if (branch->childNodes->size() > branchCap) {
            branchFission(branch);
            delete branch;
        } else
            root->childNodes->emplace_back(branch);
    }
}

void KDBTree::leafFission(SuperNode *node) {
    vector<SuperNode *> leaves = node->splitLeaf(root);
    for (auto leaf : leaves) {
        if (leaf->points->size() > leafCap) {
            leafFission(leaf);
            delete leaf;
        } else
            root->childNodes->emplace_back(leaf);
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
            buf >> id >> lat >> lon;
            array pt{lon, lat};
            Points.emplace_back(pt);
            if (++i >= limit)
                break;
        }
        file.close();
    } else
        cerr << "Data file " << filename << " not found!";

    cout << "Initiate leaf fission" << endl;
    root->points = Points;
    root->childNodes = vector<SuperNode *>();
    root->splits = vector<Split *>();
    leafFission(root);
    root->height = 1;
    root->points->clear();
    root->points.reset();

    cout << "Initiate branch fission" << endl;
    while (root->childNodes->size() > branchCap) {
        branchFission(root);
        root->height++;
    }
}

void KDBTree::insertPoint(SuperNode *pn, SuperNode *node, array<float, 2> p) {
    vector<SuperNode *> newNodes;
    if (node->points) {
        node->points->emplace_back(p);
        if (node->points->size() > leafCap)
            newNodes = node->splitLeaf(pn);
    } else {
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        insertPoint(node, *cn, p);
        if (node->childNodes->size() > branchCap)
            newNodes = node->splitBranch(pn);
    }
    if (!newNodes.empty()) {
        if (node == root) {
            root->childNodes->clear();
            root->height++;
        } else {
            pn->childNodes->erase(find(all(pn->childNodes.value()), node));
            delete node;
        }
        for (auto cn : newNodes)
            pn->childNodes->emplace_back(cn);
    }
}

void KDBTree::insertQuery(array<float, 2> p, map<string, double> &stats) {
    insertPoint(root, root, p);
}

void KDBTree::deleteQuery(array<float, 2> p, map<string, double> &stats) {
    SuperNode *node = root;
    while (node->childNodes) {
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p))
            cn++;
        node = *cn;
    }
    auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end())
        node->points->erase(pt);
}

void rangeSearch(SuperNode *node, int &pointCount, array<float, 4> query,
                 map<string, double> &stats) {
    if (node->points) {
        stats["io"]++;
        // high_resolution_clock::time_point startTime = high_resolution_clock::now();
        // pointCount += node->scan(query);
        /* stats["scanTime"] +=
            duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
    } else {
        for (auto cn : node->childNodes.value())
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
    double dist = FLT_MAX;
    bool operator<(const knnPoint &second) const { return dist < second.dist; }
} knnPoint;

typedef struct knnNode {
    SuperNode *sn;
    double dist = FLT_MAX;
    bool operator<(const knnNode &second) const { return dist > second.dist; }
} knnNode;

void kNNSearch(SuperNode *node, array<float, 4> query,
               priority_queue<knnPoint, vector<knnPoint>> &knnPts, map<string, double> &stats) {
    auto sqrDist = [](array<float, 4> x, array<float, 2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);
    };
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    // high_resolution_clock::time_point startTime;
    while (!unseenNodes.empty()) {
        // startTime = high_resolution_clock::now();
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        /* stats["explore"] +=
            duration_cast<microseconds>(high_resolution_clock::now() - startTime).count(); */
        if (dist < minDist) {
            if (node->points) {
                // startTime = high_resolution_clock::now();
                for (auto p : node->points.value()) {
                    minDist = knnPts.top().dist;
                    dist = sqrDist(query, p);
                    if (dist < minDist) {
                        knnPoint kPt;
                        kPt.pt = p;
                        kPt.dist = dist;
                        knnPts.pop();
                        knnPts.push(kPt);
                        // stats["heapAccess"]++;
                    }
                    // stats["scanCount"]++;
                }
                stats["io"]++;
                /* stats["scan"] +=
                    duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                 */
            } else {
                // startTime = high_resolution_clock::now();
                for (auto cn : node->childNodes.value()) {
                    minDist = knnPts.top().dist;
                    dist = cn->minSqrDist(query);
                    if (dist < minDist) {
                        knnNode kn;
                        kn.sn = cn;
                        kn.dist = dist;
                        unseenNodes.push(kn);
                    }
                }
                /* stats["explore"] +=
                    duration_cast<microseconds>(high_resolution_clock::now() - startTime).count();
                 */
            }
        } else
            break;
    }
}

void KDBTree::kNNQuery(array<float, 2> p, map<string, double> &stats, int k) {
    SuperNode *foundNode;
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
    int pageSize = 4 * sizeof(float) + sizeof(int) + sizeof(SuperNode *);
    int directorySize = 4 * sizeof(float) + 2 * sizeof(int) + sizeof(SuperNode *);
    int splitSize = 2 * sizeof(float) + sizeof(bool);
    stack<SuperNode *> toVisit({root});
    SuperNode *branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        stats["directories"]++;
        for (auto cn : branch->childNodes.value()) {
            if (cn->childNodes) {
                stats["pages"] += cn->childNodes->size();
                stats["splits"] += cn->splits->size();
                toVisit.push(cn);
            }
        }
    }
    totalSize += pageSize * stats["pages"] + directorySize * stats["directories"] +
                 splitSize * stats["splits"];
    return totalSize;
}
