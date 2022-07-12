#include "flowgraph.hpp"
#include <unordered_map>
#include <assert.h>
/* Interfaces */
using LabelNodeMap = std::unordered_map<Temp_Label, GRAPH::Node*>;

struct UDinfo {
    Temp_TempList uses;
    Temp_TempList defs;
    bool isMove;
    UDinfo(Temp_TempList _uses, Temp_TempList _defs, bool _isMove)
        : uses(_uses)
        , defs(_defs)
        , isMove(_isMove) {}
};
using NodeInfoMap = std::unordered_map<GRAPH::Node*, UDinfo*>;
static void UD_enter(GRAPH::Node* n, UDinfo* info);
static UDinfo* UD_lookup(GRAPH::Node* n);
static LabelNodeMap* LNTable();
static void LT_enter(GRAPH::Node* l, UDinfo* n);
static GRAPH::Node* LT_lookup(Temp_Label l);

/* Implementation */
static NodeInfoMap* UDTable = nullptr;

static void UD_init() { UDTable = new NodeInfoMap(); }

static void UD_enter(GRAPH::Node* n, UDinfo* info) { UDTable->insert(std::make_pair(n, info)); }

static UDinfo* UD_lookup(GRAPH::Node* n) {
    if (UDTable->find(n) != UDTable->end()) {
        return UDTable->at(n);
    } else
        return nullptr;
}
static LabelNodeMap* _lntable = nullptr;

static LabelNodeMap* LNTable() {
    if (_lntable == nullptr) { _lntable = new LabelNodeMap(); }
    return _lntable;
}

static GRAPH::Node* LT_lookup(Temp_Label l) {
    if (LNTable()->find(l) != LNTable()->end()) {
        return LNTable()->at(l);
    } else
        return nullptr;
}

static void LT_enter(Temp_Label l, GRAPH::Node* n) { LNTable()->insert(std::make_pair(l, n)); }

Temp_TempList* FLOW::FG_def(GRAPH::Node* n) { return &UD_lookup(n)->defs; }

Temp_TempList* FLOW::FG_use(GRAPH::Node* n) { return &UD_lookup(n)->uses; }

bool FLOW::FG_isMove(GRAPH::Node* n) { return UD_lookup(n)->isMove; }

static constexpr int IT_COMMON = 0;
static constexpr int IT_JUMP = 1;
static constexpr int IT_MOVE = 2;
GRAPH::Graph* FLOW::FG_AssemFlowGraph(ASM::InstrList* il) {
    UD_init();

    //(I) Iterate over the entire instruction list
    GRAPH::Node* prev = nullptr;
    GRAPH::Node* curr = nullptr;
    GRAPH::Graph* graph = new GRAPH::Graph();
    GRAPH::NodeList* jumpList = new GRAPH::NodeList();
    for (auto instr : *il) {
        if (instr != nullptr) {
            // 1) create a node (and put it into the graph), using the
            //    instruction as the associated info.
            curr = graph->addNode(instr);

            // 2) special handling
            int type = IT_COMMON;
            Temp_TempList defs = Temp_TempList();
            Temp_TempList uses = Temp_TempList();
            switch (instr->kind) {
            case ASM::InstrType::oper:
                // Check if it's a JUMP instruction
                // We do this check here by looking if As_target is null,
                // instead of inspecting the assembly op (j, beq, ...)
                if (!static_cast<ASM::Oper*>(instr)->jumps.empty()) {
                    type = IT_JUMP;
                    // put this instruction into a separate list
                    jumpList->push_back(curr);
                }
                defs = static_cast<ASM::Oper*>(instr)->dst;
                uses = static_cast<ASM::Oper*>(instr)->src;
                break;
            case ASM::InstrType::label:
                // 2.2) label should be also saved in the label-node list for (II)
                LT_enter(static_cast<ASM::Label*>(instr)->label, curr);
                break;
            case ASM::InstrType::move:
                // 2.3) it's a move instruction
                type = IT_MOVE;
                defs = static_cast<ASM::Move*>(instr)->dst;
                uses = static_cast<ASM::Move*>(instr)->src;
                assert(defs.size() == 1 && uses.size() == 1);
                break;
            }

            // 3) put information into table
            UD_enter(curr, new UDinfo(uses, defs, type == IT_MOVE));

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
        Temp_Label label;
        GRAPH::Node* dest;
        // for each target it may jump to, add a corresponding edge in the graph
        for (auto& label : labels) {
            // label = labels->head;
            if (label.size()) {
                // quickly retieve the target node using the label-node table
                dest = LT_lookup(label);
                // establish edge between this node and its jump target
                curr->mygraph->addEdge(curr, dest);
            }
        }
    }

    return graph;
}
