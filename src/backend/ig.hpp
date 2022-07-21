#ifndef __IG
#define __IG
#include <stdio.h>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
namespace IG {
class ConfGraph : public GRAPH::Graph {
public:
    std::set<ASM::Move*> WorklistMove;
    std::unordered_map<GRAPH::Node*, std::set<ASM::Move*>> Movelist;
    std::unordered_map<int, GRAPH::Node*> TempNodeMap;
    ConfGraph(LIVENESS::Liveness* live) {
        WorklistMove = std::set<ASM::Move*>();
        Movelist = std::unordered_map<GRAPH::Node*, std::set<ASM::Move*>>();
        TempNodeMap = std::unordered_map<int, GRAPH::Node*>();
        Create_ig(live);
    }

private:
    void Create_ig(LIVENESS::Liveness* live);
    GRAPH::Node* Look_ig(Temp_Temp t);
    void Enter_ig(Temp_Temp t1, Temp_Temp t2);
};
}  // namespace IG
#endif