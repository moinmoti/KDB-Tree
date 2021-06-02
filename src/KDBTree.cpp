#include "KDBTree.h"
#include "OrientSplit.h"

using namespace std;
using namespace std::chrono;

#define TRACE
#ifdef TRACE
#define trace(...) __f(#__VA_ARGS__, __VA_ARGS__)
template <typename Arg1>
void __f(const char* name, Arg1 && arg1){
    cerr << name << " : " << arg1 << endl;
}
template <typename Arg1, typename... Args>
void __f(const char* names, Arg1 && arg1, Args &&... args){
    const char* comma = strchr(names+1,','); cerr.write(names,comma-names) << " : " << setw(9) << arg1 << " | "; __f(comma+1, args...);
}
#else
#define trace(...)
#endif

#define NUMDIMS 2
#define all(c) c.begin(),c.end()

void printRect(string Rect, array<float, 4> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(int _leafCap, int _branchCap, array<float, 4> _boundary, string type) {
    leafCap = _leafCap;
    branchCap = _branchCap;
    splitCount = 0;

    if (type == "Orient") root = new OrientNode();
    else cerr << "Invalid Partition Scheme" << endl;
    root->rect = _boundary;
    root->height = 0;
}

KDBTree::~KDBTree() {}

void KDBTree::snapshot() {
    ofstream log("KDBTree.csv");
    stack<SuperNode*> toVisit({root});
    SuperNode *branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        log << branch->height << "," << branch->childNodes->size();
        for (auto p: branch->rect) log << "," << p;
        log << endl;
        for (auto cn: branch->childNodes.value()) {
            if (cn->points) {
                log << cn->height << "," << cn->points->size();
                for (auto p: cn->rect) log << "," << p;
                log << endl;
            }
            else toVisit.push(cn);
        }
    }
    log.close();
}

void KDBTree::branchFission(SuperNode *node) {
    printRect("Node", node->rect);
    vector<SuperNode*> branches = node->splitBranch(root);
    printRect("First Branch", branches[0]->rect);
    printRect("Second Branch", branches[1]->rect);
    for (auto branch: branches) {
        if (branch->childNodes->size() > branchCap) branchFission(branch);
        else root->childNodes->emplace_back(branch);
    }
}

void KDBTree::leafFission(SuperNode *node) {
    vector<SuperNode*> leaves = node->splitLeaf(root);
    for (auto leaf: leaves) {
        if (leaf->points->size() > leafCap) leafFission(leaf);
        else root->childNodes->emplace_back(leaf);
    }
}

void KDBTree::bulkload(string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    vector<array<float, 2>> Points;
    Points.reserve(limit);
    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lat >> lon;
            array pt{lon, lat};
            Points.emplace_back(pt);
            if (++i >= limit) break;
        }
        file.close();
    }
    else cerr << "Data file " << filename << " not found!";

    cerr << "Initiate leaf fission" << endl;
    root->points = Points;
    root->childNodes = vector<SuperNode*>();
    root->splits = vector<Split*>();
    leafFission(root);
    root->height = 1;
    root->points->clear();
    root->points.reset();

    cerr << "Initiate branch fission" << endl;
    while (root->childNodes->size() > branchCap) {
        trace(root->height);
        branchFission(root);
        root->height++;
    }
}

void KDBTree::insertPoint(SuperNode *pn, SuperNode *node, array<float, 2> p) {
    vector<SuperNode*> newNodes;
    if (node->points) {
        node->points->emplace_back(p);
        if (node->points->size() > leafCap) newNodes = node->splitLeaf(pn);
    } else {
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p)) cn++;
        insertPoint(node, *cn, p);
        if (node->childNodes->size() > branchCap) newNodes = node->splitBranch(pn);
    }
    if (node == root) {
        root->childNodes->clear(); // When this is root.
        root->height++;
    } else pn->childNodes->erase(find(all(pn->childNodes.value()), node));
    for (auto cn: newNodes) pn->childNodes->emplace_back(cn);
}

void KDBTree::insertQuery(array<float, 2> p, map<string, double> &res) {
    //vector<float> query = {p[0], p[1], p[0], p[1]};
    insertPoint(root, root, p);
}

void KDBTree::deleteQuery(array<float, 2> p, map<string, double> &res) {
    SuperNode *node = root;
    while (node->childNodes) {
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p)) cn++;
        node = *cn;
    }
    auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end()) node->points->erase(pt);
}

void rangeSearch(SuperNode *node, int &pointCount, array<float, 4> query) {
    if (node->points) pointCount += node->scan(query);
    else {
        for (auto cn: node->childNodes.value())
            if (cn->overlap(query)) rangeSearch(cn, pointCount, query);
    }
}

void KDBTree::rangeQuery(array<float, 4> query, map<string, double> &res) {
    int pointCount = 0;
    rangeSearch(root, pointCount, query);
    trace(pointCount);
}

typedef struct knnPoint {
    array<float, 2> pt;
    double dist = FLT_MAX;
    bool operator<(const knnPoint &second) const {
        return dist < second.dist;
    }
} knnPoint;

typedef struct knnNode {
    SuperNode *sn;
    double dist = FLT_MAX;
    bool operator<(const knnNode &second) const {
        return dist > second.dist;
    }
} knnNode;

void kNNSearch(SuperNode *node, array<float, 4> query, priority_queue<knnPoint, vector<knnPoint>> &knnPts) {
    auto sqrDist = [](array<float, 4> x, array<float ,2> y) {
        return pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2);};
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->points){
                for (auto p: node->points.value()) {
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
                minDist = knnPts.top().dist;
            } else {
                for (auto cn: node->childNodes.value()) {
                    minDist = knnPts.top().dist;
                    dist = cn->minSqrDist(query);
                    if (dist < minDist) {
                        knnNode kn;
                        kn.sn = cn;
                        kn.dist = dist;
                        unseenNodes.push(kn);
                    }
                }
            }
        } else break;
    }
}

void KDBTree::kNNQuery(array<float, 2> p, map<string, double> &res, int k) {
    SuperNode* foundNode;
    array query{p[0], p[1], p[0], p[1]};
    printRect("Query", query);

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    kNNSearch(root, query, knnPts);

    double sqrDist;
    while (!knnPts.empty()) {
        p = knnPts.top().pt;
        sqrDist = knnPts.top().dist;
        knnPts.pop();
        trace(p[0], p[1], sqrDist);
    }
    cerr << endl;
}

int KDBTree::size() const {
    int totalSize = 3*sizeof(int) + sizeof(SuperNode*);
    stack<SuperNode*> toVisit({root});
    SuperNode *branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        for (auto cn: branch->childNodes.value()) {
            if (cn->childNodes) toVisit.push(cn);
            totalSize += cn->size();
        }
    }
    int splitSize = (sizeof(bool) + sizeof(float) + sizeof(Split*))*splitCount;
    totalSize += splitSize;
    cout << "Number of Splits: " << splitCount << endl;
    cout << "Size of splits: " << float(splitSize/1e6) << " MB" <<  endl;
    return totalSize;
}
