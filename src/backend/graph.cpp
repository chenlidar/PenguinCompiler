/*
 * graph.c - Functions to manipulate and create control flow and
 *           interference graphs.
 */

#include <iostream>
#include <fstream>
#include <assert.h>
#include <algorithm>
#include "graph.hpp"

using namespace GRAPH;

void* Node::nodeInfo() { return this->info; }
bool Node::inNodeList(NodeList& l) {
    for (auto& i : l)
        if (i->nodeid() == this->nodeid()) return true;
    return false;
}
NodeList* Node::succ() { return &this->succs; }
NodeList* Node::pred() { return &this->preds; }
int Node::degree() { return this->preds.size() + this->succs.size(); }
int Node::nodeid() { return this->mykey; }
int Node::inDegree() {
    int deg = 0;
    return this->preds.size();
}

/* return length of successor list for node n */
int Node::outDegree() {
    int deg = 0;
    return this->succs.size();
}
void* Graph::nodeInfo(Node* n) { return n->nodeInfo(); }
bool Graph::inNodeList(Node* n, NodeList& l) { return n->inNodeList(l); }
NodeList* Graph::succ(Node* n) { return n->succ(); }
NodeList* Graph::pred(Node* n) { return n->pred(); }
int Graph::degree(Node* n) { return n->degree(); }
int Graph::nodeid(Node* n) { return n->nodeid(); }
NodeList* Graph::nodes() { return &this->mynodes; }
Node* Graph::addNode(void* info) {
    Node* node = new GRAPH::Node(this->nodecount++, this, info);
    this->mynodes.push_back(node);
    return node;
}
void Graph::addEdge(Node* from, Node* to) {
    assert(from);
    assert(to);
    assert(from->mygraph==to->mygraph);
    if (goesTo(from, to)) return;
    to->preds.push_back(from);
    from->succs.push_back(to);
}
void Graph::rmEdge(Node* from, Node* to) {
    assert(from && to);
    for (auto i = to->preds.begin(); i != to->preds.end(); ++i)
        if (from == *i) {
            to->preds.erase(i);
            break;
        }
    for (auto i = from->succs.begin(); i != from->succs.end(); ++i)
        if (to == *i) {
            from->succs.erase(i);
            break;
        }
}
static GRAPH::NodeList* del(GRAPH::Node* a, GRAPH::NodeList* l) {
    assert(a && l);
    l->erase(std::find(l->begin(), l->end(), a));
    return l;
}
void GRAPH::Graph::rmNode(GRAPH::Node* node) {
    assert(node);
    for (auto prev : node->preds) { del(node, &prev->succs); }
    for (auto succ : node->succs) { del(node, &succ->preds); }
}
void GRAPH::Graph::reverseNode(GRAPH::Node* node) {
    assert(node);
    for (auto prev : node->succs) { prev->succs.push_back(node); }
    for (auto succ : node->preds) { succ->preds.push_back(node); }
}

bool Graph::goesTo(Node* from, Node* n) { return n->inNodeList(from->succs); }
