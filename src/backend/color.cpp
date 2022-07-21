#include "color.hpp"
#include <assert.h>
using namespace COLOR;
Temp_Temp COL_result::findAlias(Temp_Temp k) {
    if (!AliasMap.count(k)) return k;
    assert(AliasMap[k] != k);
    return AliasMap[k] = findAlias(AliasMap[k]);
}
int COL_result::NodeTemp(GRAPH::Node* node) { return (int)(u_int64_t)node->info; }
bool COL_result::moveRelated(GRAPH::Node* node) {
    for (auto instr : ig->Movelist.at(node)) {
        if (ActiveMove.count(instr) || ig->WorklistMove.count(instr)) return true;
    }
    return false;
}
void COL_result::makeWorklist() {
    for (auto node : *ig->nodes()) {
        if (Precolored.count(NodeTemp(node))) continue;
        if (node->outDegree() >= REGNUM) {
            SpillWorklist.insert(node);
        } else if (moveRelated(node)) {
            FreezeWorklist.insert(node);
        } else {
            SimplifyWorklist.insert(node);
        }
    }
}
void COL_result::enableMove(GRAPH::Node* node) {
    for (auto instr : ig->Movelist.at(node)) {
        if (ActiveMove.count(instr)) {
            ActiveMove.erase(instr);
            ig->WorklistMove.insert(instr);
        }
    }
}
void COL_result::decrementDegree(GRAPH::Node* node) {
    if (Precolored.count(NodeTemp(node))) return;
    if (node->outDegree() == REGNUM - 1) {
        enableMove(node);
        for (auto adjnode : *node->succ()) enableMove(adjnode);
        SpillWorklist.erase(node);
        if (moveRelated(node))
            FreezeWorklist.insert(node);
        else { SimplifyWorklist.insert(node); }
    }
}
void COL_result::simplify() {
    GRAPH::Node* node = *SimplifyWorklist.begin();
    SimplifyWorklist.erase(node);
    SelectStack.push(node);
    for (auto adjnode : *node->succ()) {
        node->mygraph->rmNode(node, adjnode);
        decrementDegree(adjnode);
    }
}
void COL_result::addWorklist(Temp_Temp temp) {
    if (Precolored.count(temp)) return;
    GRAPH::Node* node = ig->TempNodeMap.at(temp);
    if (!moveRelated(node) && node->outDegree() < REGNUM) {
        assert(FreezeWorklist.count(node));
        FreezeWorklist.erase(node);
        SimplifyWorklist.insert(node);
    }
}
void COL_result::combine(GRAPH::Node* u, GRAPH::Node* v) {  // u pre,v no || u no,v no
    assert(!Precolored.count(NodeTemp(v)));
    if (FreezeWorklist.count(v)) {
        FreezeWorklist.erase(v);
    } else {
        assert(SpillWorklist.count(v));
        SpillWorklist.erase(v);
    }
    assert(findAlias(NodeTemp(v)) == NodeTemp(v) && findAlias(NodeTemp(u)) == NodeTemp(u));
    AliasMap[NodeTemp(v)] = NodeTemp(u);
    for (auto instr : ig->Movelist.at(v)) {
        if (!ActiveMove.count(instr) && !ig->WorklistMove.count(instr)) continue;
        ig->Movelist.at(u).insert(instr);
    }
    // enableMove(v);
    for (auto it : *v->succ()) {
        it->mygraph->addEdge(it, u);
        it->mygraph->addEdge(u, it);
        it->mygraph->rmNode(v, it);
        decrementDegree(it);  // maybe a precolor
    }
    if (u->outDegree() >= REGNUM && FreezeWorklist.count(u)) {  // u may already in SpillWorklist
        FreezeWorklist.erase(u);
        SpillWorklist.insert(u);
    }
}
bool COL_result::george(GRAPH::Node* u, GRAPH::Node* v) {
    assert(!Precolored.count(NodeTemp(v)));
    for (auto adjnode : *v->succ()) {
        if (adjnode->outDegree() < REGNUM || Precolored.count(NodeTemp(adjnode))
            || adjnode->succ()->count(u))
            continue;
        else
            return false;
    }
    return true;
}
bool COL_result::briggs(GRAPH::Node* u, GRAPH::Node* v) {
    std::set<GRAPH::Node*> cnt;
    for (auto node : *u->succ()) {
        if (node->outDegree() >= REGNUM || Precolored.count(NodeTemp(node))) cnt.insert(node);
    }
    for (auto node : *v->succ()) {
        if (node->outDegree() >= REGNUM || Precolored.count(NodeTemp(node))) cnt.insert(node);
    }
    return cnt.size() < REGNUM;
}
void COL_result::unionNode() {
    ASM::Move* instr = *ig->WorklistMove.begin();
    ig->WorklistMove.erase(instr);
    Temp_Temp x = instr->dst.at(0);
    Temp_Temp y = instr->src.at(0);
    x = findAlias(x);
    y = findAlias(y);
    if (Precolored.count(y)) std::swap(x, y);
    GRAPH::Node* u = ig->TempNodeMap.at(x);
    GRAPH::Node* v = ig->TempNodeMap.at(y);
    /* 1. x pre,y pre
     * 2. x pre,y no
     * 3. x no,y pre (not exist)
     * 4. x no,y no
     */
    if (x == y) {  // 1,4, x may become not moverelate
        UnionMove.insert(instr);
        addWorklist(x);
    } else if (Precolored.count(y) || v->succ()->count(u)) {  // 1 || 2
        addWorklist(x);
        addWorklist(y);
    } else if ((Precolored.count(x) && george(u, v))
               || (!Precolored.count(x) && briggs(u, v))) {  // 2 || 4
        UnionMove.insert(instr);
        combine(u, v);
        addWorklist(x);
    } else
        ActiveMove.insert(instr);
}
void COL_result::freezeMove(GRAPH::Node* node) {  // must be a no-precolored node
    for (auto instr : ig->Movelist.at(node)) {
        if (!ActiveMove.count(instr) && !ig->WorklistMove.count(instr)) continue;
        Temp_Temp x = instr->dst.at(0);
        Temp_Temp y = instr->src.at(0);
        GRAPH::Node *u = node, *v;
        if (findAlias(y) == findAlias(NodeTemp(node))) {
            v = ig->TempNodeMap.at(findAlias(x));
        } else {
            assert(findAlias(x) == findAlias(NodeTemp(node)));
            v = ig->TempNodeMap.at(findAlias(y));
        }
        // u(node)-----v(adjnode)
        assert(ActiveMove.count(instr));
        assert(!ig->WorklistMove.count(instr));
        ActiveMove.erase(instr);
        // precolored node do nothing
        if (Precolored.count(NodeTemp(v)))
            continue;
        else if (!moveRelated(v) && v->outDegree() < REGNUM) {  // remove nomove node to simplify
            assert(FreezeWorklist.count(v));
            FreezeWorklist.erase(v);
            SimplifyWorklist.insert(v);
        }
    }
}
void COL_result::freeze() {
    GRAPH::Node* node = *FreezeWorklist.begin();
    assert(!Precolored.count(NodeTemp(node)));
    FreezeWorklist.erase(node);
    SimplifyWorklist.insert(node);
    freezeMove(node);
}
void COL_result::selectSpill() {
    for (auto it : SpillWorklist) {
        if (stkuse->count(NodeTemp(it))) continue;
        assert(!Precolored.count(NodeTemp(it)));
        SpillWorklist.erase(it);
        SimplifyWorklist.insert(it);
        freezeMove(it);
        break;
    }
}
void COL_result::assignColor() {
    while (!SelectStack.empty()) {
        GRAPH::Node* node = SelectStack.top();
        SelectStack.pop();
        assert(!Precolored.count(NodeTemp(node)));
        std::set<Temp_Temp> okcolor = Precolored;
        node->mygraph->reverseNode(node);
        for (auto adjnode : *node->succ()) {
            if (ColorMap.count(findAlias(NodeTemp(adjnode)))) {
                okcolor.erase(ColorMap[findAlias(NodeTemp(adjnode))]);
            }
        }
        if (okcolor.empty()) {
            SpilledNode.insert(NodeTemp(node));
        } else {
            ColorMap[NodeTemp(node)] = *okcolor.begin();
        }
    }
    for (auto node : AliasMap) {
        assert(node.first != findAlias(node.first));
        ColorMap[node.first] = ColorMap[findAlias(node.first)];
    }
}
void COL_result::COL_Color() {
    makeWorklist();
    int cnt = 0;
    while (!SimplifyWorklist.empty() || !SpillWorklist.empty() || !FreezeWorklist.empty()
           || !ig->WorklistMove.empty()) {
        if (!SimplifyWorklist.empty())
            simplify();
        else if (!ig->WorklistMove.empty())
            unionNode();
        else if (!FreezeWorklist.empty())
            freeze();
        else if (!SpillWorklist.empty())
            selectSpill();
    }
    assignColor();
    return;
}
