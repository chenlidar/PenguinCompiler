#include "Dtree.hpp"
#include "assert.h"
namespace DTREE {
Dtree::Dtree(GRAPH::Graph* g)
    : cnt()
    , dfnum()
    , vertex()
    , parent()
    , samedom()
    , semi()
    , idom()
    , bucket()
    , best()
    , ancestor() {
    dfs(g->nodes()->at(0), nullptr);  // must be a function label
    assert(cnt == g->nodes()->size());
    for (int i = cnt - 1; i >= 1; i--) {
        auto node = vertex[i];
        auto p = parent[node];
        auto s = p;
        for (auto pred : *node->pred()) {
            GRAPH::Node* nxts = nullptr;
            if (dfnum[pred] <= dfnum[node])
                nxts = pred;
            else
                nxts = semi[findLowestSemiAncestor(pred)];
            if (dfnum[s] > dfnum[nxts]) s = nxts;
        }
        semi[node] = s;
        bucket[s].push_back(node);
        link(p, node);
        for (auto v : bucket[p]) {
            auto y = findLowestSemiAncestor(v);
            if (semi[y] == semi[v])
                idom[v] = p;
            else
                samedom[v] = y;
        }
        bucket[p].clear();
    }
    for (int i = 1; i < cnt; i++) {
        auto node = vertex[i];
        if (samedom.count(node)) { idom[node] = idom[samedom[node]]; }
    }
    for(int i=0;i<cnt;i++){
        auto node = g->nodes()->at(i);
        this->addNode(node);
    }
    for(int i=0;i<cnt;i++){
        auto node = g->nodes()->at(i);
        this->addNode(node);
        if(idom.count(node)){
            this->addEdge(this->nodes()->at(idom[node]->mykey),this->nodes()->at(node->mykey));
        }
    }
}
void Dtree::dfs(GRAPH::Node* node, GRAPH::Node* fa) {
    assert(!dfnum.count(node));
    dfnum[node] = cnt;
    vertex[cnt] = node;
    parent[node] = fa;
    cnt++;
    for (auto succ : *node->succ()) {
        if (dfnum.count(succ)) continue;
        dfs(succ, node);
    }
}
void Dtree::link(GRAPH::Node* fa, GRAPH::Node* node) {
    ancestor[node] = fa;
    best[node] = node;
}
GRAPH::Node* Dtree::findLowestSemiAncestor(GRAPH::Node* node) {
    auto a = ancestor[node];
    if (ancestor.count(a)) {
        auto b = findLowestSemiAncestor(a);
        ancestor[node] = ancestor[a];
        if (dfnum[semi[b]] < dfnum[semi[best[node]]]) { best[node] = b; }
    }
    return best[node];
}
}  // namespace DTREE