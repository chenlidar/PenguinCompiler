#include <assert.h>
#include "BuildSSA.hpp"
#include "../util/utils.hpp"
#include "Dtree.hpp"
// using namespace SSA;
SSA::SSAIR::SSAIR(CANON::Block blocks)
    : CFG::CFGraph(blocks) {
    // A. stmlist -> graph
    // B. Dominator tree
    dtree = new DTREE::Dtree(this, 0);
    // C. Dominance frontiers
    dtree->computeDF();
    // D. insert phi function
    placePhi();
    // E. rename temp, output a graph
    rename();
    endlabel = blocks.label;
    opt.ir = this;
}
void SSA::SSAIR::placePhi() {
    defsites = std::unordered_map<Temp_Temp, std::vector<int>>();
    for (int i = 0; i < nodecount; i++) {
        for (auto dst : orig[i]) { defsites[dst].push_back(i); }
    }
    std::unordered_set<int> worklist;
    for (auto dst : defsites) {
        worklist.clear();
        for (auto node : dst.second) worklist.insert(node);
        while (!worklist.empty()) {
            int node = *worklist.begin();
            worklist.erase(node);
            for (auto y : dtree->DF[node]) {
                if (Aphi[y].count(dst.first)) continue;
                if (y == exitnum) continue;
                IR::ExpList param = IR::ExpList();
                for (int i = 0; i < mynodes[y]->pred()->size(); i++)
                    param.push_back(new IR::Temp(dst.first));
                IR::Stm* phifunc = new IR::Move(new IR::Temp(dst.first), new IR::Call("$", param));
                IR::StmList* stmlist = (IR::StmList*)this->mynodes[y]->nodeInfo();
                stmlist->tail = new IR::StmList(phifunc, stmlist->tail);
                Aphi[y].insert(std::make_pair(dst.first, stmlist->tail));
                if (!orig[y].count(dst.first)) {
                    worklist.insert(y);
                    orig[y].insert(dst.first);
                }
            }
        }
    }
}
void SSA::SSAIR::rename() {
    rename(0);
    std::vector<std::pair<Temp_Temp, IR::StmList*>> v;
    for (int i = 0; i < nodecount; i++) {
        v.clear();
        for (auto it : Aphi[i]) { v.push_back(it); }
        Aphi[i].clear();
        for (auto& it : v) {
            it.first = static_cast<IR::Temp*>(*getDef(it.second->stm))->tempid;
            Aphi[i].insert(it);
        }
    }
}
void SSA::SSAIR::rename(int node) {
    IR::StmList* stmlist = (IR::StmList*)mynodes[node]->nodeInfo();
    IR::vector<Temp_Temp> rev = IR::vector<Temp_Temp>();
    for (; stmlist; stmlist = stmlist->tail) {
        IR::Stm* stm = stmlist->stm;
        if (stm->kind == IR::stmType::move
            && static_cast<IR::Move*>(stm)->src->kind == IR::expType::call
            && static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->fun[0] == '$')
            ;  // is phi,do nothing
        else {
            // change use
            std::vector<IR::Exp**> v = getUses(stm);
            for (IR::Exp** usev : v) {
                if (stk[static_cast<IR::Temp*>(*usev)->tempid].empty()) {
                    static_cast<IR::Temp*>(*usev)->tempid = Temp_newtemp();  // var not decl
                } else {
                    static_cast<IR::Temp*>(*usev)->tempid
                        = stk[static_cast<IR::Temp*>(*usev)->tempid].top();
                }
            }
        }
        // change dst
        IR::Exp** dst = getDef(stm);
        if (dst != nullptr) {
            Temp_Temp temp = Temp_newtemp();
            stk[static_cast<IR::Temp*>(*dst)->tempid].push(temp);
            rev.push_back(static_cast<IR::Temp*>(*dst)->tempid);
            static_cast<IR::Temp*>(*dst)->tempid = temp;
        }
    }
    for (auto succn : *mynodes[node]->succ()) {
        int succ = succn->mykey;
        for (auto it : Aphi[succ]) {
            IR::Move* phimove = static_cast<IR::Move*>(it.second->stm);
            IR::Call* phicall = static_cast<IR::Call*>(phimove->src);
            int cnt = 0;
            for (auto pred : prednode[succ]) {
                if (pred == node) {
                    if (stk[it.first].empty())
                        static_cast<IR::Temp*>(phicall->args[cnt])->tempid = Temp_newtemp();
                    else {
                        static_cast<IR::Temp*>(phicall->args[cnt])->tempid = stk[it.first].top();
                    }
                    break;
                }
                cnt++;
            }
        }
    }
    for (auto succ : dtree->children[node]) rename(succ);
    for (auto var : rev) stk[var].pop();
}

CANON::Block SSA::SSAIR::ssa2ir() {
    for (int i = 0; i < nodecount; i++) {
        if (mynodes[i]->inDegree() <= 1) continue;  // one pred or no pred nodedont have phi
        if (Aphi[i].size() == 0) continue;  // exitnode must has 0 phi
        Temp_Label label = static_cast<IR::Label*>(blocklabel[i]->stm)->label;
        int cnt = 0;
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
                prednode.push_back(std::vector<int>({pre}));
                pre = prenode;
            }
            for (auto it : Aphi[i]) {
                auto movephi = static_cast<IR::Move*>(it.second->stm);
                auto callphi = static_cast<IR::Call*>(movephi->src);
                IR::Exp* paramexp = callphi->args[cnt];
                int dsttemp = static_cast<IR::Temp*>(movephi->dst)->tempid;
                blockjump[pre]->tail = new IR::StmList(blockjump[pre]->stm, nullptr);
                blockjump[pre]->stm = new IR::Move(new IR::Temp(dsttemp), paramexp);
                blockjump[pre] = blockjump[pre]->tail;
            }
            cnt++;
        }
        for (auto it : Aphi[i]) it.second->stm = new IR::ExpStm(new IR::Const(0));
    }
    CANON::Block b;
    CANON::StmListList *head = new CANON::StmListList(nullptr, nullptr), *tail;
    tail = head;
    for (int i = 0; i < nodecount; i++) {
        if (mynodes[i]->inDegree() == 0 && i != 0) continue;
        if (i == exitnum) continue;
        tail = tail->tail = new CANON::StmListList((IR::StmList*)mynodes[i]->nodeInfo(), nullptr);
    }
    b.llist = head->tail;
    b.label = endlabel;
    return b;
}