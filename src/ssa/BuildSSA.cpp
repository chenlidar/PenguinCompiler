#include "BuildSSA.hpp"
using namespace SSA;
SSAIR::SSAIR(CANON::Block blocks)
    : CFG::CFGraph(blocks) {
    // DONE: A. stmlist -> graph
    // DONE: B. Dominator tree
    dtree = new DTREE::Dtree(this);
    // DONE: C. Dominance frontiers
    dtree->computeDF();
    // TODO: D. insert phi function
    placePhi();
    // TODO: E. rename temp, output a graph
}
void SSAIR::placePhi(){
    defsites=std::unordered_map<Temp_Temp, std::vector<int>>();
    for(int i=0;i<nodecount;i++){
        for(auto dst:orig[i]){
            defsites[dst].push_back(i);
        }
    }
    for(auto dst:defsites){
        std::unordered_set<int> worklist;
        for(auto it:dst.second)worklist.insert(it);
        while(!worklist.empty()){
            int node=*worklist.begin();
            worklist.erase(node);
            for(auto y:dtree->DF[node]){
                if(!Aphi[node].count(y)){
                    IR::Stm* phifunc=new IR::ExpStm(new IR::Call("$",IR::ExpList()));
                    IR::StmList* stmlist=(IR::StmList*)this->mynodes[node]->nodeInfo();
                    stmlist->tail=new IR::StmList(phifunc,stmlist->tail);
                }
            }
        }
    }
}