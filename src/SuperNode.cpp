#include "SuperNode.h"

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

/////////////////////////////////////////////////////////////////////////////////////////
//Rectangle Methods
/////////////////////////////////////////////////////////////////////////////////////////

vector<float> SuperNode::combineRect(vector<float> r) {
    vector<float> newRect;
    for (int i = 0; i < NUMDIMS; i++)  {
        newRect[i] = min(rect[i], r[i]);
        newRect[i+NUMDIMS] = max(rect[i+NUMDIMS], r[i+NUMDIMS]);
    }
    return newRect;
}

bool SuperNode::overlap(vector<float> r) const {
    for (int i=0; i<NUMDIMS; i++)
        if (rect[i] > r[i+NUMDIMS] || r[i] > rect[i+NUMDIMS])
            return false;
    return true;
}

float SuperNode::edgeOverlap(int d, vector<float> r) const {
    int i = !(d % 2);
    return min(rect[i+NUMDIMS], r[i+NUMDIMS]) - max(rect[i], r[i]);
}

float SuperNode::overlapArea(vector<float> r) const {
    float overlap=1.0;
    for (int i=0; i<NUMDIMS; i++){
        if (rect[i] >= r[i+NUMDIMS] || r[i] >= rect[i+NUMDIMS])
            return 0;
        else overlap *= min(rect[i+NUMDIMS], r[i+NUMDIMS]) - max(rect[i], r[i]);
    }
    return overlap;
}

bool contains(vector<float> r, vector<float> p) {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (r[i] <= p[i]) & (r[i+NUMDIMS] >= p[i]);
    return result;
}

bool SuperNode::containsPt(vector<float> p) const {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (rect[i] <= p[i]) & (rect[i+NUMDIMS] >= p[i]);
    return result;
}

bool SuperNode::contained(vector<float> r) const {
    bool result = true;
    for (int i = 0; i < NUMDIMS; i++)
        result = result & (rect[i] >= r[i]) & (rect[i+NUMDIMS] <= r[i+NUMDIMS]);
  return result;
}

vector<float> SuperNode::getCenter() const {
    return vector<float> {
        (rect[0] + rect[2])/2,
        (rect[1] + rect[3])/2
    };
}

vector<float> SuperNode::getOverlapCenter(vector<float> r) const {
    return vector<float> {
        (max(r[0], rect[0]) + min(r[2], rect[2]))/2,
        (max(r[1], rect[1]) + min(r[3], rect[3]))/2
    };
}

double SuperNode::minManhattanDist(vector<float> r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top && left)
        return distManhattan(rect[0], rect[3], r[2], r[1]);
    if (left && bottom)
        return distManhattan(rect[0], rect[1], r[2], r[3]);
    if (bottom && right)
        return distManhattan(rect[2], rect[1], r[0], r[3]);
    if (right && top)
        return distManhattan(rect[2], rect[3], r[0], r[1]);
    if (left)
        return abs(rect[0] - r[2]);
    if (right)
        return abs(r[0] - rect[2]);
    if (bottom)
        return abs(rect[1] - r[3]);
    if (top)
        return abs(r[1] - rect[3]);
    return 0;
}

double SuperNode::minSqrDist(vector<float> r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top && left)
        return dist(rect[0], rect[3], r[2], r[1]);
    if (left && bottom)
        return dist(rect[0], rect[1], r[2], r[3]);
    if (bottom && right)
        return dist(rect[2], rect[1], r[0], r[3]);
    if (right && top)
        return dist(rect[2], rect[3], r[0], r[1]);
    if (left)
        return (rect[0] - r[2])*(rect[0] - r[2]);
    if (right)
        return (r[0] - rect[2])*(r[0] - rect[2]);
    if (bottom)
        return (rect[1] - r[3])*(rect[1] - r[3]);
    if (top)
        return (r[1] - rect[3])*(r[1] - rect[3]);
    return 0;
}

void SuperNode::createRect(vector<float> r, Split *_split, int side) {
    rect = r;
    rect[_split->axis + side*NUMDIMS] = _split->pt;
}

