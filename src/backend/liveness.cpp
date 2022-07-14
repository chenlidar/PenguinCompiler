#include "liveness.hpp"
#include <unordered_map>
#include "flowgraph.hpp"
#include <assert.h>
#include <algorithm>
#include <queue>
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

static int gi = 0;
static void init_INOUT(GRAPH::NodeList* l) {
    InOutTable = std::unordered_map<GRAPH::Node*, InOut*>();
    gi = 0;
    for (auto it : *l) {
        if (InOutTable.count(it) == 0) {
            std::set<int>*use, *def, *in, *out;
            use = new std::set<int>();
            in = new std::set<int>();
            out = new std::set<int>();
            def = new std::set<int>();
            for (auto it : *FLOW::FG_use(it)) {
                use->insert(it);
                in->insert(it);
            }
            for (auto it : *FLOW::FG_def(it)) def->insert(it);
            InOutTable.insert(std::make_pair(it, new InOut(use, def, in, out)));
        }
    }
}

static void LivenessInteration(GRAPH::NodeList* gl) {
    std::queue<GRAPH::Node*> workset;
    std::set<GRAPH::Node*> vis;
    for (auto n = gl->rbegin(); n != gl->rend(); ++n) {
        workset.push(*n);
        vis.insert(*n);
    }
    while (!workset.empty()) {
        GRAPH::Node* n = workset.front();
        workset.pop();
        vis.erase(vis.find(n));
        std::set<int>* newIn = new std::set<int>();
        std::set<int>* newOut = new std::set<int>();
        std::set<int> tempset;
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

        if (*node->out != *newOut) {
            GRAPH::NodeList* pred = n->pred();
            for (auto it : *pred) {
                if (vis.count(it))
                    ;
                else {
                    workset.push(it);
                    vis.insert(it);
                }
            }
        } else
            assert(*newIn == *node->in);
        delete node->in;
        delete node->out;
        node->in = newIn;
        node->out = newOut;
    }
}
// static bool LivenessInteration(GRAPH::NodeList* gl) {
//     bool changed = false;
//     gi++;
//     for (auto n = gl->rbegin(); n != gl->rend(); ++n) {
//         // do in[n] = use[n] union (out[n] - def[n])
//         std::set<int>* newIn = new std::set<int>();
//         std::set<int>* newOut = new std::set<int>();
//         std::set<int> tempset;
//         InOut* node = InOutTable.at(*n);
//         std::set_difference(node->out->begin(), node->out->end(), node->def->begin(),
//                             node->def->end(), std::inserter(tempset, tempset.begin()));
//         std::set_union(node->use->begin(), node->use->end(), tempset.begin(), tempset.end(),
//                        std::inserter(*newIn, newIn->begin()));
//         // Now do out[n]=union_s in succ[n] (in[s])
//         GRAPH::NodeList* s = (*n)->succ();
//         for (auto it : *s) {
//             InOut* succ = InOutTable.at(it);
//             for (auto ite : *succ->in) { newOut->insert(ite); }
//         }
//         // See if any in/out changed
//         if (*node->in != *newIn || *node->out != *newOut) changed = true;
//         delete node->in;
//         delete node->out;
//         node->in = newIn;
//         node->out = newOut;
//     }
//     return changed;
// }

GRAPH::NodeList* LIVENESS::Liveness(GRAPH::NodeList* l) {
    init_INOUT(l);  // Initialize InOut table
    // int cnt=0;
    // for(auto it:*l)cnt++;
    // std::cerr<<"node number"<<cnt<<std::endl;
    LivenessInteration(l);
    return l;
}
std::set<int>* LIVENESS::FG_Out(GRAPH::Node* node) { return InOutTable.at(node)->out; }
std::set<int>* LIVENESS::FG_In(GRAPH::Node* node) { return InOutTable.at(node)->in; }