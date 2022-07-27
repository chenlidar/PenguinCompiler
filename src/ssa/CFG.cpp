#include "CFG.hpp"
#include "../util/utils.hpp"
namespace CFG {
CFGraph::CFGraph(CANON::Block blocks) {
    LNTable = std::unordered_map<Temp_Label, GRAPH::Node*>();
    for (CANON::StmListList* llist = blocks.llist; llist; llist = llist->tail) {  // add node
        IR::StmList* stmlist = llist->head;
        GRAPH::Node* node = this->addNode(stmlist);
        orig.push_back(std::unordered_set<Temp_Temp>());
        blocklabel.push_back(stmlist);
        IR::Stm* stm = stmlist->stm;
        assert(stm->kind == IR::stmType::label);
        Temp_Label label = static_cast<IR::Label*>(stm)->label;
        LNTable.insert(std::make_pair(label, node));
    }
    Temp_Label label = blocks.label;
    auto endlabelstm = new IR::StmList(new IR::Label(label), nullptr);
    GRAPH::Node* endnode = this->addNode(endlabelstm);
    exitnum = endnode->mykey;
    orig.push_back(std::unordered_set<Temp_Temp>());
    blocklabel.push_back(endlabelstm);
    LNTable.insert(std::make_pair(label, endnode));
    for (auto node : mynodes) {
        if (node == endnode) continue;  // last node
        IR::StmList* stmlist = (IR::StmList*)node->nodeInfo();
        IR::Stm* stm;
        auto list = stmlist;
        for (; list->tail; list = list->tail) {
            stm = list->stm;
            IR::Exp** def = getDef(stm);
            if (def) orig[node->mykey].insert(static_cast<IR::Temp*>(*def)->tempid);
        }
        blockjump.push_back(list);
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
    blockjump.push_back(nullptr);
    exist = std::vector<bool>(nodecount, false);
    prednode = std::vector<std::vector<int>>(nodecount, std::vector<int>());
    dfs(0);
    for (int i = 0; i < nodecount; i++) {
        if (exist[i]) continue;
        for (auto pred : *mynodes[i]->pred()) { pred->succs.erase(pred->succs.find(mynodes[i])); }
        mynodes[i]->preds.clear();
        for (auto succ : *mynodes[i]->succ()) { succ->preds.erase(succ->preds.find(mynodes[i])); }
        mynodes[i]->succs.clear();
        orig[i].clear();
    }
    for (int i = 0; i < nodecount; i++) {
        if (!exist[i]) continue;
        for (auto pred : *mynodes[i]->pred()) prednode[i].push_back(pred->mykey);
    }
}
void CFGraph::dfs(int node) {
    exist[node] = true;
    for (auto it : *mynodes[node]->succ()) {
        if (exist[it->mykey]) continue;
        dfs(it->mykey);
    }
}
};  // namespace CFG