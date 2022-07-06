/*
 * graph.h - Abstract Data Type (ADT) for directed graphs
 */

#ifndef __GRAPH
#define __GRAPH

#include <unordered_map>
#include <iostream>
#include <vector>

//#include "util.h"

namespace Graph{

class graph;
class node_;
typedef node_ *node;    /* The "node" type */
typedef std::vector<node> nodeList;

struct node_{
    int mykey;
    graph *mygraph;
    nodeList succs;
    nodeList preds;
    void *info;

    node_(){}
    node_(int _mykey,graph* _mygraph,void *_info=NULL):mykey(_mykey),mygraph(_mygraph),info(_info)
    {
        succs = nodeList();
        preds = nodeList();
    }

    /* Get the "info" associated with node*/
    void *nodeInfo(); 

    /* Tell if node is in the list "l" */
    bool inNodeList(nodeList &l); 

    /* Get all the successors of node*/
    nodeList succ(); 

    /* Get all the predecessors of node */
    nodeList pred(); 

    /* Tell how many edges lead to or from "n" */
    int degree();

    /* Tell the id of "n" */
    int nodeid();

    /* Get all the successors and predecessors of "n" */
    nodeList adj();
};

class graph{
private:
    int nodecount;
	nodeList mynodes;

public:
    graph(){
        nodecount = 0;
        mynodes = nodeList();
    }
	~graph(){
		for(auto &i: mynodes)
			delete i;
	}

    /*create new node and return its pointer */
    node newNode();

    /* Get the "info" associated with node "n" */
    void *nodeInfo(node n); 

    /* Tell if "a" is in the list "l" */
    bool inNodeList(node n,nodeList &l); 

    /* Get all the successors of node "n" */
    nodeList succ(node n); 

    /* Get all the predecessors of node "n" */
    nodeList pred(node n); 

    /* Tell how many edges lead to or from "n" */
    int degree(node n);

    /* Tell the id of "n" */
    int nodeid(node n);

    /* Get all the successors and predecessors of "n" */
    nodeList adj(node n);

    /* Make a new graph */
    graph Graph(void); 
    /* Make a new node in graph "g", with associated "info" */
    node Node(graph g, void *info);

    /* Get the list of nodes belonging to "g" */
    nodeList nodes(graph g);

    /* Make a new edge joining nodes "from" and "to", which must belong
        to the same graph */
    void addEdge(node from, node to);

    /* Delete the edge joining "from" and "to" */
    void rmEdge(node from, node to);

    /* Show all the nodes and edges in the graph, using the function "showInfo"
        to print the name of each node */
    void show(FILE *out, nodeList p, void showInfo(void *));

    /* Tell if there is an edge from "from" to "to" */
    bool goesTo(node from, node n);

};

template <typename T>
class table{
	std::unordered_map<int,T> t;
public:
	table(){}
	void enter(node node, T value);
	T look(node node);
};


template <typename T>
void table<T>::enter(node node, T value)
{
	this->t[node->mykey]=value;
}
template <typename T>
T table<T>::look(node node)
{
	this->t.erase(node->mykey);
}


}


#endif
