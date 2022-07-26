#pragma once
#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/flowgraph.hpp"
#include "../backend/canon.hpp"
#include "CFG.hpp"
#include "./Dtree.hpp"
namespace SSA {
class SSAIR : public CFG::CFGraph {
public:
    SSAIR(CANON::Block blocks);
    CANON::Block ssa2ir();
    std::unordered_map<int, std::unordered_map<Temp_Temp, IR::StmList*>> Aphi;

private:
    DTREE::Dtree* dtree;
    std::unordered_map<Temp_Temp, std::vector<int>> defsites;
    std::unordered_map<Temp_Temp, std::stack<Temp_Temp>> stk;
    void placePhi();
    void rename();
    void rename(int node);
    void edge_split();
    // Optimizer opt;
};
}  // namespace SSA
