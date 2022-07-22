#include "../backend/graph.hpp"
namespace DTREE {
class Dtree {
public:
    std::vector<std::vector<int>> children;
    Dtree(GRAPH::Graph* _g);

private:
    int cnt;
    GRAPH::Graph* g;
    std::vector<int> dfnum;
    std::vector<int> vertex;
    std::vector<int> parent;
    std::vector<int> samedom;
    std::vector<int> semi;
    std::vector<int> idom;
    std::vector<int> best;
    std::vector<int> ancestor;
    std::vector<std::vector<int>> bucket;
    void dfs(int root, int fa);
    void link(int fa, int node);
    int findLowestSemiAncestor(int node);
};
}  // namespace DTREE