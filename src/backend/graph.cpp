/*
 * graph.c - Functions to manipulate and create control flow and
 *           interference graphs.
 */

#include <iostream>
#include <fstream>
#include <assert.h>
#include "graph.hpp"

using namespace Graph;

void * node_::nodeInfo()
{
    return this->info;
}
bool node_::inNodeList(nodeList &l)
{
    for(auto &i: l)
        if(i->nodeid() == this->nodeid())
            return true;
    return false;
}
nodeList node_::succ()
{
	return this->succs;
}
nodeList node_::pred()
{
	return this->preds;
}
int node_::degree()
{
	return this->preds.size() + this->succs.size();
}
int node_::nodeid()
{
	return this->mykey;
}
nodeList node_::adj()
{
	nodeList ret = this->preds;
	ret.insert(ret.end(),this->succs.begin(),this->succs.end());
	return ret;
}

node graph::newNode()
{
	mynodes.push_back(new node_(nodecount++,this));
	return mynodes.back();
}

void * graph::nodeInfo(node n)
{
    return n->nodeInfo();
}
bool graph::inNodeList(node n, nodeList &l)
{
    return n->inNodeList(l);
}
nodeList graph::succ(node n)
{
	return n->succ();
}
nodeList graph::pred(node n)
{
	return n->pred();
}
int graph::degree(node n)
{
	return n->degree();
}
int graph::nodeid(node n)
{
	return n->nodeid();
}
nodeList graph::adj(node n)
{
	return n->adj();
}
void graph::addEdge(node from, node to)
{
	assert(from);  assert(to);
	if (goesTo(from, to)) return;
  	to->preds.push_back(from);
  	from->succs.push_back(to);
}
void graph::rmEdge(node from, node to)
{
	assert(from && to);
	for(auto i = to->preds.begin();i!=to->preds.end();++i)
		if(from == *i){
			to->preds.erase(i);
			break;
		}
	for(auto i = from->succs.begin();i!=from->succs.end();++i)
		if(to == *i){
			from->succs.erase(i);
			break;
		}
}
void show(std::ofstream out, nodeList p, void showInfo(void *))
{
	for (auto &n: p) {
		assert(n);
		nodeList q(n->succ());
		if (showInfo) 
			showInfo(n->info);
		out<<" ("<<(n->mykey)<<"): "; 
		for(auto &s:q) 
			out<<s->mykey<<" ";
		out<<"\n";
	}
}
bool goesTo(node from, node n)
{
	return n->inNodeList(from->succs);
}

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


