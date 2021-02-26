#include "OrientSplit.h"

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

#define all(c) c.begin(),c.end()
#define NUMDIMS 2
#define dist(x1,y1,x2,y2) (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)
#define distManhattan(x1,y1,x2,y2) std::abs(x1-x2)+std::abs(y1-y2)
#define oppDir(d) (d + NUMDIMS) % (NUMDIMS*2)

#define V 0
#define H 1

vector<float> getMedian(vector<vector<float>> pts) {
    if (pts.size() % 2) return pts[pts.size()/2];
    vector<float> l, r;
    l = pts[pts.size()/2-1];
    r = pts[pts.size()/2];
    return vector<float>({(l[0] + r[0])/2, (l[1] + r[1])/2});
}

vector<SuperNode*> OrientNode::splitLeaf(Split *newSplit, vector<SuperNode*> sns) {
    sns = {new OrientNode(), new OrientNode()};

    bool splitCase = !guide->axis;
    sort(all(points.value()), [splitCase](const vector<float> &l, const vector<float> &r) {
        return l[splitCase] < r[splitCase];
    });
    float median = getMedian(points.value())[splitCase];

    newSplit = new Split();
    newSplit->axis = splitCase;
    newSplit->pt = median;
    return SuperNode::splitLeaf(newSplit, sns);
}

vector<SuperNode*> OrientNode::splitBranch(Split *bestSplit, vector<SuperNode*> sns) {
    sns = {new OrientNode(), new OrientNode()};

    bool side = getCenter()[guide->axis] < guide->pt;
    for (auto cs: guide->branches[side]) {
        if (rect[cs->axis] < cs->pt && cs->pt < rect[cs->axis + NUMDIMS]) {
            bestSplit = cs;
            break;
        }
    }

    return SuperNode::splitBranch(bestSplit, sns);
}
