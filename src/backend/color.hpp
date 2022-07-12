/*
 * color.h - Data structures and function prototypes for coloring algorithm
 *             to determine register allocation.
 */
#ifndef __COLOR
#define __COLOR
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unordered_map>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
#include "ig.hpp"
namespace COLOR {
struct COL_result {
    std::unordered_map<Temp_Temp, Temp_Temp>* coloring;
    Temp_TempList* spills;

    COL_result()
        : coloring(new std::unordered_map<Temp_Temp, Temp_Temp>())
        , spills(new Temp_TempList()) {}
};
COL_result* COL_Color(GRAPH::NodeList* ig, std::unordered_map<Temp_Temp, Temp_Temp>* stkuse);
}  // namespace COLOR
#endif