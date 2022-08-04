#include "CFG.hpp"
#include "../util/utils.hpp"
namespace CFG {
CFGraph::CFGraph(CANON::Block blocks) {
    LNTable = std::unordered_map<Temp_Label, GRAPH::Node*>();
    for (CANON::StmListList* llist = blocks.llist; llist; llist = llist->tail) {  // add node
        IR::StmList* stmlist = llist->head;
        GRAPH::Node* node = this->addNode(stmlist);
        orig.push_back(std::set<Temp_Temp>());
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
    orig.push_back(std::set<Temp_Temp>());
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
        while (mynodes[i]->preds.size())
            this->rmEdge(mynodes[*mynodes[i]->preds.begin()], mynodes[i]);
        while (mynodes[i]->succs.size())
            this->rmEdge(mynodes[i], mynodes[*mynodes[i]->succs.begin()]);
        orig[i].clear();
    }
    for (int i = 0; i < nodecount; i++) {
        if (!exist[i]) continue;
        for (auto pred : *mynodes[i]->pred()) prednode[i].push_back(pred);
    }
    cut_edge();
}
void CFGraph::dfs(int node) {
    exist[node] = true;
    for (auto it : *mynodes[node]->succ()) {
        if (exist[it]) continue;
        dfs(it);
    }
}
void CFGraph::cut_edge() {
    int nodenum = nodecount;
    for (int i = 0; i < nodenum; i++) {
        if (mynodes[i]->inDegree() <= 1) continue;
        Temp_Label label = static_cast<IR::Label*>(blocklabel[i]->stm)->label;
        for (auto& pre : prednode[i]) {
            if (blockjump[pre]->stm->kind == IR::stmType::cjump) {
                Temp_Label prelabel;
                int prenode = pre;
                IR::Stm* cjmp = blockjump[pre]->stm;
                IR::StmList* prelist;
                if (static_cast<IR::Cjump*>(cjmp)->trueLabel == label) {  // truelabel
                    prelabel = Temp_newlabel();
                    static_cast<IR::Cjump*>(cjmp)->trueLabel = prelabel;
                    prelist = new IR::StmList(new IR::Label(prelabel),
                                              new IR::StmList(new IR::Jump(label), NULL));
                    prenode = this->addNode(prelist)->mykey;
                } else {  // falselabel
                    prelabel = Temp_newlabel();
                    static_cast<IR::Cjump*>(cjmp)->falseLabel = prelabel;
                    prelist = new IR::StmList(new IR::Label(prelabel),
                                              new IR::StmList(new IR::Jump(label), NULL));
                    prenode = this->addNode(prelist)->mykey;
                }
                this->rmEdge(mynodes[pre], mynodes[i]);
                this->addEdge(mynodes[pre], mynodes[prenode]);
                this->addEdge(mynodes[prenode], mynodes[i]);
                blocklabel.push_back(prelist);
                blockjump.push_back(prelist->tail);
                orig.push_back(std::set<Temp_Temp>());
                prednode.push_back(std::vector<int>({pre}));
                pre = prenode;
            }
        }
    }
}
};  // namespace CFG