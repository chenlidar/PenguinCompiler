#include "flowgraph.hpp"
#include <unordered_map>
#include <assert.h>
using namespace FLOW;

Temp_TempList* FlowGraph::FG_def(GRAPH::Node* n) { return &UDTable.at(n)->defs; }
Temp_TempList* FlowGraph::FG_use(GRAPH::Node* n) { return &UDTable.at(n)->uses; }
bool FlowGraph::FG_isMove(GRAPH::Node* n) { return UDTable.at(n)->isMove; }

static constexpr int IT_COMMON = 0;
static constexpr int IT_JUMP = 1;
static constexpr int IT_MOVE = 2;
void FlowGraph::FG_AssemFlowGraph(ASM::InstrList* il) {
    //(I) Iterate over the entire instruction list
    GRAPH::Node* prev = nullptr;
    GRAPH::Node* curr = nullptr;
    GRAPH::NodeList* jumpList = new GRAPH::NodeList();
    for (auto instr : *il) {
        if (instr != nullptr) {
            // 1) create a node (and put it into the graph), using the
            //    instruction as the associated info.
            curr = this->addNode(instr);

            // 2) special handling
            int type = IT_COMMON;
            Temp_TempList defs = Temp_TempList();
            Temp_TempList uses = Temp_TempList();
            switch (instr->kind) {
            case ASM::InstrType::oper:
                if (!static_cast<ASM::Oper*>(instr)->jumps.empty()) {
                    type = IT_JUMP;
                    // put this instruction into a separate list
                    jumpList->insert(curr);
                }
                defs = static_cast<ASM::Oper*>(instr)->dst;
                uses = static_cast<ASM::Oper*>(instr)->src;
                break;
            case ASM::InstrType::label:
                // 2.2) label should be also saved in the label-node list for (II)
                LNTable.insert(std::make_pair(static_cast<ASM::Label*>(instr)->label, curr));
                break;
            case ASM::InstrType::move:
                type = IT_MOVE;
                defs = static_cast<ASM::Move*>(instr)->dst;
                uses = static_cast<ASM::Move*>(instr)->src;
                assert(defs.size() == 1 && uses.size() == 1);
                break;
            }

            // 3) put information into table
            UDTable.insert(std::make_pair(curr, new UDinfo(uses, defs, type == IT_MOVE)));

            // 4) link with the previous node for falling through, if possible.
            //    Note that prev is NULL if the previous instruction is a JUMP.
            if (prev != NULL) { prev->mygraph->addEdge(prev, curr); }

            // 5) set as previous node for next time of iteration
            prev = (type != IT_JUMP) ? curr : NULL;
        }
    }

    //(II) Iterate over the list that has all the JUMP instruction collected.
    Temp_LabelList labels;
    for (auto& curr : *jumpList) {
        ASM::Instr* x = (ASM::Instr*)(curr->nodeInfo());
        labels = static_cast<ASM::Oper*>(x)->jumps;  // no need to check its nullity again
        GRAPH::Node* dest;
        // for each target it may jump to, add a corresponding edge in the graph
        for (auto& label : labels) {
            // label = labels->head;
            if (label.size()) {
                // quickly retieve the target node using the label-node table
                dest = LNTable.at(label);
                // establish edge between this node and its jump target
                curr->mygraph->addEdge(curr, dest);
            }
        }
    }

    return;
}
