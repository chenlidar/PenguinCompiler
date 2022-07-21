#include "color.hpp"
#include <assert.h>
#include <stack>
using namespace COLOR;
#define REGNUM 13

// Data structure
static const std::set<Temp_Temp> Precolored
    = std::set<Temp_Temp>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14});
static std::set<GRAPH::Node*> SpillWorklist;
static std::set<GRAPH::Node*> FreezeWorklist;
static std::set<GRAPH::Node*> SimplifyWorklist;
static std::unordered_map<Temp_Temp, Temp_Temp> AliasMap;
static std::unordered_map<Temp_Temp, Temp_Temp> ColorMap;
static std::set<Temp_Temp> SpilledNode;
static std::set<ASM::Move*> UnionMove;
static std::set<ASM::Move*> ActiveMove;
static std::stack<GRAPH::Node*> SelectStack;
static const COL_result COLresult = COL_result(&ColorMap, &SpilledNode, &UnionMove);
static Temp_Temp findAlias(Temp_Temp k) {
    if (!AliasMap.count(k)) return k;
    assert(AliasMap[k] != k);
    return AliasMap[k] = findAlias(AliasMap[k]);
}
static int NodeTemp(GRAPH::Node* node) { return (int)(u_int64_t)node->info; }
static void init() {
    SpillWorklist.clear();
    FreezeWorklist.clear();
    SimplifyWorklist.clear();
    AliasMap.clear();
    ColorMap.clear();
    SpilledNode.clear();
    UnionMove.clear();
    ActiveMove.clear();
    while (!SelectStack.empty()) SelectStack.pop();
    for (int i = 0; i < 15; i++) ColorMap[i] = i;
}
static bool moveRelated(GRAPH::Node* node) {
    for (auto instr : IG::movelist()->at(node)) {
        if (ActiveMove.count(instr) || IG::worklistMove()->count(instr)) return true;
    }
    return false;
}
static void makeWorklist(std::vector<GRAPH::Node*>* ig) {
    for (auto node : *ig) {
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
static void enableMove(GRAPH::Node* node) {
    for (auto instr : IG::movelist()->at(node)) {
        if (ActiveMove.count(instr)) {
            ActiveMove.erase(instr);
            IG::worklistMove()->insert(instr);
        }
    }
}
static void decrementDegree(GRAPH::Node* node) {
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
static void simplify() {
    GRAPH::Node* node = *SimplifyWorklist.begin();
    SimplifyWorklist.erase(node);
    SelectStack.push(node);
    for (auto adjnode : *node->succ()) {
        node->mygraph->rmNode(node, adjnode);
        decrementDegree(adjnode);
    }
}
static void addWorklist(Temp_Temp temp) {
    if (Precolored.count(temp)) return;
    GRAPH::Node* node = IG::tempNodeMap()->at(temp);
    if (!moveRelated(node) && node->outDegree() < REGNUM) {
        assert(FreezeWorklist.count(node));
        FreezeWorklist.erase(node);
        SimplifyWorklist.insert(node);
    }
}
static void combine(GRAPH::Node* u, GRAPH::Node* v) {  // u pre,v no || u no,v no
    assert(!Precolored.count(NodeTemp(v)));
    if (FreezeWorklist.count(v)) {
        FreezeWorklist.erase(v);
    } else {
        assert(SpillWorklist.count(v));
        SpillWorklist.erase(v);
    }
    assert(findAlias(NodeTemp(v)) == NodeTemp(v) && findAlias(NodeTemp(u)) == NodeTemp(u));
    AliasMap[NodeTemp(v)] = NodeTemp(u);
    for (auto instr : IG::movelist()->at(v)) {
        if (!ActiveMove.count(instr) && !IG::worklistMove()->count(instr)) continue;
        IG::movelist()->at(u).insert(instr);
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
static bool george(GRAPH::Node* u, GRAPH::Node* v) {
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
static bool briggs(GRAPH::Node* u, GRAPH::Node* v) {
    std::set<GRAPH::Node*> cnt;
    for (auto node : *u->succ()) {
        if (node->outDegree() >= REGNUM || Precolored.count(NodeTemp(node))) cnt.insert(node);
    }
    for (auto node : *v->succ()) {
        if (node->outDegree() >= REGNUM || Precolored.count(NodeTemp(node))) cnt.insert(node);
    }
    return cnt.size() < REGNUM;
}
static void unionNode() {
    ASM::Move* instr = *IG::worklistMove()->begin();
    IG::worklistMove()->erase(instr);
    Temp_Temp x = instr->dst.at(0);
    Temp_Temp y = instr->src.at(0);
    x = findAlias(x);
    y = findAlias(y);
    if (Precolored.count(y)) std::swap(x, y);
    GRAPH::Node* u = IG::tempNodeMap()->at(x);
    GRAPH::Node* v = IG::tempNodeMap()->at(y);
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
static void freezeMove(GRAPH::Node* node) {  // must be a no-precolored node
    for (auto instr : IG::movelist()->at(node)) {
        if (!ActiveMove.count(instr) && !IG::worklistMove()->count(instr)) continue;
        Temp_Temp x = instr->dst.at(0);
        Temp_Temp y = instr->src.at(0);
        GRAPH::Node *u = node, *v;
        if (findAlias(y) == findAlias(NodeTemp(node))) {
            v = IG::tempNodeMap()->at(findAlias(x));
        } else {
            assert(findAlias(x) == findAlias(NodeTemp(node)));
            v = IG::tempNodeMap()->at(findAlias(y));
        }
        // u(node)-----v(adjnode)
        assert(ActiveMove.count(instr));
        assert(!IG::worklistMove()->count(instr));
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
static void freeze() {
    GRAPH::Node* node = *FreezeWorklist.begin();
    assert(!Precolored.count(NodeTemp(node)));
    FreezeWorklist.erase(node);
    SimplifyWorklist.insert(node);
    freezeMove(node);
}
static void selectSpill(std::unordered_map<Temp_Temp, Temp_Temp>* stkuse) {
    for (auto it : SpillWorklist) {
        if (stkuse->count(NodeTemp(it))) continue;
        assert(!Precolored.count(NodeTemp(it)));
        SpillWorklist.erase(it);
        SimplifyWorklist.insert(it);
        freezeMove(it);
        break;
    }
}
static void assignColor() {
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
const COL_result* COLOR::COL_Color(std::vector<GRAPH::Node*>* ig,
                                   std::unordered_map<Temp_Temp, Temp_Temp>* stkuse) {
    init();
    makeWorklist(ig);
    int cnt = 0;
    while (!SimplifyWorklist.empty() || !SpillWorklist.empty() || !FreezeWorklist.empty()
           || !IG::worklistMove()->empty()) {
        if (!SimplifyWorklist.empty())
            simplify();
        else if (!IG::worklistMove()->empty())
            unionNode();
        else if (!FreezeWorklist.empty())
            freeze();
        else if (!SpillWorklist.empty())
            selectSpill(stkuse);
    }
    assignColor();
    return &COLresult;
}
