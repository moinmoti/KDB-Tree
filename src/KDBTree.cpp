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
    const char* comma = strchr(names+1,','); cerr.write(names,comma-names) << " : " << arg1 << " | "; __f(comma+1, args...);
}
#else
#define trace(...)
#endif

#define NUMDIMS 2
#define all(c) c.begin(),c.end()

void printRect(string Rect, vector<float> r) {
    cerr << Rect << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

KDBTree::KDBTree(int _leafCap, int _branchCap, vector<float> _boundary, string type) {
    leafCap = _leafCap;
    branchCap = _branchCap;
    splitCount = 0;

    SuperNode *firstLeaf;
    if (type == "Orient") {
        root = new OrientNode();
        firstLeaf = new OrientNode();
    } else cerr << "Invalid Partition Scheme" << endl;

    root->rect = _boundary;
    root->height = 1;
    root->childNodes = vector<SuperNode*>();

    firstLeaf->rect = root->rect;
    firstLeaf->height = 0;
    firstLeaf->points = vector<vector<float>>();

    Split *split = new Split();
    split->axis = (root->rect[2]-root->rect[0]) > (root->rect[3] - root->rect[1]);
    split->pt = root->rect[split->axis];
    root->guide = split;
    firstLeaf->guide = split;
    root->childNodes->emplace_back(firstLeaf);
}

KDBTree::~KDBTree() {}

void KDBTree::snapshot() {
    ofstream log("KDBTree.csv");
    stack<SuperNode*> toVisit({root});
    SuperNode* branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        log << branch->height << "," << branch->childNodes->size();
        for (auto p: branch->rect) log << "," << p;
        log << endl;
        for (auto cn: branch->childNodes.value()) {
            if (cn->height > 0) toVisit.push(cn);
            else {
                log << cn->height << "," << cn->points->size();
                for (auto p: cn->rect) log << "," << p;
                log << endl;
            }
        }
    }
    log.close();
}

/*
void KDBTree::fullSnapshot(vector<float> query) {
    ofstream log("fullSnapshot.csv");
    int i = 1;
    for (auto pn: parentNodes) {
        log << "0," << pn->children.size();
        for (auto p: pn->rect) log << "," << p;
        log << endl;
        for (auto cn: pn->children) {
            //log << i << "," << cn->points.size();
            for (auto p: cn->rect) log << "," << p;
            log << endl;
            for (auto h: cn->hubs) {
                log << ">> ";
                for (auto p: h.second->rect) log << "," << p;
                log << endl;
            }
        }
        i++;
    }
    for (auto p: query) log << p << ',';
    log << "\n" << "**************************************************" << "\n" << endl;
    log.close();
}
*/

void KDBTree::load(string prefix, string filename, long limit) {
    string line;
    ifstream file(filename);

    int i = 0;
    map<string, double> res;

    trace(leafCap, branchCap);

    if (file.is_open()) {
        getline(file, line);
        while (getline(file, line)) {
            int id;
            float lat, lon;
            istringstream buf(line);
            buf >> id >> lat >> lon;
            vector<float> point = {lon, lat};
            insertQuery(point, res);
            if (++i >= limit) break;
            //snapshot();
        }
        file.close();
    }
    else cerr << "Data file " << filename << " not found!";

    snapshot();
}

void KDBTree::insertPoint(SuperNode *pn, SuperNode *node, vector<float> p) {
    vector<SuperNode*> newNodes;
    if (node->height > 0){
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p)) cn++;
        insertPoint(node, *cn, p);
        if (node->childNodes->size() > branchCap) newNodes = node->splitBranch();
    } else {
        node->points->emplace_back(p);
        if (node->points->size() > leafCap) newNodes = node->splitLeaf();
    }
    if (!newNodes.empty()){
        if (node == root) pn->childNodes->clear();
        else pn->childNodes->erase(find(all(pn->childNodes.value()), node));
        for (auto cn: newNodes) pn->childNodes->emplace_back(cn);
    }
}

