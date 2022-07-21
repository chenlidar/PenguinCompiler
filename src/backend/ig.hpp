#ifndef __IG
#define __IG
#include <stdio.h>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
namespace IG {
std::vector<GRAPH::Node*>* Create_ig(std::vector<GRAPH::Node*>*);
std::set<ASM::Move*>* worklistMove();
std::unordered_map<GRAPH::Node*, std::set<ASM::Move*>>* movelist();
std::unordered_map<int, GRAPH::Node*>* tempNodeMap();
}  // namespace IG
#endif