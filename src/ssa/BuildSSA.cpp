#include "BuildSSA.hpp"
using namespace SSA;
SSAIR::SSAIR(IR::StmList* stmlist)
    : CFG::CFGraph(stmlist) {
    // DONE: A. stmlist -> graph
    // DONE: B. Dominator tree
    dtree = new DTREE::Dtree(this);
    // TODO: C. Dominance frontiers

    // TODO: D. insert phi function
    // TODO: E. rename temp, output a graph
}