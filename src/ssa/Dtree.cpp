#include "Dtree.hpp"
#include "assert.h"
namespace DTREE {
Dtree::Dtree(GRAPH::Graph* _g)
    : cnt(0)
    , dfnum(_g->nodecount, -1)
    , vertex(_g->nodecount, -1)
    , parent(_g->nodecount, -1)
    , samedom(_g->nodecount, -1)
    , semi(_g->nodecount, -1)
    , idom(_g->nodecount, -1)
    , bucket(_g->nodecount, std::vector<int>())
    , best(_g->nodecount, -1)
    , ancestor(_g->nodecount, -1)
    , children(_g->nodecount, std::vector<int>())
    , DF() {
    g = _g;
    dfs(0, -1);  // must be a function label
    for (int i = g->nodecount - 1; i >= 1; i--) {
        auto node = vertex[i];
        auto p = parent[node];
        auto s = p;
        for (auto pred : *g->nodes()->at(node)->pred()) {
            int nxts = -1;
            if (dfnum[pred->mykey] <= dfnum[node])
                nxts = pred->mykey;
            else
                nxts = semi[findLowestSemiAncestor(pred->mykey)];
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
        int node = vertex[i];
        if (samedom[node] != -1) { idom[node] = idom[samedom[node]]; }
    }
    for (int i = 1; i < cnt; i++) {
        assert(idom[i] != -1);
        this->children[idom[i]].push_back(i);
    }
}
void Dtree::dfs(int node, int fa) {
    dfnum[node] = cnt;
    vertex[cnt] = node;
    parent[node] = fa;
    cnt++;
    for (auto succ : *g->nodes()->at(node)->succ()) {
        if (dfnum[succ->mykey] != -1) continue;
        dfs(succ->mykey, node);
    }
}
void Dtree::link(int fa, int node) {
    ancestor[node] = fa;
    best[node] = node;
}
int Dtree::findLowestSemiAncestor(int node) {
    int a = ancestor[node];
    if (ancestor[a] != -1) {
        int b = findLowestSemiAncestor(a);
        ancestor[node] = ancestor[a];
        if (dfnum[semi[b]] < dfnum[semi[best[node]]]) { best[node] = b; }
    }
    return best[node];
}
int Dtree::find(int node){
    if(ancestor[node]==node)return node;
    return ancestor[node]=find(ancestor[node]);
}
void Dtree::computeDF(int node) {
    DF = std::vector<std::vector<int>>(g->nodecount, std::vector<int>());
    for(auto succ:*g->nodes()->at(node)->succ()){
        if(idom[succ->mykey]!=node)DF[node].push_back(succ->mykey);
    }
    for(int succ:children[node]){
        assert(ancestor[succ]==succ);
        computeDF(succ);
        ancestor[succ]=node;
    }
    for(int succ:children[node]){
        for(int w:DF[succ]){
            if(node==w||find(w)!=node){
                DF[node].push_back(w);
            }
        }
    }
}
void Dtree::computeDF(){
    for(int i=0;i<ancestor.size();i++)ancestor[i]=i;
    computeDF(0);
}
}  // namespace DTREE