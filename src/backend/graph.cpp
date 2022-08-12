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
std::vector<Node*>* Graph::nodes() { return &this->mynodes; }
Node* Graph::addNode(void* info) {
    Node* node = new GRAPH::Node(this->nodecount++, this, info);
    this->mynodes.push_back(node);
    return node;
}
void Graph::addEdge(Node* from, Node* to) {
    assert(from);
    assert(to);
    assert(from->mygraph == to->mygraph);
    if (goesTo(from, to)) return;
    to->preds.insert(from->mykey);
    from->succs.insert(to->mykey);
}
void Graph::rmEdge(Node* from, Node* to) {
    assert(from && to);
    to->preds.erase(to->preds.find(from->mykey));
    from->succs.erase(from->succs.find(to->mykey));
}

void GRAPH::Graph::rmNode(GRAPH::Node* node, GRAPH::Node* adjnode) {
    assert(node && adjnode);
    assert(adjnode->succs.count(node->mykey) && adjnode->preds.count(node->mykey));
    adjnode->succs.erase(node->mykey);
    adjnode->preds.erase(node->mykey);
}
void GRAPH::Graph::reverseNode(GRAPH::Node* node) {
    assert(node);
    for (auto adjnode : node->succs) {
        node->mygraph->mynodes[adjnode]->succs.insert(node->mykey);
        node->mygraph->mynodes[adjnode]->preds.insert(node->mykey);
    }
}

bool Graph::goesTo(Node* from, Node* n) { return from->succs.count(n->mykey); }
