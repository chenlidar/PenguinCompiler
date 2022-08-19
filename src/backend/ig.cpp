#include "ig.hpp"
#include <assert.h>
using namespace IG;
void ConfGraph::addNode(Temp_Temp t) {
    if (TempNodeMap.count(t)) return;
    if (t == 11 || t == 13) return;
    if (t < 0 && !isf || t >= 0 && isf) return;
    Node node(nodecount);
    mynodes.push_back(node);
    TempNodeMap[t] = nodecount;
    mynodes[nodecount].temp.push_back(t);
    nodecount++;
}
int ConfGraph::temp2node(Temp_Temp t) {
    if (!TempNodeMap.count(t)) return -1;
    return TempNodeMap.at(t);
}
void ConfGraph::addEdge(int n1, int n2) {
    if (n1 == n2) return;
    assert(!mynodes[n1].h_adj.count(n2) && !mynodes[n2].h_adj.count(n1));
    mynodes[n1].adj.insert(n2);
    mynodes[n2].adj.insert(n1);
}
void ConfGraph::rmEdge(int n1, int n2) {
    if (n1 == n2) return;
    assert(mynodes[n1].adj.count(n2) && mynodes[n2].adj.count(n1));
    mynodes[n1].adj.erase(n2);
    mynodes[n2].adj.erase(n1);
}
void ConfGraph::addEdge_h(int n1, int n2) {
    if (n1 == n2) return;
    assert(!mynodes[n1].adj.count(n2) && !mynodes[n2].adj.count(n1));
    mynodes[n1].h_adj.insert(n2);
    mynodes[n2].h_adj.insert(n1);
}
void ConfGraph::rmEdge_h(int n1, int n2) {
    if (n1 == n2) return;
    assert(mynodes[n1].h_adj.count(n2) && mynodes[n2].h_adj.count(n1));
    mynodes[n1].h_adj.erase(n2);
    mynodes[n2].h_adj.erase(n1);
}
void ConfGraph::addEdge_mv(int n1, int n2) {
    if (n1 == n2) return;
    mynodes[n1].mv_adj.insert(n2);
    mynodes[n2].mv_adj.insert(n1);
}
void ConfGraph::rmEdge_mv(int n1, int n2) {
    if (n1 == n2) return;
    assert(mynodes[n1].mv_adj.count(n2) && mynodes[n2].mv_adj.count(n1));
    mynodes[n1].mv_adj.erase(n2);
    mynodes[n2].mv_adj.erase(n1);
}
void ConfGraph::p2hEdge(int n1, int n2) {
    assert(mynodes[n1].adj.count(n2) && mynodes[n2].adj.count(n1));
    mynodes[n1].adj.erase(n2);
    mynodes[n2].adj.erase(n1);
    mynodes[n1].h_adj.insert(n2);
    mynodes[n2].h_adj.insert(n1);
}
void ConfGraph::h2pEdge(int n1, int n2) {
    assert(mynodes[n1].h_adj.count(n2) && mynodes[n2].h_adj.count(n1));
    mynodes[n1].h_adj.erase(n2);
    mynodes[n2].h_adj.erase(n1);
    mynodes[n1].adj.insert(n2);
    mynodes[n2].adj.insert(n1);
}
int ConfGraph::Node::degree() { return adj.size(); }
void ConfGraph::addMove(ASM::Move* mv) {
    int n1 = temp2node(mv->dst.at(0));
    int n2 = temp2node(mv->src.at(0));
    if (n1 == -1 || n2 == -1) return;
    addEdge_mv(n1, n2);
}
int ConfGraph::Node::nodeTemp() { return temp.at(0); }
void ConfGraph::Create_ig(LIVENESS::Liveness* live) {
    // first: add node
    for (auto node : *live->flowgraph->nodes()) {
        Temp_TempList* defList = live->flowgraph->FG_def(node);
        Temp_TempList* useList = live->flowgraph->FG_use(node);
        std::set<int>* outList = live->FG_Out(node);
        for (auto temp : *defList) addNode(temp);
        for (auto temp : *useList) addNode(temp);
        for (auto temp : *outList) addNode(temp);
    }
    // second: add edge
    for (auto it : *live->flowgraph->nodes()) {
        std::set<int>* outList = live->FG_Out(it);
        Temp_TempList* defList = live->flowgraph->FG_def(it);
        Temp_TempList* useList = live->flowgraph->FG_use(it);
        if (live->flowgraph->FG_isMove(it)) {
            assert(useList->size() == 1);
            assert(defList->size() == 1);
            int dst = defList->at(0);
            int src = useList->at(0);
            ASM::Move* mvinstr = static_cast<ASM::Move*>((ASM::Instr*)it->nodeInfo());
            addMove(mvinstr);
            for (auto livetemp : *outList) {
                if (src != livetemp) {
                    int n1 = temp2node(dst), n2 = temp2node(livetemp);
                    if (n1 != -1 && n2 != -1) addEdge(n1, n2);
                }
            }

        } else {
            for (auto dst : *defList) {
                for (auto livetemp : *outList) {
                    int n1 = temp2node(dst), n2 = temp2node(livetemp);
                    if (n1 != -1 && n2 != -1) addEdge(n1, n2);
                }
            }
        }
    }
    return;
}
