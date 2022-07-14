#include "ig.hpp"
#include "flowgraph.hpp"
#include <assert.h>
#include <map>
static GRAPH::Graph* RA_ig;  // info of this graph is a Temp_Temp

void Ig_empty() { RA_ig = new GRAPH::Graph(); }

GRAPH::Graph* IG::Ig_graph() { return RA_ig; }

static std::map<int, GRAPH::Node*> tempNodeMap;
static GRAPH::Node* Look_ig(Temp_Temp t) {
    if (t == 11 || t == 13) return nullptr;
    if (tempNodeMap.count(t)) {
        return tempNodeMap[t];
    } else {
        GRAPH::Node* n = RA_ig->addNode((void*)(uint64_t)t);
        tempNodeMap.insert(std::make_pair(t, n));
        return n;
    }
}

void IG::Enter_ig(Temp_Temp t1, Temp_Temp t2) {
    if (t1 == 11 || t1 == 13 || t2 == 11 || t2 == 13) return;
    GRAPH::Node* n1 = Look_ig(t1);
    GRAPH::Node* n2 = Look_ig(t2);
    // G_addEdge(n1, n2);
    n1->mygraph->addEdge(n1, n2);
    return;
}

// input flowgraph after liveness analysis (so FG_In and FG_Out are available)

GRAPH::NodeList* IG::Create_ig(GRAPH::NodeList* flowgraph) {
    RA_ig = new GRAPH::Graph();
    tempNodeMap.clear();
    for (auto it : *flowgraph) {
        std::set<int>* outList = LIVENESS::FG_Out(it);
        Temp_TempList* defList = FLOW::FG_def(it);
        Temp_TempList* useList = FLOW::FG_use(it);
        for (auto ite2 : *defList) Look_ig(ite2);
        for (auto ite2 : *useList) Look_ig(ite2);
        for (auto ite2 : *outList) Look_ig(ite2);
        if (FLOW::FG_isMove(it)) {
            assert(useList->size() == 1);
            for (auto ite2 : *outList) {
                if (useList->at(0) != ite2) {
                    if (defList->at(0) != ite2) {
                        Enter_ig(defList->at(0), ite2);
                        Enter_ig(ite2, defList->at(0));
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
    return (RA_ig->nodes());
}
