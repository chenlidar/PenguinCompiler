#include "ig.hpp"
#include "flowgraph.hpp"
#include <assert.h>
#include <map>
using namespace IG;
GRAPH::Node* ConfGraph::Look_ig(Temp_Temp t) {
    if (t == 11 || t == 13) return nullptr;
    if (TempNodeMap.count(t)) {
        return TempNodeMap[t];
    } else {
        GRAPH::Node* n = this->addNode((void*)(uint64_t)t);
        TempNodeMap.insert(std::make_pair(t, n));
        Movelist.insert(std::make_pair(n, std::set<ASM::Move*>()));
        return n;
    }
}
void ConfGraph::Enter_ig(Temp_Temp t1, Temp_Temp t2) {
    if (t1 == 11 || t1 == 13 || t2 == 11 || t2 == 13) return;
    GRAPH::Node* n1 = Look_ig(t1);
    GRAPH::Node* n2 = Look_ig(t2);
    n1->mygraph->addEdge(n1, n2);
    return;
}

// input flowgraph after liveness analysis (so FG_In and FG_Out are available)

void ConfGraph::Create_ig(LIVENESS::Liveness* live) {
    for (auto it : *live->flowgraph->nodes()) {
        std::set<int>* outList = live->FG_Out(it);
        Temp_TempList* defList = live->flowgraph->FG_def(it);
        Temp_TempList* useList = live->flowgraph->FG_use(it);
        for (auto ite2 : *defList) Look_ig(ite2);
        for (auto ite2 : *useList) Look_ig(ite2);
        for (auto ite2 : *outList) Look_ig(ite2);
        if (live->flowgraph->FG_isMove(it)) {
            assert(useList->size() == 1);
            ASM::Instr* instr = (ASM::Instr*)it->nodeInfo();
            ASM::Move* insmove = static_cast<ASM::Move*>(instr);
            if (defList->at(0) == 11 || defList->at(0) == 13 || useList->at(0) == 11
                || useList->at(0) == 13)
                ;
            else {
                WorklistMove.insert(insmove);
                Movelist[Look_ig(defList->at(0))].insert(insmove);
                Movelist[Look_ig(useList->at(0))].insert(insmove);
                for (auto ite2 : *outList) {
                    if (useList->at(0) != ite2) {
                        if (defList->at(0) != ite2) {
                            Enter_ig(defList->at(0), ite2);
                            Enter_ig(ite2, defList->at(0));
                        }
                    }
                }
            }
        } else {
            for (auto ite1 : *defList) {
                for (auto ite2 : *outList) {
                    if (ite1 != ite2) {
                        Enter_ig(ite1, ite2);
                        Enter_ig(ite2, ite1);
                    }
                }
            }
        }
    }
    return;
}
