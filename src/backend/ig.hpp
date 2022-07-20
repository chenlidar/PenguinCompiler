#ifndef __IG
#define __IG
#include <stdio.h>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
namespace IG {
GRAPH::NodeList* Create_ig(GRAPH::NodeList*);
std::set<ASM::Move*>* worklistMove();
std::unordered_map<GRAPH::Node*, std::set<ASM::Move*>>* movelist();
std::unordered_map<int, GRAPH::Node*>* tempNodeMap();
}  // namespace IG
#endif