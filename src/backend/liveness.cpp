#include "liveness.hpp"
#include <unordered_map>
#include "flowgraph.hpp"
using namespace LIVENESS;
// structure for in and out temps to attach to the flowgraph nodes
struct InOut {
    Temp_TempList* in;
    Temp_TempList* out;
    InOut(Temp_TempList* _in, Temp_TempList* _out)
        : in(in)
        , out(out) {}
};

// This is the (global) table for storing the in and out temps
static std::unordered_map<GRAPH::Node*, InOut*>* InOutTable = nullptr;

// initialize the table
static void init_INOUT() {
    if (InOutTable == nullptr) { InOutTable = new std::unordered_map<GRAPH::Node*, InOut*>(); }
}

// Attach the inOut info to the table
static void INOUT_enter(GRAPH::Node* n, InOut* info) {
    InOutTable->insert(std::make_pair(n, info));
}

// Lookup the inOut info
static InOut* INOUT_lookup(GRAPH::Node* n) {
    if (InOutTable->find(n) != InOutTable->end()) {
        return InOutTable->at(n);
    } else
        return nullptr;
}

// do a simple union (add each one at a time)
static Temp_TempList* TempList_union(Temp_TempList* tl1, Temp_TempList* tl2) {
    for (const auto& x : *tl2) tl1->push_back(x);
    return tl1;
}

// Implement a list difference tl1-tl2 (for each on in tl1, scan tl2)
static Temp_TempList* TempList_diff(Temp_TempList* tl1, Temp_TempList* tl2) {
    bool found;
    Temp_TempList* result = nullptr;
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
    if (TempList_diff(tl1, tl2) != NULL)
        return false;
    else {
        if (TempList_diff(tl2, tl1) != NULL)
            return false;
        else
            return true;
    }
}

Temp_TempList* LIVENESS::FG_In(GRAPH::Node* n) {
    InOut* io;
    io = INOUT_lookup(n);
    if (io != NULL)
        return io->in;
    else
        return NULL;
}

Temp_TempList* LIVENESS::FG_Out(GRAPH::Node* n) {
    InOut* io;
    io = INOUT_lookup(n);
    if (io != NULL)
        return io->out;
    else
        return NULL;
}

// initialize the INOUT info for a graph
static void init_INOUT_graph(GRAPH::NodeList* l) {
    for (auto it : *l) {
        if (INOUT_lookup(it) == nullptr)  // If there is no io info yet, initialize one
            INOUT_enter(it, new InOut(nullptr, nullptr));
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
        GRAPH::NodeList* s = &n->succ();
        Temp_TempList* out = new Temp_TempList();  // out is an accumulator
        for (auto it : *s) { out = TempList_union(out, FG_In(it)); }
        // See if any in/out changed
        if (!(TempList_eq(FG_In(n), in) && TempList_eq(FG_Out(n), out))) changed = true;
        // enter the new info
        InOutTable->insert(std::make_pair(n, new InOut(in, out)));
    }
    return changed;
}

GRAPH::NodeList* Liveness(GRAPH::NodeList* l) {
    init_INOUT();  // Initialize InOut table
    bool changed = true;
    while (changed) changed = LivenessInteration(l);
    return l;
}
