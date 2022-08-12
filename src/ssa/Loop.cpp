
#include "Loop.hpp"
#include "../util/utils.hpp"
#include "CFG.hpp"
#include <assert.h>
#include <algorithm>

using namespace Loop;

Loop_Nesting_Tree::Loop_Nesting_Tree(CFG::CFGraph* cfg){
    this->graph_attached = cfg;
    initHead();
    buildTree();
}
Loop_Nesting_Tree::~Loop_Nesting_Tree(){
    for(auto &i: this->loops)
        delete i;
}


void Loop_Nesting_Tree::buildTree(){//before this, make sure initHead have done
    visit.clear();
    vector<GRAPH::Node*> vec;
    for(auto &i:heads){
        vec.push_back(i);
    }
    std::sort(vec.begin(),vec.end(),[&](GRAPH::Node* x,GRAPH::Node* y){
        return this->dfsinfo[x].dfsnum > this->dfsinfo[y].dfsnum;
    });

    for(auto &i: vec){
        dfsBuild(i,i);
    }

    std::cerr<<"[Loop Nesting Tree]"<<"Build the tree, done."<<std::endl;
}

void Loop_Nesting_Tree::dfsBuild(GRAPH::Node* n,GRAPH::Node* head){
    assert(n && head);
    this->visit.insert(n);
    for(auto &pre: *n->pred()){
        if(visit.count(pre) ||  !is_ancestor(pre,head))continue;
        if(!parent.count(pre)){
            parent[pre] = head;
            children[head].insert(pre); 
        }
        dfsBuild(pre,head);
    }
}

bool Loop_Nesting_Tree::is_ancestor(GRAPH::Node* x,GRAPH::Node* anc){
    assert(this->dfsinfo.count(x) && this->dfsinfo.count(anc));
    auto _anc = dfsinfo[anc], _x = dfsinfo[x];
    return _anc.dfsnum <= _x.dfsnum && _x.dfsnum < _anc.dfsnum + _anc.size; 
}

void Loop_Nesting_Tree::initHead(){
    visit.clear();
    heads.clear();
    dfsinfo.clear();
    loops.clear();//leak?

    std::unordered_set <GRAPH::Node*> instk;
    int dfn=0;
    dfsHead((*(graph_attached->nodes()))[0],instk,dfn);//I think mynodes[0] is the root
    std::cerr<<"[Loop Nesting Tree]"<<"Initiate the loop-head, done."<<std::endl;
}

void Loop_Nesting_Tree::dfsHead(GRAPH::Node* n,std::unordered_set <GRAPH::Node*> &instk,int &dfn){
    assert(n);
    this->dfsinfo[n]=(DfsInfo(++dfn,1));
    instk.insert(n);
    this->visit.insert(n);
    for(auto &son: *n->succ()){
        if(!visit.count(son)){
            dfsHead(son,instk,dfn);
            this->dfsinfo[n].size +=  this->dfsinfo[son].size;
        }
        else if(instk.count(son)){
            if(!this->heads.count(son)){
                this->loops.insert(new Loop(this->graph_attached,son));
                this->heads.insert(son);
            }
        }
    }
    instk.erase(n);
}
