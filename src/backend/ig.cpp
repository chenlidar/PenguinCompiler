#include "ig.hpp"
#include "flowgraph.hpp"
#include <assert.h>
static GRAPH::Graph* RA_ig;  // info of this graph is a Temp_Temp

void Ig_empty() { RA_ig = new GRAPH::Graph(); }

GRAPH::Graph* Ig_graph() { return RA_ig; }

GRAPH::Node* Look_ig(Temp_Temp t) {
    GRAPH::Node* n1 = NULL;
    for (const auto& x : RA_ig->nodes()) {
        if ((Temp_Temp)(x->nodeInfo()) == t) {
            n1 = x;
            break;
        }
    }
    if (n1 == NULL)
        return (RA_ig->addNode((void*)t));
    else
        return n1;
}

void Enter_ig(Temp_Temp t1, Temp_Temp t2) {
    GRAPH::Node* n1 = Look_ig(t1);
    GRAPH::Node* n2 = Look_ig(t2);
    // G_addEdge(n1, n2);
    n1->mygraph->addEdge(n1, n2);
    return;
}

// input flowgraph after liveness analysis (so FG_In and FG_Out are available)

GRAPH::NodeList* Create_ig(GRAPH::NodeList* flowgraph) {
    // DONE:You need to fill in here!
    RA_ig = new GRAPH::Graph();
    for (auto it : *flowgraph) {
        Temp_TempList* outList = LIVENESS::FG_Out(it);
        Temp_TempList* defList = FLOW::FG_def(it);
        Temp_TempList* useList = FLOW::FG_use(it);
        for (auto ite2 : *defList) Look_ig(ite2);
        for (auto ite2 : *useList) Look_ig(ite2);
        if (FLOW::FG_isMove(it)) {
            assert(useList->size() == 1);
            for (auto ite2 : *outList) {
                if (useList->at(0) != ite2) {
                    if (defList->at(0) != ite2) {
                        Enter_ig(defList->at(0), ite2);
                        Enter_ig(ite2, defList->at(0));
                    }
                }
            }
        } else {
            for (auto ite1 : *defList) {
                for (auto ite2 : *outList) {
                    if (ite1 != ite2) {
                        Enter_ig(ite1, ite2);
                        Enter_ig(ite2, ite1);
                    }
                }
            }
        }
    }
    return &RA_ig->nodes();
}

// static void show_temp(Temp_Temp t) {
//     fprintf(stdout, "%s, ", Temp_look(Temp_name(), t));
// }

// void Show_ig(FILE* out, G_nodeList l) {
//         G_show(out, l, (void*)show_temp);
// }
