#include "CFG.hpp"
namespace CFG {
CFGraph::CFGraph(CANON::Block blocks) {
    LNTable = std::unordered_map<Temp_Label, GRAPH::Node*>();
    for (CANON::StmListList* llist = blocks.llist; llist; llist = llist->tail) {  // add node
        IR::StmList* stmlist = llist->head;
        GRAPH::Node* node = this->addNode(stmlist);
        orig.push_back(std::unordered_set<Temp_Temp>());
        IR::Stm* stm = stmlist->stm;
        assert(stm->kind == IR::stmType::label);
        Temp_Label label = static_cast<IR::Label*>(stm)->label;
        LNTable.insert(std::make_pair(label, node));
    }
    GRAPH::Node* endnode = this->addNode(new IR::StmList(new IR::Label(blocks.label), nullptr));
    exitnum = endnode->mykey;
    orig.push_back(std::unordered_set<Temp_Temp>());
    Temp_Label label = "RETURN";
    LNTable.insert(std::make_pair(label, endnode));
    for (auto node : mynodes) {
        if (node == endnode) continue;  // last node
        IR::StmList* stmlist = (IR::StmList*)node->nodeInfo();
        IR::Stm* stm;
        auto list = stmlist;
        for (; list->tail; list = list->tail) {
            stm = list->stm;
            switch (stm->kind) {
            case IR::stmType::move: {
                orig[node->mykey].insert(
                    static_cast<IR::Temp*>(static_cast<IR::Move*>(stm)->dst)->tempid);
            } break;
            case IR::stmType::jump:
            case IR::stmType::cjump:
            case IR::stmType::label:
            case IR::stmType::exp: break;
            default: assert(0);
            }
        }
        stm = list->stm;
        switch (stm->kind) {
        case IR::stmType::jump: {
            IR::Jump* jmp = static_cast<IR::Jump*>(stm);
            Temp_Label label = jmp->target;
            GRAPH::Node* dst = LNTable.at(label);
            node->mygraph->addEdge(node, dst);
        } break;
        case IR::stmType::cjump: {
            IR::Cjump* cjmp = static_cast<IR::Cjump*>(stm);
            Temp_Label tl = cjmp->trueLabel;
            Temp_Label fl = cjmp->falseLabel;
            GRAPH::Node* dst = LNTable.at(tl);
            node->mygraph->addEdge(node, dst);
            dst = LNTable.at(fl);
            node->mygraph->addEdge(node, dst);
        } break;
        default: assert(0);
        }
    }
}
};  // namespace CFG