/*
 * graph.h - Abstract Data Type (ADT) for directed graphs
 */

#ifndef __GRAPH
#define __GRAPH

#include <unordered_map>
#include <iostream>
#include <set>
#include <vector>

//#include "util.h"

namespace GRAPH {

class Graph;
class Node;
typedef std::set<Node*> NodeList;

struct Node {
    int mykey;
    Graph* mygraph;
    NodeList succs;
    NodeList preds;
    void* info;

    Node() {}
    Node(int _mykey, Graph* _mygraph, void* _info = NULL)
        : mykey(_mykey)
        , mygraph(_mygraph)
        , info(_info) {
        succs = NodeList();
        preds = NodeList();
    }

    void* nodeInfo();
    NodeList* succ();
    NodeList* pred();
    int nodeid();
    int outDegree();
    int inDegree();
};

class Graph {
private:
    std::vector<Node*> mynodes;

public:
    int nodecount;
    Graph() {
        nodecount = 0;
        mynodes = std::vector<Node*>();
    }
    ~Graph() {
        for (auto& i : mynodes) delete i;
    }
    void clear() {
        for (auto& i : mynodes) delete i;
        mynodes.clear();
        nodecount = 0;
    }
    /* Make a new node in graph "g", with associated "info" */
    Node* addNode(void* info);

    /* Get the list of nodes belonging to "g" */
    std::vector<Node*>* nodes();

    /* Make a new edge joining nodes "from" and "to", which must belong
        to the same graph */
    void addEdge(Node* from, Node* to);

    /* Delete the edge joining "from" and "to" */
    void rmEdge(Node* from, Node* to);

    /* Tell if there is an edge from "from" to "to" */
    bool goesTo(Node* from, Node* n);
    void rmNode(GRAPH::Node* node, GRAPH::Node* adjnode);
    void reverseNode(GRAPH::Node* node);
};
}  // namespace GRAPH

#endif