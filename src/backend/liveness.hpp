#ifndef __LIVENESS
#define __LIVENESS
#include <cstdio>
#include "graph.hpp"
#include <set>
#include <stack>
#include "flowgraph.hpp"
#include <unordered_map>
#include "../structure/assem.h"
namespace LIVENESS {

struct InOut {
    std::set<int>* in;
    std::set<int>* out;
    std::set<int>* use;
    std::set<int>* def;
    InOut(std::set<int>* _use, std::set<int>* _def, std::set<int>* _in, std::set<int>* _out)
        : use(_use)
        , def(_def)
        , in(_in)
        , out(_out) {}
};
class Liveness {
public:
    FLOW::FlowGraph* flowgraph;
    std::set<int>* FG_Out(GRAPH::Node*);
    std::set<int>* FG_In(GRAPH::Node*);
    Liveness(FLOW::FlowGraph* _flowgraph) {
        flowgraph = _flowgraph;
        InOutTable = std::unordered_map<GRAPH::Node*, InOut*>();
        workset = std::stack<GRAPH::Node*>();
        for (auto it : *flowgraph->nodes()) {
            if (InOutTable.count(it) == 0) {
                std::set<int>*use, *def, *in, *out;
                use = new std::set<int>();
                in = new std::set<int>();
                out = new std::set<int>();
                def = new std::set<int>();
                for (auto it : *flowgraph->FG_use(it)) { use->insert(it); }
                for (auto it : *flowgraph->FG_def(it)) def->insert(it);
                InOutTable.insert(std::make_pair(it, new InOut(use, def, in, out)));
            }
        }
        analysis();
    }
    ~Liveness() {
        for (auto& it : InOutTable) {
            delete it.second->def;
            delete it.second->out;
            delete it.second->use;
            delete it.second->in;
        }
    }

private:
    std::unordered_map<GRAPH::Node*, InOut*> InOutTable;
    std::stack<GRAPH::Node*> workset;
    void analysis();
};
}  // namespace LIVENESS
#endif