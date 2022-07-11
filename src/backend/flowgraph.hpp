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
Temp_TempList* FG_def(GRAPH::Node* n);
Temp_TempList* FG_use(GRAPH::Node* n);
bool FG_isMove(GRAPH::Node* n);
GRAPH::Graph* FG_AssemFlowGraph(ASM::InstrList* il);
}  // namespace FLOW
#endif