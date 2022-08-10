#ifndef __COLOR
#define __COLOR
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include <unordered_set>
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
    std::map<Temp_Temp,Temp_Temp> SpilledTemp;
    COL_result(IG::ConfGraph* _ig, std::unordered_set<Temp_Temp>* _stkuse) {
        SpillWorklist = std::set<int>();
        SimplifyWorklist = std::set<int>();
        CombineWorklist= std::set<int>();
        FreezeWorklist= std::set<int>();
        SpilledTemp = std::map<Temp_Temp,Temp_Temp>();
        SelectStack = std::stack<int>();
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
    std::set<int> SpillWorklist;
    std::set<int> SimplifyWorklist;
    std::set<int> CombineWorklist,FreezeWorklist;
    std::stack<int> SelectStack;
    IG::ConfGraph* ig;
    std::unordered_set<Temp_Temp>* stkuse;
    void makeWorklist();
    void simplify();
    void changeWorklist(Temp_Temp temp);
    void combine(int u, int v);
    bool briggs(int u, int v);
    void unionNode();
    void freezeMove(int node);
    void freeze();
    void selectSpill();
    void assignColor();
    void COL_Color();
};
}  // namespace COLOR
#endif