#include "liveness.hpp"
#include <unordered_map>
#include "flowgraph.hpp"
#include <assert.h>
using namespace LIVENESS;
// structure for in and out temps to attach to the flowgraph nodes
struct InOut {
    Temp_TempList* in;
    Temp_TempList* out;
    InOut(Temp_TempList* _in, Temp_TempList* _out)
        : in(_in)
        , out(_out) {}
};

// This is the (global) table for storing the in and out temps
static std::unordered_map<GRAPH::Node*, InOut*>* InOutTable = nullptr;

// initialize the table
static void init_INOUT() {
    InOutTable = new std::unordered_map<GRAPH::Node*, InOut*>();
}

// do a simple union (add each one at a time)
static Temp_TempList* TempList_union(Temp_TempList* tl1, Temp_TempList* tl2) {
    Temp_TempList* ret=new Temp_TempList();
    for (const auto& x : *tl1) ret->push_back(x);
    for (const auto& x : *tl2) ret->push_back(x);
    return ret;
}

// Implement a list difference tl1-tl2 (for each on in tl1, scan tl2)
static Temp_TempList* TempList_diff(Temp_TempList* tl1, Temp_TempList* tl2) {
    bool found;
    Temp_TempList* result = new Temp_TempList();
    for (auto it : *tl1) {
        found = false;
        for (auto ite : *tl2) {
            if (it == ite) found = true;
        }
        if (!found) {  // if not found in tl2, then add to the result list
            result->push_back(it);
        }
    }
    return (result);
}

// a simple eq test using diff twice
static bool TempList_eq(Temp_TempList* tl1, Temp_TempList* tl2) {
    if (!TempList_diff(tl1, tl2)->empty())
        return false;
    else if (!TempList_diff(tl2, tl1)->empty())
        return false;
    else
        return true;
}

Temp_TempList* LIVENESS::FG_In(GRAPH::Node* n) {
    InOut* io;
    io = InOutTable->at(n);
    return io->in;
}

Temp_TempList* LIVENESS::FG_Out(GRAPH::Node* n) {
    InOut* io;
    io = InOutTable->at(n);;
    return io->out;
}

// initialize the INOUT info for a graph
static void init_INOUT_graph(GRAPH::NodeList* l) {
    for (auto it : *l) {
        if (InOutTable->count(it) == 0)  // If there is no io info yet, initialize one
            InOutTable->insert(
                std::make_pair(it, new InOut(new Temp_TempList(), new Temp_TempList())));
    }
}

static int gi = 0;

static bool LivenessInteration(GRAPH::NodeList* gl) {
    bool changed = false;
    gi++;
    for (auto n : *gl) {
        // do in[n] = use[n] union (out[n] - def[n])
        Temp_TempList* in
            = TempList_union(FLOW::FG_use(n), TempList_diff(FG_Out(n), FLOW::FG_def(n)));

        // Now do out[n]=union_s in succ[n] (in[s])
        GRAPH::NodeList* s = n->succ();
        Temp_TempList* out = new Temp_TempList();  // out is an accumulator
        for (auto it : *s) { out = TempList_union(out, FG_In(it)); }
        // See if any in/out changed
        if (!(TempList_eq(FG_In(n), in) && TempList_eq(FG_Out(n), out))) changed = true;
        // enter the new info
        InOutTable->at(n)=new InOut(in, out);
    }
    return changed;
}

GRAPH::NodeList* LIVENESS::Liveness(GRAPH::NodeList* l) {
    init_INOUT();  // Initialize InOut table
    init_INOUT_graph(l);
    bool changed = true;
    while (changed) changed = LivenessInteration(l);
    return l;
}
