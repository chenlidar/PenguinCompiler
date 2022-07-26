
// #include <unordered_map>
// #include <iostream>
// #include <set>
// #include <vector>
// #include "assert.h"
// #include <string.h>
// #include <stdlib.h>
// #include <string>

// namespace GRAPH {

// class Graph;
// class Node;
// typedef std::set<Node*> NodeList;

// struct Node {
//     int mykey;
//     Graph* mygraph;
//     NodeList succs;
//     NodeList preds;
//     void* info;

//     Node() {}
//     Node(int _mykey, Graph* _mygraph, void* _info = NULL)
//         : mykey(_mykey)
//         , mygraph(_mygraph)
//         , info(_info) {
//         succs = NodeList();
//         preds = NodeList();
//     }

//     void* nodeInfo();
//     NodeList* succ();
//     NodeList* pred();
//     int nodeid();
//     int outDegree();
//     int inDegree();
// };

// class Graph {
// private:
//     std::vector<Node*> mynodes;

// public:
//     int nodecount;
//     Graph() {
//         nodecount = 0;
//         mynodes = std::vector<Node*>();
//     }
//     ~Graph() {
//         for (auto& i : mynodes) delete i;
//     }
//     void clear() {
//         for (auto& i : mynodes) delete i;
//         mynodes.clear();
//         nodecount = 0;
//     }
//     /* Make a new node in graph "g", with associated "info" */
//     Node* addNode(void* info);

//     /* Get the list of nodes belonging to "g" */
//     std::vector<Node*>* nodes();

//     /* Make a new edge joining nodes "from" and "to", which must belong
//         to the same graph */
//     void addEdge(Node* from, Node* to);

//     /* Delete the edge joining "from" and "to" */
//     void rmEdge(Node* from, Node* to);

//     /* Tell if there is an edge from "from" to "to" */
//     bool goesTo(Node* from, Node* n);
//     void rmNode(GRAPH::Node* node, GRAPH::Node* adjnode);
//     void reverseNode(GRAPH::Node* node);
// };
// }  // namespace GRAPH

// namespace DTREE{
//     class Dtree{
//         public:
//         std::vector<std::vector<int> > children;
//         Dtree(GRAPH::Graph* _g);
//         private:
//         int cnt;
//         GRAPH::Graph* g;
//         std::vector<int> dfnum;
//         std::vector<int> vertex;
//         std::vector<int> parent;
//         std::vector<int> samedom;
//         std::vector<int> semi;
//         std::vector<int> idom;
//         std::vector<int> best;
//         std::vector<int> ancestor;
//         std::vector<std::vector<int> > bucket;
//         void dfs(int root,int fa);
//         void link(int fa,int node);
//         int findLowestSemiAncestor(int node);
//     };
// }
// DTREE::Dtree* tree;
//     int num[200005];
// int calsize(int node){
//     if(num[node]!=-1)return num[node];
//     num[node]=1;
//     for(auto it:tree->children[node]){
//         num[node]+=calsize(it);
//     }
//     return num[node];
// }
// int main(){
//     GRAPH::Graph* g=new GRAPH::Graph();
//     int n,m,u,v;
//     std::cin>>n>>m;
//     for(int i=0;i<n;i++)g->addNode(new int(i));
//     for(int i=0;i<m;i++){
//         std::cin>>u>>v;u=u-1;v=v-1;
//         g->addEdge(g->nodes()->at(u),g->nodes()->at(v));
//     }
//     memset(num,-1,sizeof(num));
//     tree=new DTREE::Dtree(g);
//     for(int i=0;i<n;i++){
//         std::cout<<calsize(i)<<" ";
//     }
// }
// using namespace GRAPH;
// using namespace DTREE;
// void* Node::nodeInfo() { return this->info; }
// NodeList* Node::succ() { return &this->succs; }
// NodeList* Node::pred() { return &this->preds; }
// int Node::nodeid() { return this->mykey; }
// int Node::inDegree() {
//     int deg = 0;
//     return this->preds.size();
// }