void SuperNode::setGuide(Split *newSplit, Split *pGuide) {
    if (abs(rect[0] - rect[2]) > abs(rect[1] - rect[3])) {
        if (newSplit->axis == V) guide = pGuide;
        else guide = newSplit;
    } else {
        if (newSplit->axis == V) guide = newSplit;
        else guide = pGuide;
    }
}

//void SuperNode::mergeNode(int mergeDir, SuperNode *expiredNode) {
    //for (int dir = 0; dir < neighbors.size(); dir++) {
        //int oppDir = (dir + NUMDIMS) % (NUMDIMS*2);
        //if (dir == mergeDir) {
            //neighbors[oppDir].erase(expiredNode);
            //continue;
        //}
        //for (auto nb: expiredNode->neighbors[dir]) {
            //if (nb->neighbors[oppDir].find(expiredNode) == nb->neighbors[oppDir].end())
                //cerr << "Node not found!!!" << endl;
            //nb->neighbors[oppDir].erase(expiredNode);
            //nb->neighbors[oppDir].insert(this);
            //neighbors[dir].insert(nb);
        //}
    //}
    //rect = combineRect(expiredNode->rect);
//}

//int SuperNode::deleteNode(SuperNode *pn) {
    //for (int dir = 0; dir < neighbors.size(); dir++) {
        //if (neighbors[dir].size() > 1) continue;
        //int oppDir = (dir + NUMDIMS) % (NUMDIMS*2);
        //for (auto nb: neighbors[dir]) {
            //if (rect[dir] == nb->rect[oppDir]
                    //&& rect[(dir+1)%2] == nb->rect[(dir+1)%2]
                    //&& rect[(dir+1)%2 + 2] == nb->rect[(dir+1)%2 + 2]) {
                //nb->mergeNode(dir, this);
                //pn->children.erase(find(all(pn->children), this));
                //return 1;
            //}
        //}
    //}
    //return 0;
//}

vector<SuperNode*> SuperNode::splitLeaf(Split *newSplit, vector<SuperNode*> sns) {
    bool side =  getCenter()[!newSplit->axis] < guide->pt;
    guide->branches[side].emplace_back(newSplit);
    for (int i = 0; i < sns.size(); i++) {
        sns[i]->height = 0;
        sns[i]->createRect(rect, newSplit, i);
        sns[i]->setGuide(newSplit, guide);
        sns[i]->points = vector<vector<float>>();
    }

    // Splitting points
    for (auto p: (points).value()) {
        if (p[newSplit->axis] < newSplit->pt)
            sns[1]->points->emplace_back(p);
        else if (p[newSplit->axis] > newSplit->pt)
            sns[0]->points->emplace_back(p);
        else {
            if (sns[0]->points->size() < sns[1]->points->size())
                sns[0]->points->emplace_back(p);
            else sns[1]->points->emplace_back(p);
        }
    }
    return sns;
}

vector<SuperNode*> SuperNode::splitBranch(Split *bestSplit, vector<SuperNode*> sns) {
    for (int i = 0; i < sns.size(); i++) {
        sns[i]->height = height;
        sns[i]->createRect(rect, bestSplit, i);
        sns[i]->setGuide(bestSplit, guide);
        sns[i]->childNodes = vector<SuperNode*>();
        sns[i]->childNodes->reserve(childNodes->size());
    }
    for (auto c: childNodes.value())
        sns[c->getCenter()[bestSplit->axis] < bestSplit->pt]->childNodes->emplace_back(c);
    return sns;
}

int overlaps(vector<float> r, vector<float> p) {
    for (int i=0; i<NUMDIMS; i++) {
        if (r[i] > p[i] || p[i] > r[i + NUMDIMS])
            return false;
    }
    return true;
}

vector<vector<float>> SuperNode::scan(vector<float> query) const {
    vector<vector<float>> matchedPoints;
    matchedPoints.reserve(points->size());
    if (contained(query)) return points.value();
    for (auto p: points.value()){
        if (overlaps(query, p)) {
            // cout << "LSI: " << p[0] << "," << p[1] << endl;
            matchedPoints.emplace_back(p);
        }
    }
    return matchedPoints;
}

int SuperNode::getSize() const {
    return sizeof(float)*4 + sizeof(Split*);
}

SuperNode::~SuperNode(){}
