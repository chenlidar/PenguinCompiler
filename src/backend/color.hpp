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
#include <stack>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
#include "ig.hpp"
namespace COLOR {
class COL_result {
public:
    std::unordered_map<Temp_Temp, Temp_Temp> ColorMap;
    std::set<ASM::Move*> UnionMove;
    std::set<Temp_Temp> SpilledNode;
    COL_result(IG::ConfGraph* _ig, std::unordered_map<Temp_Temp, Temp_Temp>* _stkuse) {
        SpillWorklist = std::set<GRAPH::Node*>();
        FreezeWorklist = std::set<GRAPH::Node*>();
        SimplifyWorklist = std::set<GRAPH::Node*>();
        AliasMap = std::unordered_map<Temp_Temp, Temp_Temp>();
        ColorMap = std::unordered_map<Temp_Temp, Temp_Temp>();
        SpilledNode = std::set<Temp_Temp>();
        UnionMove = std::set<ASM::Move*>();
        ActiveMove = std::set<ASM::Move*>();
        SelectStack = std::stack<GRAPH::Node*>();
        ColorMap = std::unordered_map<Temp_Temp, Temp_Temp>();
        for (int i = 0; i < 15; i++) ColorMap[i] = i;
        ig = _ig;
        stkuse = _stkuse;
        COL_Color();
    }

private:
    const int REGNUM = 13;
    const std::set<Temp_Temp> Precolored
        = std::set<Temp_Temp>({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14});
    std::set<GRAPH::Node*> SpillWorklist;
    std::set<GRAPH::Node*> FreezeWorklist;
    std::set<GRAPH::Node*> SimplifyWorklist;
    std::unordered_map<Temp_Temp, Temp_Temp> AliasMap;
    std::set<ASM::Move*> ActiveMove;
    std::stack<GRAPH::Node*> SelectStack;
    IG::ConfGraph* ig;
    std::unordered_map<Temp_Temp, Temp_Temp>* stkuse;
    Temp_Temp findAlias(Temp_Temp k);
    int NodeTemp(GRAPH::Node* node);
    bool moveRelated(GRAPH::Node* node);
    void makeWorklist();
    void enableMove(GRAPH::Node* node);
    void decrementDegree(GRAPH::Node* node);
    void simplify();
    void addWorklist(Temp_Temp temp);
    void combine(GRAPH::Node* u, GRAPH::Node* v);
    bool george(GRAPH::Node* u, GRAPH::Node* v);
    bool briggs(GRAPH::Node* u, GRAPH::Node* v);
    void unionNode();
    void freezeMove(GRAPH::Node* node);
    void freeze();
    void selectSpill();
    void assignColor();
    void COL_Color();
};
}  // namespace COLOR
#endif