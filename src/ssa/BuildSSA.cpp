#include "BuildSSA.hpp"
using namespace SSA;
SSAIR::SSAIR(CANON::Block blocks)
    : CFG::CFGraph(blocks) {
    // DONE: A. stmlist -> graph
    // DONE: B. Dominator tree
    dtree = new DTREE::Dtree(this);
    // DONE: C. Dominance frontiers
    dtree->computeDF();
    // DONE: D. insert phi function
    placePhi();
    // TODO: E. rename temp, output a graph

}
void SSAIR::placePhi() {
    defsites = std::unordered_map<Temp_Temp, std::vector<int>>();
    for (int i = 0; i < nodecount; i++) {
        for (auto dst : orig[i]) { defsites[dst].push_back(i); }
    }
    std::unordered_set<int> worklist;
    for (auto dst : defsites) {
        worklist.clear();
        for (auto node : dst.second) worklist.insert(node);
        while (!worklist.empty()) {
            int node = *worklist.begin();
            worklist.erase(node);
            for (auto y : dtree->DF[node]) {
                if (Aphi[y].count(dst.first)) continue;
                IR::Stm* phifunc
                    = new IR::Move(new IR::Temp(dst.first),
                                   new IR::Call("$", IR::ExpList(mynodes[y]->pred()->size(),
                                                                 new IR::Temp(dst.first))));
                IR::StmList* stmlist = (IR::StmList*)this->mynodes[y]->nodeInfo();
                stmlist->tail = new IR::StmList(phifunc, stmlist->tail);
                Aphi[y].insert(dst.first);
                if (!orig[y].count(dst.first)) {
                    worklist.insert(y);
                    orig[y].insert(dst.first);
                }
            }
        }
    }
}