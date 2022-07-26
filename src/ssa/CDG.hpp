#include "../backend/graph.hpp"
#include "Dtree.hpp"
namespace CDG {
class CDgraph : public GRAPH::Graph {
public:
    CDgraph(GRAPH::Graph* g);
    std::vector<std::vector<int>> CDnode;

private:
    int exitnum;
    DTREE::Dtree* dtree;
    void bfs(int node);
};

}  // namespace CDG