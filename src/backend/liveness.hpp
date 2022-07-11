#ifndef __LIVENESS
#define __LIVENESS
#include <cstdio>
#include "graph.hpp"
#include "../structure/assem.h"
namespace LIVENESS {
GRAPH::NodeList* Liveness(GRAPH::NodeList*);
Temp_TempList* FG_Out(GRAPH::Node*);
Temp_TempList* FG_In(GRAPH::Node*);
}  // namespace LIVENESS
#endif