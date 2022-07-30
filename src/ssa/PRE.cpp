#include "BuildSSA.hpp"
#include "../util/utils.hpp"
namespace SSA {
void Optimizer::PRE() {
    ir->dtree=new DTREE::Dtree(ir,0);
    // A. build table
    buildTable();
    // B. insert phi, computation
    insertPRE();
    // C. delete
    deletePRE();
}
void Optimizer::buildTable() {
    // use dtree
    avail = std::vector<Vtb>(ir->nodecount, Vtb());
    exp_gen = std::vector<std::vector<Biexp>>(ir->nodecount, std::vector<Biexp>());
    tmp_gen = std::vector<std::unordered_set<int>>(ir->nodecount, std::unordered_set<int>());
    phicomp = std::vector<std::unordered_map<int, Biexp>>(ir->nodecount, std::unordered_map<int, Biexp>());
    buildAvail(0, -1);
    anticIn = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    anticOut = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    // buildAntic();
}
void Optimizer::buildAvail(int node, int fa) {
    if (fa != -1) avail[node] = avail[fa];
    IR::StmList* list = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
    for (; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        IR::Exp** dst = getDef(stm);
        if (!dst) continue;
        assert(stm->kind == IR::stmType::move);
        IR::Move* mvstm = static_cast<IR::Move*>(stm);
        if (mvstm->src->kind == IR::expType::binop
            || mvstm->src->kind == IR::expType::call
                   && static_cast<IR::Call*>(mvstm->src)->fun[0] == '$')
            ;
        else {
            tmp_gen[node].insert(static_cast<IR::Temp*>(*dst)->tempid);
            continue;
        }
        int tempid = static_cast<IR::Temp*>(mvstm->dst)->tempid;
        switch (mvstm->src->kind) {
        case IR::expType::binop: {
            IR::Binop* bistm = static_cast<IR::Binop*>(mvstm->src);
            Biexp biexp(bistm->op, bistm->left, bistm->right);
            avail[node].insert({biexp, tempid});  // insert will not cover
            exp_gen[node].push_back(biexp);
        } break;
        case IR::expType::call: break;  // must be a phi
        default: assert(0);
        }
    }
    for (auto dom : ir->dtree->children[node]) buildAvail(dom, node);
}
void Optimizer::buildAntic() {
    std::stack<int> worklist;
    for (int i = ir->nodecount - 1; i >= 0; i--) {
        auto it = ir->nodes()->at(i);
        if (it->inDegree()) worklist.push(it->mykey);
    }  // FIXME: use dfs from exitnum
    worklist.push(ir->exitnum);
    while (!worklist.empty()) {
        int node = worklist.top();
        worklist.pop();
        std::list<Biexp> newin, newout;
        std::unordered_set<Biexp, hash_name> vis;
        if (ir->nodes()->at(node)->outDegree() == 1) {
            int cnt = 0;
            int succnode = (*ir->nodes()->at(node)->succs.begin())->mykey;
            for (auto pred : ir->prednode[succnode]) {
                if (node == pred) break;
                cnt++;
            }
            for (auto it : anticIn[succnode]) {
                auto nw = it;
                if (nw.l->kind == IR::expType::temp) {
                    int tempid = static_cast<IR::Temp*>(nw.l)->tempid;
                    if (ir->Aphi[succnode].count(tempid)) {
                        nw.l = static_cast<IR::Call*>(
                                   static_cast<IR::Move*>(ir->Aphi[succnode].at(tempid)->stm)->src)
                                   ->args.at(cnt);
                    }
                }
                if (nw.r->kind == IR::expType::temp) {
                    int tempid = static_cast<IR::Temp*>(nw.r)->tempid;
                    if (ir->Aphi[succnode].count(tempid)) {
                        nw.r = static_cast<IR::Call*>(
                                   static_cast<IR::Move*>(ir->Aphi[succnode].at(tempid)->stm)->src)
                                   ->args.at(cnt);
                    }
                }
                newout.push_back(nw);
            }
        } else if (ir->nodes()->at(node)->outDegree() == 2) {  //==2
            vis.clear();
            int cnt = 0;
            int succnode[2];
            for (auto succ : *ir->nodes()->at(node)->succ()) {
                succnode[cnt] = succ->mykey;
                cnt++;
            }
            for (auto it : anticIn[succnode[1]]) vis.insert(it);
            for (auto it : anticIn[succnode[0]])
                if (vis.count(it)) newout.push_back(it);
        } else
            assert(node == ir->exitnum);
        // anticIn
        for (auto it : newout) {
            if (it.l->kind == IR::expType::temp
                && tmp_gen[node].count(static_cast<IR::Temp*>(it.l)->tempid))
                continue;
            if (it.r->kind == IR::expType::temp
                && tmp_gen[node].count(static_cast<IR::Temp*>(it.r)->tempid))
                continue;
            newin.push_back(it);
        }
        for (int i = exp_gen[node].size() - 1; i >= 0; i--) {
            auto it = exp_gen[node][i];
            if (it.l->kind == IR::expType::temp
                && tmp_gen[node].count(static_cast<IR::Temp*>(it.l)->tempid))
                continue;
            if (it.r->kind == IR::expType::temp
                && tmp_gen[node].count(static_cast<IR::Temp*>(it.r)->tempid))
                continue;
            newin.push_front(it);
        }
        if (newin != anticIn[node]) {
            anticIn[node] = std::move(newin);
            for (auto prednode : *ir->nodes()->at(node)->pred()) {
                worklist.push(prednode->mykey);
            }
        }
    }
}
void SSA::Optimizer::insertPRE() {
    for (auto it : *ir->nodes()) {
        if (it->inDegree() <= 1) continue;
        // TODO:
    }
}
void SSA::Optimizer::deletenode(int node, int fa) {
    if (fa != -1) avail[node] = avail[fa];
    IR::StmList* list = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
    for (; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        IR::Exp** dst = getDef(stm);
        if (!dst) continue;
        assert(stm->kind == IR::stmType::move);
        IR::Move* mvstm = static_cast<IR::Move*>(stm);
        if (mvstm->src->kind == IR::expType::binop
            || mvstm->src->kind == IR::expType::call
                   && static_cast<IR::Call*>(mvstm->src)->fun[0] == '$')
            ;
        else
            continue;
        int tempid = static_cast<IR::Temp*>(mvstm->dst)->tempid;
        switch (mvstm->src->kind) {
        case IR::expType::binop: {
            IR::Binop* bistm = static_cast<IR::Binop*>(mvstm->src);
            Biexp biexp(bistm->op, bistm->left, bistm->right);
            if (avail[node].count(
                    biexp)) {  // already has,full redundancy,replace this with temp<-temp
                mvstm->src = new IR::Temp(avail[node].at(biexp));
            } else {
                avail[node].insert({biexp, tempid});  // insert will not cover
            }

        } break;
        case IR::expType::call: {  // must be a phi
            if (phicomp[node].count(tempid)) {  // handle as a binop
                avail[node].insert({phicomp[node][tempid], tempid});
            }
        } break;
        default: assert(0);
        }
    }
    for (auto dom : ir->dtree->children[node]) deletenode(dom, node);
}
void SSA::Optimizer::deletePRE() {
    avail = std::vector<Vtb>(ir->nodecount, Vtb());
    deletenode(0, -1);
}
}  // namespace SSA