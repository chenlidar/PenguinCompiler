#include <assert.h>
#include <algorithm>
#include "liveness.hpp"
using namespace LIVENESS;

std::set<int>* Liveness::FG_Out(GRAPH::Node* node) { return InOutTable.at(node)->out; }
std::set<int>* Liveness::FG_In(GRAPH::Node* node) { return InOutTable.at(node)->in; }
void Liveness::analysis() {
    assert(workset.empty());
    for (auto n : *flowgraph->nodes()) {
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