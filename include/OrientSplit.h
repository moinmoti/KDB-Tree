#pragma once

#include <bits/stdc++.h>
#include "SuperNode.h"
#include <chrono>

class OrientNode: public SuperNode {
public:
    vector<SuperNode*> splitLeaf(SuperNode *, Split * = NULL, vector<SuperNode*> = vector<SuperNode*>()) override;
    vector<SuperNode*> splitBranch(SuperNode *, vector<SuperNode*> = vector<SuperNode*>()) override;
};
