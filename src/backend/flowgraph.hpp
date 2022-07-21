#ifndef __FLOWGRAPH
#define __FLOWGRAPH
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../util/temptemp.hpp"
#include "../util/templabel.hpp"
#include "graph.hpp"
#include "../structure/assem.h"
namespace FLOW {
struct UDinfo {
    Temp_TempList uses;
    Temp_TempList defs;
    bool isMove;
    UDinfo(Temp_TempList _uses, Temp_TempList _defs, bool _isMove)
        : uses(_uses)
        , defs(_defs)
        , isMove(_isMove) {}
};
class FlowGraph : public GRAPH::Graph {
public:
    Temp_TempList* FG_def(GRAPH::Node* n);
    Temp_TempList* FG_use(GRAPH::Node* n);
    bool FG_isMove(GRAPH::Node* n);
    FlowGraph(ASM::InstrList* il) {
        UDTable = std::unordered_map<GRAPH::Node*, UDinfo*>();
        LNTable = std::unordered_map<Temp_Label, GRAPH::Node*>();
        FG_AssemFlowGraph(il);
    }
    ~FlowGraph() {
        for (const auto& it : UDTable) delete it.second;
    }

private:
    std::unordered_map<GRAPH::Node*, UDinfo*> UDTable;
    std::unordered_map<Temp_Label, GRAPH::Node*> LNTable;
    void FG_AssemFlowGraph(ASM::InstrList* il);
};

}  // namespace FLOW
#endif