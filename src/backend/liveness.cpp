#include "liveness.hpp"
#include <unordered_map>
#include "flowgraph.hpp"
#include <assert.h>
#include <algorithm>
using namespace LIVENESS;
struct InOut {
    std::set<int>* in;
    std::set<int>* out;
    std::set<int>* use;
    std::set<int>* def;
    InOut(std::set<int>* _use, std::set<int>* _def)
        : use(_use)
        , def(_def) {
        in = new std::set<int>();
        out = new std::set<int>();
    }
};

static std::unordered_map<GRAPH::Node*, InOut*> InOutTable;

static void init_INOUT(GRAPH::NodeList* l) {
    InOutTable = std::unordered_map<GRAPH::Node*, InOut*>();
    for (auto it : *l) {
        if (InOutTable.count(it) == 0) {
            std::set<int>*use, *def;
            use = new std::set<int>();
            def = new std::set<int>();
            for (auto it : *FLOW::FG_use(it)) use->insert(it);
            for (auto it : *FLOW::FG_def(it)) def->insert(it);
            InOutTable.insert(std::make_pair(it, new InOut(use, def)));
        }
    }
}

static int gi = 0;

static bool LivenessInteration(GRAPH::NodeList* gl) {
    bool changed = false;
    gi++;
    for (auto n : *gl) {
        // do in[n] = use[n] union (out[n] - def[n])
        std::set<int>* newIn = new std::set<int>();
        std::set<int>* newOut = new std::set<int>();
        std::set<int> tempset;
        InOut* node = InOutTable.at(n);
        std::set_difference(node->out->begin(), node->out->end(), node->def->begin(),
                            node->def->end(), std::inserter(tempset, tempset.begin()));
        std::set_union(node->use->begin(), node->use->end(),tempset.begin(),tempset.end(),std::inserter(*newIn,newIn->begin()));
        // Now do out[n]=union_s in succ[n] (in[s])
        GRAPH::NodeList* s = n->succ();
        for (auto it : *s) {
            InOut* succ=InOutTable.at(it);
            for(auto ite:*succ->in){
                newOut->insert(ite);
            }
        }
        // See if any in/out changed
        if(*node->in!=*newIn||*node->out!=*newOut)changed=true;
        node->in=newIn;node->out=newOut;
    }
    return changed;
}

GRAPH::NodeList* LIVENESS::Liveness(GRAPH::NodeList* l) {
    init_INOUT(l);  // Initialize InOut table
    bool changed = true;
    while (changed) changed = LivenessInteration(l);
    return l;
}
std::set<int>* LIVENESS::FG_Out(GRAPH::Node* node){return InOutTable.at(node)->out;}
std::set<int>* LIVENESS::FG_In(GRAPH::Node* node){return InOutTable.at(node)->in;}