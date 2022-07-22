
// #include <unordered_map>
// #include <iostream>
// #include <set>
// #include <vector>
// #include "assert.h"
// #include <string.h>
// #include <stdlib.h>
// #include <string>
// //#include "util.h"

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
//     int nodecount;
//     std::vector<Node*> mynodes;

// public:
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
//     class Dtree:public GRAPH::Graph{
//         public:
//         Dtree(GRAPH::Graph* g);
//         private:
//         std::unordered_map<GRAPH::Node*,int> dfnum;
//         std::unordered_map<int,GRAPH::Node*> vertex;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> parent;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> samedom;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> semi;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> idom;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> best;
//         std::unordered_map<GRAPH::Node*,GRAPH::Node*> ancestor;
//         std::unordered_map<GRAPH::Node*,std::vector<GRAPH::Node*> > bucket;
//         int cnt;
//         void dfs(GRAPH::Node* root,GRAPH::Node *fa);
//         void link(GRAPH::Node* fa,GRAPH::Node* node);
//         GRAPH::Node* findLowestSemiAncestor(GRAPH::Node* node);
//     };
// }

//     int num[200005];
// int calsize(GRAPH::Node* node){
//     if(num[node->mykey]!=-1)return num[node->mykey];
//     num[node->mykey]=1;
//     for(auto it:*node->succ()){
//         num[node->mykey]+=calsize(it);
//     }
//     return num[node->mykey];
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
//     DTREE::Dtree* tree=new DTREE::Dtree(g);
//     for(int i=0;i<n;i++){
//         auto node=tree->nodes()->at(i);
//         std::cout<<calsize(tree->nodes()->at(i))<<" ";
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
// Dtree::Dtree(GRAPH::Graph* g)
//     : cnt()
//     , dfnum()
//     , vertex()
//     , parent()
//     , samedom()
//     , semi()
//     , idom()
//     , bucket()
//     , best()
//     , ancestor() {
//     dfs(g->nodes()->at(0), nullptr);  // must be a function label
//     assert(cnt == g->nodes()->size());
//     for (int i = cnt - 1; i >= 1; i--) {
//         auto node = vertex[i];
//         auto p = parent[node];
//         auto s = p;
//         for (auto pred : *node->pred()) {
//             GRAPH::Node* nxts = nullptr;
//             if (dfnum[pred] <= dfnum[node])
//                 nxts = pred;
//             else
//                 nxts = semi[findLowestSemiAncestor(pred)];
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
//         auto node = vertex[i];
//         if (samedom.count(node)) { idom[node] = idom[samedom[node]]; }
//     }
//     for(int i=0;i<cnt;i++){
//         auto node = g->nodes()->at(i);
//         this->addNode(node);
//     }
//     for(int i=0;i<cnt;i++){
//         auto node = g->nodes()->at(i);
//         this->addNode(node);
//         if(idom.count(node)){
//             this->addEdge(this->nodes()->at(idom[node]->mykey),this->nodes()->at(node->mykey));
//         }
//     }
// }
// void Dtree::dfs(GRAPH::Node* node, GRAPH::Node* fa) {
//     assert(!dfnum.count(node));
//     dfnum[node] = cnt;
//     vertex[cnt] = node;
//     parent[node] = fa;
//     cnt++;
//     for (auto succ : *node->succ()) {
//         if (dfnum.count(succ)) continue;
//         dfs(succ, node);
//     }
// }
// void Dtree::link(GRAPH::Node* fa, GRAPH::Node* node) {
//     ancestor[node] = fa;
//     best[node] = node;
// }
// GRAPH::Node* Dtree::findLowestSemiAncestor(GRAPH::Node* node) {
//     auto a = ancestor[node];
//     if (ancestor.count(a)) {
//         auto b = findLowestSemiAncestor(a);
//         ancestor[node] = ancestor[a];
//         if (dfnum[semi[b]] < dfnum[semi[best[node]]]) { best[node] = b; }
//     }
//     return best[node];
// }