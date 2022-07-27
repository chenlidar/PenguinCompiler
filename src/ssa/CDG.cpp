#include "CDG.hpp"
#include <queue>
using namespace CDG;
CDgraph::CDgraph(CFG::CFGraph* g) {
    for (auto node : *g->nodes()) {  // build graph G'
        auto newnode = addNode(node);
        newnode->succs = *node->pred();
        newnode->preds = *node->succ();
    }
    exitnum = g->exitnum;
    auto r = addNode(0);  // r node
    addEdge(mynodes[exitnum], r);
    addEdge(mynodes[0], r);
    dtree = new DTREE::Dtree(this, exitnum);
    dtree->computeDF();
    CDnode = std::vector<std::vector<int>>(nodecount, std::vector<int>());
    for (int i = 0; i <= exitnum; i++) { bfs(i); }
}
void CDgraph::bfs(int node) {
    std::vector<bool> vis(this->nodecount, false);
    std::queue<int> q;
    q.push(node);
    while (!q.empty()) {
        int n = q.front();
        q.pop();
        for (auto pre : dtree->DF[n]) {
            CDnode[node].push_back(pre);
            if (!vis[pre]) {
                vis[pre] = true;
                q.push(pre);
            }
        }
    }
}