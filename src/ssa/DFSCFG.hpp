#ifndef __DFSCFG
#define __DFSCFG

#include "../backend/graph.hpp"
#include "Dtree.hpp"
#include "CFG.hpp"
#include "BuildSSA.hpp"

namespace DFSCFG {

struct Loop {
    Loop() {}
    Loop(CFG::CFGraph* _inCFGraph, GRAPH::Node* _head)
        : inCFGraph(_inCFGraph)
        , head(_head) {
        elements = std::unordered_set<GRAPH::Node*>();
        elements.insert(_head);
    }
    CFG::CFGraph* inCFGraph;  // Node->info:IR::Stmlist*--a block begin with label,end
    GRAPH::Node* head;
    std::unordered_set<GRAPH::Node*> elements;
};

class Loop_Nesting_Tree {
public:
    Loop_Nesting_Tree(){};
    Loop_Nesting_Tree(SSA::SSAIR* cfg);
    ~Loop_Nesting_Tree();  // Delete Loop of loops

    int Block_Dfsnum(int nodekey);
    int Block_Dfsnum(GRAPH::Node* node);

    // CFG node
    bool is_Dominate(int pa, int ch);
    bool is_StrictlyDominate(int pa, int ch);

private:
    SSA::SSAIR* graph_attached;
    std::unordered_set<Loop*> loops;  // seems to be useless
    std::unordered_set<GRAPH::Node*> heads;
    std::unordered_map<GRAPH::Node*, Loop*> loop_of_head;
    std::unordered_map<GRAPH::Node*, GRAPH::Node*> parent;
    std::unordered_map<GRAPH::Node*, std::set<GRAPH::Node*>> children;

    struct DfsInfo {
        int dfsnum, size;
        DfsInfo(int _dfn, int _siz)
            : dfsnum(_dfn)
            , size(_siz) {}
    };
    std::unordered_set<GRAPH::Node*> visit;
    std::unordered_map<GRAPH::Node*, DfsInfo> dfsinfo;
    std::unordered_map<int, DfsInfo> dtreeinfo;

    bool is_ancestor(GRAPH::Node* x, GRAPH::Node* anc);
    void buildTree();
    void initHead();
    void dfsHead(GRAPH::Node* n, std::unordered_set<GRAPH::Node*>& instk, int& dfn);
    void dfsDTree(int node, int& dfn);
    void dfsBuild(GRAPH::Node* n, GRAPH::Node* head);
};

}  // namespace DFSCFG

#endif