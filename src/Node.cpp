#include "Node.h"

int overlaps(array<float, 4> r, array<float, 2> p) {
    for (int i = 0; i < D; i++) {
        if (r[i] > p[i] || p[i] > r[i + D])
            return false;
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Rectangle Methods
/////////////////////////////////////////////////////////////////////////////////////////

void printNode(string str, array<float, 4> r) {
    cerr << str << ": " << r[0] << " | " << r[1] << " | " << r[2] << " | " << r[3] << endl;
}

bool Node::overlap(array<float, 4> r) const {
    for (int i = 0; i < D; i++)
        if (rect[i] > r[i + D] || r[i] > rect[i + D])
            return false;
    return true;
}

bool Node::containsPt(array<float, 2> p) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] <= p[i]) & (rect[i + D] >= p[i]);
    return result;
}

bool Node::inside(array<float, 4> r) const {
    bool result = true;
    for (int i = 0; i < D; i++)
        result = result & (rect[i] >= r[i]) & (rect[i + D] <= r[i + D]);
    return result;
}

array<float, 2> Node::getCenter() const {
    return array<float, 2>{(rect[0] + rect[2]) / 2, (rect[1] + rect[3]) / 2};
}

double Node::minSqrDist(array<float, 4> r) const {
    bool left = r[2] < rect[0];
    bool right = rect[2] < r[0];
    bool bottom = r[3] < rect[1];
    bool top = rect[3] < r[1];
    if (top) {
        if (left)
            return dist(rect[0], rect[3], r[2], r[1]);
        if (right)
            return dist(rect[2], rect[3], r[0], r[1]);
        return (r[1] - rect[3]) * (r[1] - rect[3]);
    }
    if (bottom) {
        if (left)
            return dist(rect[0], rect[1], r[2], r[3]);
        if (right)
            return dist(rect[2], rect[1], r[0], r[3]);
        return (rect[1] - r[3]) * (rect[1] - r[3]);
    }
    if (left)
        return (rect[0] - r[2]) * (rect[0] - r[2]);
    if (right)
        return (r[0] - rect[2]) * (r[0] - rect[2]);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Node Methods
/////////////////////////////////////////////////////////////////////////////////////////

vector<Record> Node::getPoints() const {
    vector<Record> allPoints;
    if (points)
        allPoints = points.value();
    else {
        for (auto cn : contents.value()) {
            vector<Record> tempPoints = cn->getPoints();
            allPoints.insert(allPoints.end(), all(tempPoints));
        }
    }
    return allPoints;
}

Split *Node::getSplit() const {
    bool axis;
    vector<Record> allPoints = getPoints();
    if (TYPE == CYCLIC) {
        axis = !splitDim;
    } else if (TYPE == SPREAD) {
        array<float, 2> low({180, 90}), high({-180, -90});
        for (auto p : allPoints) {
            if (p.data[0] < low[0])
                low[0] = p.data[0];
            if (p.data[1] < low[1])
                low[1] = p.data[1];
            if (p.data[0] > high[0])
                high[0] = p.data[0];
            if (p.data[1] > high[1])
                high[1] = p.data[1];
        }
        axis = (high[0] - low[0]) < (high[1] - low[1]);
    } else
        cerr << "Error: Invalid TYPE!!!" << endl;
    sort(all(allPoints),
        [axis](const Record &l, const Record &r) { return l.data[axis] < r.data[axis]; });
    Split *split = new Split();
    split->axis = axis;
    split->pt = allPoints[allPoints.size() / 2].data[axis];
    return split;
}

int Node::scan(array<float, 4> query) const {
    int totalPoints = 0;
    if (inside(query))
        return points->size();
    for (auto p : points.value())
        if (overlaps(query, p.data))
            totalPoints++;
    return totalPoints;
}

int Node::size() const {
    int rectSize = sizeof(float) * 4;
    int typeSize = 0;
    if (points)
        typeSize = sizeof(vector<array<float, 2>>);
    else
        typeSize = sizeof(vector<Node *>) + contents->size() * sizeof(Node *);
    int totalSize = typeSize + rectSize;
    return totalSize;
}

vector<Node *> Node::splitDirectory(int &writes, Split *split) {
    vector<Node *> dirs = {new Node(), new Node()};
    if (split == NULL)
        split = getSplit();
    for (int i = 0; i < dirs.size(); i++) {
        dirs[i]->height = height;
        dirs[i]->rect = rect;
        dirs[i]->rect[split->axis + !i * D] = split->pt;
        dirs[i]->contents = vector<Node *>();
        dirs[i]->splitDim = split->axis;
    }
    for (auto cn : contents.value()) {
        if (cn->rect[split->axis + D] <= split->pt)
            dirs[0]->contents->emplace_back(cn);
        else if (cn->rect[split->axis] >= split->pt)
            dirs[1]->contents->emplace_back(cn);
        else {
            vector<Node *> newNodes;
            if (cn->points) {
                newNodes = cn->splitPage(split);
                writes += 3;
            } else
                newNodes = cn->splitDirectory(writes, split);
            for (auto node : newNodes)
                dirs[node->getCenter()[split->axis] > split->pt]->contents->emplace_back(node);
        }
    }

    contents->clear();
    return dirs;
}

vector<Node *> Node::splitPage(Split *split) {
    vector<Node *> pages = {new Node(), new Node()};
    if (split == NULL)
        split = getSplit();
    for (int i = 0; i < pages.size(); i++) {
        pages[i]->height = 0;
        pages[i]->rect = rect;
        pages[i]->rect[split->axis + !i * D] = split->pt;
        pages[i]->points = vector<Record>();
        pages[i]->splitDim = split->axis;
    }

    // Splitting points
    for (auto p : (points).value()) {
        if (p.data[split->axis] < split->pt)
            pages[0]->points->emplace_back(p);
        else if (p.data[split->axis] > split->pt)
            pages[1]->points->emplace_back(p);
        else {
            if (pages[0]->points->size() < pages[1]->points->size())
                pages[0]->points->emplace_back(p);
            else
                pages[1]->points->emplace_back(p);
        }
    }
    return pages;
}

Node::~Node() {
    if (points) {
        points->clear();
        points.reset();
    } else {
        contents->clear();
        contents.reset();
    }
}
