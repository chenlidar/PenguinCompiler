#ifndef __LIVENESS
#define __LIVENESS
#include <cstdio>
#include "graph.hpp"
#include <set>
#include "../structure/assem.h"
namespace LIVENESS {
GRAPH::NodeList* Liveness(GRAPH::NodeList*);
std::set<int>* FG_Out(GRAPH::Node*);
std::set<int>* FG_In(GRAPH::Node*);
}  // namespace LIVENESS
#endif