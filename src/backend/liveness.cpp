#include "liveness.hpp"
#include <unordered_map>
#include "flowgraph.hpp"
#include <assert.h>
#include <algorithm>
#include <stack>
using namespace LIVENESS;
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

static std::unordered_map<GRAPH::Node*, InOut*> InOutTable;
static std::stack<GRAPH::Node*> workset;
static int gi = 0;
static void init_INOUT(std::vector<GRAPH::Node*>* l) {
    for (auto& it : InOutTable) {
        delete it.second->def;
        delete it.second->out;
        delete it.second->use;
        delete it.second->in;
    }
    InOutTable.clear();
    while (!workset.empty()) workset.pop();
    gi = 0;
    for (auto it : *l) {
        if (InOutTable.count(it) == 0) {
            std::set<int>*use, *def, *in, *out;
            use = new std::set<int>();
            in = new std::set<int>();
            out = new std::set<int>();
            def = new std::set<int>();
            for (auto it : *FLOW::FG_use(it)) { use->insert(it); }
            for (auto it : *FLOW::FG_def(it)) def->insert(it);
            InOutTable.insert(std::make_pair(it, new InOut(use, def, in, out)));
        }
    }
}
static void LivenessInteration(std::vector<GRAPH::Node*>* gl) {
    // assert(InOutTable.empty());
    assert(workset.empty());
    for (auto n : *gl) {
        workset.push(n);
        assert(InOutTable.at(n)->in->size() == 0);
        assert(InOutTable.at(n)->out->size() == 0);
    }
    while (!workset.empty()) {
        GRAPH::Node* n = workset.top();
        workset.pop();
        std::set<int>* newIn = new std::set<int>();
        std::set<int>* newOut = new std::set<int>();
        std::set<int> tempset;
        assert(tempset.empty());
        InOut* node = InOutTable.at(n);
        // Now do out[n]=union_s in succ[n] (in[s])
        GRAPH::NodeList* s = n->succ();
        for (auto it : *s) {
            InOut* succ = InOutTable.at(it);
            for (auto ite : *succ->in) { newOut->insert(ite); }
        }
        std::set_difference(newOut->begin(), newOut->end(), node->def->begin(), node->def->end(),
                            std::inserter(tempset, tempset.begin()));
        std::set_union(node->use->begin(), node->use->end(), tempset.begin(), tempset.end(),
                       std::inserter(*newIn, newIn->begin()));

        if (*node->in != *newIn) {
            assert(newIn->size() > node->in->size());
            assert(newIn->size() <= InOutTable.size());
            GRAPH::NodeList* pred = n->pred();
            for (auto it : *pred) {
                assert(it != n);
                workset.push(it);
            }
        }
        delete node->in;
        delete node->out;
        node->in = newIn;
        node->out = newOut;
    }
}

std::vector<GRAPH::Node*>* LIVENESS::Liveness(std::vector<GRAPH::Node*>* l) {
    init_INOUT(l);  // Initialize InOut table
    // int cnt=0;
    // for(auto it:*l)cnt++;
    // std::cerr<<"node number"<<cnt<<std::endl;
    LivenessInteration(l);
    return l;
}
std::set<int>* LIVENESS::FG_Out(GRAPH::Node* node) { return InOutTable.at(node)->out; }
std::set<int>* LIVENESS::FG_In(GRAPH::Node* node) { return InOutTable.at(node)->in; }