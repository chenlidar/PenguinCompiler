#ifndef __LIVENESS
#define __LIVENESS
#include <cstdio>
#include "graph.hpp"
#include <set>
#include "../structure/assem.h"
namespace LIVENESS {
std::vector<GRAPH::Node*>* Liveness(std::vector<GRAPH::Node*>*);
std::set<int>* FG_Out(GRAPH::Node*);
std::set<int>* FG_In(GRAPH::Node*);
}  // namespace LIVENESS
#endif