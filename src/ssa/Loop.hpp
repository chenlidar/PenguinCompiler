#ifndef __LOOP
#define __LOOP

#include "../backend/graph.hpp"
#include "Dtree.hpp"
#include "CFG.hpp"

namespace Loop {

struct Loop{
    Loop(){}
    Loop(CFG::CFGraph* _inCFGraph,GRAPH::Node* _head):inCFGraph(_inCFGraph),head(_head) {
        elements=std::unordered_set<GRAPH::Node*>();
        elements.insert(_head);
    }
    CFG::CFGraph* inCFGraph;// Node->info:IR::Stmlist*--a block begin with label,end
    GRAPH::Node* head;
    std::unordered_set<GRAPH::Node*> elements; 
};

class Loop_Nesting_Tree{
    public:
        Loop_Nesting_Tree(){};
        Loop_Nesting_Tree(CFG::CFGraph* cfg);
        ~Loop_Nesting_Tree();//Delete Loop of loops
    private:
        CFG::CFGraph* graph_attached;
        std::unordered_set<Loop*> loops;//seems to be useless
        std::unordered_set<GRAPH::Node*> heads;
        std::unordered_map<GRAPH::Node*,Loop*> loop_of_head;
        std::unordered_map<GRAPH::Node*,GRAPH::Node*> parent;
        std::unordered_map<GRAPH::Node*,std::set<GRAPH::Node*> > children;

        struct DfsInfo{
            int dfsnum,size;
            DfsInfo(int _dfn,int _siz):dfsnum(_dfn),size(_siz){}
        };
        std::unordered_set <GRAPH::Node*> visit;
        std::unordered_map <GRAPH::Node*,DfsInfo> dfsinfo;

        bool is_ancestor(GRAPH::Node* x,GRAPH::Node* anc);
        void buildTree();
        void initHead();
        void dfsHead(GRAPH::Node* n,std::unordered_set <GRAPH::Node*> &instk,int &dfn);
        void dfsBuild(GRAPH::Node* n,GRAPH::Node* head);
        
};

}  // namespace Loop

#endif