/*
 * graph.h - Abstract Data Type (ADT) for directed graphs
 */

#ifndef __GRAPH
#define __GRAPH

#include <unordered_map>
#include <iostream>
#include <vector>

//#include "util.h"

namespace GRAPH {

class Graph;
class Node;
typedef std::vector<Node*> NodeList;

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

    /* Get the "info" associated with node*/
    void* nodeInfo();

    /* Tell if node is in the list "l" */
    bool inNodeList(NodeList& l);

    /* Get all the successors of node*/
    NodeList* succ();

    /* Get all the predecessors of node */
    NodeList* pred();

    /* Tell how many edges lead to or from "n" */
    int degree();

    /* Tell the id of "n" */
    int nodeid();

    int outDegree();
    int inDegree();
};

class Graph {
private:
    int nodecount;
    NodeList mynodes;

public:
    Graph() {
        nodecount = 0;
        mynodes = NodeList();
    }
    ~Graph() {
        for (auto& i : mynodes) delete i;
    }

    /* Get the "info" associated with node "n" */
    void* nodeInfo(Node* n);

    /* Tell if "a" is in the list "l" */
    bool inNodeList(Node* n, NodeList& l);

    /* Get all the successors of node "n" */
    NodeList* succ(Node* n);

    /* Get all the predecessors of node "n" */
    NodeList* pred(Node* n);

    /* Tell how many edges lead to or from "n" */
    int degree(Node* n);

    /* Tell the id of "n" */
    int nodeid(Node* n);

    /* Make a new node in graph "g", with associated "info" */
    Node* addNode(void* info);

    /* Get the list of nodes belonging to "g" */
    NodeList *nodes();

    /* Make a new edge joining nodes "from" and "to", which must belong
        to the same graph */
    void addEdge(Node* from, Node* to);

    /* Delete the edge joining "from" and "to" */
    void rmEdge(Node* from, Node* to);

    /* Tell if there is an edge from "from" to "to" */
    bool goesTo(Node* from, Node* n);
    void rmNode(GRAPH::Node* node);
    void reverseNode(GRAPH::Node* node);
};

template <typename T> class Table {
    std::unordered_map<int, T> t;

public:
    Table() {}
    void enter(Node* node, T value);
    T look(Node* node);
};

template <typename T> void Table<T>::enter(Node* node, T value) { this->t[node->mykey] = value; }
template <typename T> T Table<T>::look(Node* node) { this->t.erase(node->mykey); }

}  // namespace GRAPH

#endif