void KDBTree::insertQuery(vector<float> p, map<string, double> &res) {
    //vector<float> query = {p[0], p[1], p[0], p[1]};
    insertPoint(root, root, p);
}

void KDBTree::deleteQuery(vector<float> p, map<string, double> &res) {
    SuperNode *node = root;
    while (node->height == 0) {
        auto cn = node->childNodes->begin();
        while (!(*cn)->containsPt(p)) cn++;
        node = *cn;
    }
    auto pt = find(all(node->points.value()), p);
    if (pt != node->points->end()) node->points->erase(pt);
}

void rangeSearch(SuperNode *node, int &pointCount, vector<float> query) {
    if (node->height == 0) pointCount += node->scan(query).size();
    else {
        for (auto cn: node->childNodes.value())
            if (cn->overlap(query)) rangeSearch(cn, pointCount, query);
    }
}

void KDBTree::rangeQuery(float *p1, float *p2, map<string, double> &res) {

    float lb[2], hb[2];
    SuperNode* foundNode;

    lb[0] = min(p1[0], p2[0]);
    lb[1] = min(p1[1], p2[1]);
    hb[0] = max(p1[0], p2[0]);
    hb[1] = max(p1[1], p2[1]);

    vector<float> query = {lb[0], lb[1], hb[0], hb[1]};
    int pointCount = 0;
    rangeSearch(root, pointCount, query);
    trace(pointCount);
}

typedef struct knnPoint {
    vector<float> pt;
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

void kNNSearch(SuperNode* node, vector<float> query, priority_queue<knnPoint, vector<knnPoint>> &knnPts) {
    auto sqrtDist = [](vector<float> x, vector<float> y) {
        return sqrt(pow((x[0] - y[0]), 2) + pow((x[1] - y[1]), 2));};
    priority_queue<knnNode, vector<knnNode>> unseenNodes;
    unseenNodes.emplace(knnNode{node, node->minSqrDist(query)});
    double dist, minDist;
    while (!unseenNodes.empty()) {
        node = unseenNodes.top().sn;
        dist = unseenNodes.top().dist;
        unseenNodes.pop();
        minDist = knnPts.top().dist;
        if (dist < minDist) {
            if (node->height == 0){
                for (auto p: node->points.value()) {
                    minDist = knnPts.top().dist;
                    dist = sqrtDist(query, p);
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
                    dist =cn->minSqrDist(query);
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

void KDBTree::kNNQuery(vector<float> p, map<string, double> &res, int k) {
    SuperNode* foundNode;
    vector<float> query = {p[0], p[1], p[0], p[1]};

    vector<knnPoint> tempPts(k);
    priority_queue<knnPoint, vector<knnPoint>> knnPts(all(tempPts));
    kNNSearch(root, query, knnPts);

    //double dist;
    //cerr << "KDBTree:" << endl;
    //while (!knnPts.empty()) {
        //p = knnPts.top().pt;
        //dist = knnPts.top().dist;
        //knnPts.pop();
        //trace(p[0], p[1], dist);
    //}
}

int KDBTree::getSize() {
    int size = 0;
    vector<int> nCount;
    stack<SuperNode*> toVisit({root});
    SuperNode* branch;
    while (!toVisit.empty()) {
        branch = toVisit.top();
        toVisit.pop();
        for (auto cn: branch->childNodes.value()) {
            if (cn->height > 0) toVisit.push(cn);
            size += cn->getSize();
        }
    }
    size += (sizeof(bool) + sizeof(float) + sizeof(Split*))*splitCount;
    cout << "Number of Splits: " << splitCount << endl;
    cout << "Size of splits: " << float(((sizeof(bool) + sizeof(float) + sizeof(Split*))*splitCount)/1e6) << " MB" <<  endl;
    return size;
}
