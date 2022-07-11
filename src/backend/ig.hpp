#ifndef __IG
#define __IG
#include <stdio.h>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
namespace IG {
void Enter_ig(Temp_Temp, Temp_Temp);
GRAPH::NodeList* Create_ig(GRAPH::NodeList*);
void Show_ig(FILE*, GRAPH::NodeList*);
GRAPH::Graph* Ig_graph();
void Ig_empty();
}  // namespace IG
#endif