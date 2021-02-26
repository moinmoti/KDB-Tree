#pragma once

#include <bits/stdc++.h>
#include "SuperNode.h"
#include <chrono>

class OrientNode: public SuperNode {
public:
    vector<SuperNode*> splitLeaf(Split* = NULL, vector<SuperNode*> = vector<SuperNode*>()) override;
    vector<SuperNode*> splitBranch(Split* = NULL, vector<SuperNode*> = vector<SuperNode*>()) override;
};
