#ifndef __IG
#define __IG
#include <stdio.h>
#include "graph.hpp"
#include "../structure/assem.h"
#include "flowgraph.hpp"
#include "liveness.hpp"
#include <map>
#include <set>
namespace IG {
class ConfGraph {
public:
    std::unordered_map<int, int> TempNodeMap;
    ConfGraph(LIVENESS::Liveness* live) {
        nodecount = 0;
        TempNodeMap = std::unordered_map<int, int>();
        Create_ig(live);
    }
    struct Node {
        std::set<int> adj,h_adj,mv_adj;
        std::vector<int> temp;
        int mykey;
        Node(int _mykey):adj(),h_adj(),mv_adj(),temp(),mykey(_mykey){}
        int degree();
        int nodeTemp();
        bool isreg(){
            return temp[0]<16;
        }
    };
    std::vector<Node> mynodes;
    int nodecount;

    void Create_ig(LIVENESS::Liveness* live);
    int temp2node(Temp_Temp t);
    void addNode(Temp_Temp t);
    void addEdge(int n1,int n2);
    void rmEdge(int n1,int n2);
    void addEdge_h(int n1,int n2);
    void rmEdge_h(int n1,int n2);
    void addEdge_mv(int n1,int n2);
    void rmEdge_mv(int n1,int n2);
    void addMove(ASM::Move* mv);
    void p2hEdge(int n1,int n2);
    void h2pEdge(int n1,int n2);
};
}  // namespace IG
#endif