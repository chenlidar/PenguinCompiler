#ifndef __CDG_H
#define __CDG_H
#include "../backend/graph.hpp"
#include "Dtree.hpp"
#include "CFG.hpp"
namespace CDG {
class CDgraph : public GRAPH::Graph {
public:
    CDgraph(CFG::CFGraph* g);
    std::vector<std::vector<int>> CDnode;
    DTREE::Dtree* dtree;
    int exitnum;

private:
    void bfs(int node);
};

}  // namespace CDG
#endif