// /* return length of successor list for node n */
// int Node::outDegree() {
//     int deg = 0;
//     return this->succs.size();
// }
// std::vector<Node*>* Graph::nodes() { return &this->mynodes; }
// Node* Graph::addNode(void* info) {
//     Node* node = new GRAPH::Node(this->nodecount++, this, info);
//     this->mynodes.push_back(node);
//     return node;
// }
// void Graph::addEdge(Node* from, Node* to) {
//     assert(from);
//     assert(to);
//     assert(from->mygraph == to->mygraph);
//     if (goesTo(from, to)) return;
//     to->preds.insert(from);
//     from->succs.insert(to);
// }
// void Graph::rmEdge(Node* from, Node* to) {
//     assert(from && to);
//     to->preds.erase(to->preds.find(from));
//     from->succs.erase(from->succs.find(to));
// }

// void GRAPH::Graph::rmNode(GRAPH::Node* node, GRAPH::Node* adjnode) {
//     assert(node && adjnode);
//     assert(adjnode->succs.count(node) && adjnode->preds.count(node));
//     adjnode->succs.erase(node);
//     adjnode->preds.erase(node);
// }
// void GRAPH::Graph::reverseNode(GRAPH::Node* node) {
//     assert(node);
//     for (auto adjnode : node->succs) {
//         adjnode->succs.insert(node);
//         adjnode->preds.insert(node);
//     }
// }

// bool Graph::goesTo(Node* from, Node* n) { return from->succs.count(n); }
// namespace DTREE {
// Dtree::Dtree(GRAPH::Graph* _g)
//     : cnt(0)
//     , dfnum(_g->nodecount, -1)
//     , vertex(_g->nodecount, -1)
//     , parent(_g->nodecount, -1)
//     , samedom(_g->nodecount, -1)
//     , semi(_g->nodecount, -1)
//     , idom(_g->nodecount, -1)
//     , bucket(_g->nodecount, std::vector<int>())
//     , best(_g->nodecount, -1)
//     , ancestor(_g->nodecount, -1)
//     , children(_g->nodecount, std::vector<int>()) {
//     g = _g;
//     dfs(0, -1);  // must be a function label
//     for (int i = g->nodecount - 1; i >= 1; i--) {
//         auto node = vertex[i];
//         auto p = parent[node];
//         auto s = p;
//         for (auto pred : *g->nodes()->at(node)->pred()) {
//             int nxts = -1;
//             if (dfnum[pred->mykey] <= dfnum[node])
//                 nxts = pred->mykey;
//             else
//                 nxts = semi[findLowestSemiAncestor(pred->mykey)];
//             if (dfnum[s] > dfnum[nxts]) s = nxts;
//         }
//         semi[node] = s;
//         bucket[s].push_back(node);
//         link(p, node);
//         for (auto v : bucket[p]) {
//             auto y = findLowestSemiAncestor(v);
//             if (semi[y] == semi[v])
//                 idom[v] = p;
//             else
//                 samedom[v] = y;
//         }
//         bucket[p].clear();
//     }
//     for (int i = 1; i < cnt; i++) {
//         int node = vertex[i];
//         if (samedom[node] != -1) { idom[node] = idom[samedom[node]]; }
//     }
//     for (int i = 1; i < cnt; i++) {
//         assert(idom[i] != -1);
//         this->children[idom[i]].push_back(i);
//     }
// }
// void Dtree::dfs(int node, int fa) {
//     dfnum[node] = cnt;
//     vertex[cnt] = node;
//     parent[node] = fa;
//     cnt++;
//     for (auto succ : *g->nodes()->at(node)->succ()) {
//         if (dfnum[succ->mykey] != -1) continue;
//         dfs(succ->mykey, node);
//     }
// }
// void Dtree::link(int fa, int node) {
//     ancestor[node] = fa;
//     best[node] = node;
// }
// int Dtree::findLowestSemiAncestor(int node) {
//     int a = ancestor[node];
//     if (ancestor[a] != -1) {
//         int b = findLowestSemiAncestor(a);
//         ancestor[node] = ancestor[a];
//         if (dfnum[semi[b]] < dfnum[semi[best[node]]]) { best[node] = b; }
//     }
//     return best[node];
// }
// }  // namespace DTREE