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
    std::set<Temp_Temp>* spills;
    std::set<ASM::Move*>* UnionMove;

    COL_result(std::unordered_map<Temp_Temp, Temp_Temp>* _coloring,
               std::set<Temp_Temp>* _spills, std::set<ASM::Move*>* _UnionMove)
        : coloring(_coloring)
        , spills(_spills)
        , UnionMove(_UnionMove) {}
};
const COL_result* COL_Color(GRAPH::NodeList* ig, std::unordered_map<Temp_Temp, Temp_Temp>* stkuse);
}  // namespace COLOR
#endif