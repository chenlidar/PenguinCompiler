#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/flowgraph.hpp"
namespace SSA{
    GRAPH::Graph* build(IR::StmList* stmlist);
}