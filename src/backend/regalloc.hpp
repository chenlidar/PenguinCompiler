/* function prototype from regalloc.c */
// given a temp_map that gives assigns registers to temps (with some of them as "Spill")
// Update the input instruction list to generate the final instruction list
#ifndef __REGALLOC
#define __REGALLOC
#include <cstring>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
#include "ig.hpp"
#include "color.hpp"
namespace RA {
ASM::InstrList* RA_RegAlloc(ASM::InstrList* il, int stksize);
}
#endif