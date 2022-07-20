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
NodeList* Node::succ() { return &this->succs; }
NodeList* Node::pred() { return &this->preds; }
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
NodeList* Graph::nodes() { return &this->mynodes; }
Node* Graph::addNode(void* info) {
    Node* node = new GRAPH::Node(this->nodecount++, this, info);
    this->mynodes.insert(node);
    return node;
}
void Graph::addEdge(Node* from, Node* to) {
    assert(from);
    assert(to);
    assert(from->mygraph == to->mygraph);
    if (goesTo(from, to)) return;
    to->preds.insert(from);
    from->succs.insert(to);
}
void Graph::rmEdge(Node* from, Node* to) {
    assert(from && to);
    to->preds.erase(to->preds.find(from));
    from->succs.erase(from->succs.find(to));
}

void GRAPH::Graph::rmNode(GRAPH::Node* node, GRAPH::Node* adjnode) {
    assert(node && adjnode);
    assert(adjnode->succs.count(node) && adjnode->preds.count(node));
    adjnode->succs.erase(node);
    adjnode->preds.erase(node);
}
void GRAPH::Graph::reverseNode(GRAPH::Node* node) {
    assert(node);
    for (auto adjnode : node->succs) { adjnode->succs.insert(node);adjnode->preds.insert(node); }
}

bool Graph::goesTo(Node* from, Node* n) { return from->succs.count(n); }
