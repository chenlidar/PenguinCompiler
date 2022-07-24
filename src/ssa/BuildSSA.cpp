#include "BuildSSA.hpp"
#include "../util/utils.hpp"
using namespace SSA;
SSAIR::SSAIR(CANON::Block blocks)
    : CFG::CFGraph(blocks) {
    // A. stmlist -> graph
    // B. Dominator tree
    dtree = new DTREE::Dtree(this);
    // C. Dominance frontiers
    dtree->computeDF();
    // D. insert phi function
    placePhi();
    // E. rename temp, output a graph
    rename();
}
void SSAIR::placePhi() {
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
                IR::Stm* phifunc
                    = new IR::Move(new IR::Temp(dst.first),
                                   new IR::Call("$", IR::ExpList({})));  // push_back in rename
                IR::StmList* stmlist = (IR::StmList*)this->mynodes[y]->nodeInfo();
                stmlist->tail = new IR::StmList(phifunc, stmlist->tail);
                Aphi[y].insert(
                    std::make_pair(dst.first, std::make_pair(phifunc, std::vector<int>())));
                if (!orig[y].count(dst.first)) {
                    worklist.insert(y);
                    orig[y].insert(dst.first);
                }
            }
        }
    }
}
void SSAIR::rename() { rename(0); }
void SSAIR::rename(int node) {
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
                    *usev = new IR::Const(0);  // var not decl replace with const 0
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
            IR::Move* phimove = static_cast<IR::Move*>(it.second.first);
            IR::Call* phicall = static_cast<IR::Call*>(phimove->src);
            phicall->args.push_back(new IR::Temp(stk[it.first].top()));
            it.second.second.push_back(node);
        }
    }
    for (auto succ : dtree->children[node]) rename(succ);
    for (auto var : rev) stk[var].pop();
}