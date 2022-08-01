#include "BuildSSA.hpp"
#include "../util/utils.hpp"
namespace SSA {
void Optimizer::PRE() {
    ir->dtree = new DTREE::Dtree(ir, 0);
    // A. build table
    buildTable();
    // B. insert phi, computation
    insertPRE();
    // C. delete
    deletePRE();
}
void Optimizer::buildTable() {
    // use dtree
    avail_out = std::vector<std::unordered_set<int>>(ir->nodecount, std::unordered_set<int>());
    avail_map
        = std::vector<std::unordered_map<int, int>>(ir->nodecount, std::unordered_map<int, int>());
    G_map.clear();
    // exp_gen = std::vector<std::vector<Biexp>>(ir->nodecount, std::vector<Biexp>());
    // tmp_gen = std::vector<std::unordered_set<int>>(ir->nodecount, std::unordered_set<int>());
    buildAvail();
    // anticIn = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    // anticOut = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    // buildAntic();
}
void Optimizer::buildAvail() { buildAvail(0, -1); }
Optimizer::Biexp Optimizer::U2Biexp(IR::Exp* e) {
    IR::Const const_temp = IR::Const(0);
    return Biexp(IR::binop::T_plus, e, &const_temp);
}
void Optimizer::buildAvail(int node, int fa) {
    auto findGV = [&](const Biexp& biexp) {
        int val = Temp_newtemp();
        if (!G_map.count(biexp)) {
            G_map[biexp] = val;
            IR::Temp temp = IR::Temp(val);
            G_map.insert({U2Biexp(&temp), val});
        }
        return G_map.at(biexp);
    };

    if (fa != -1) {
        avail_out[node] = avail_out[fa];
        avail_map[node] = avail_map[fa];
    }
    IR::StmList* list = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
    for (; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        IR::Exp** dst = getDef(stm);
        if (!dst) continue;
        assert(stm->kind == IR::stmType::move);
        IR::Move* mvstm = static_cast<IR::Move*>(stm);
        int dstval;
        int tempid = static_cast<IR::Temp*>(mvstm->dst)->tempid;
        if (mvstm->src->kind == IR::expType::binop
            || mvstm->src->kind == IR::expType::call
                   && static_cast<IR::Call*>(mvstm->src)->fun[0]
                          == '$') {  // temp <- s | (s op t) | phi
            switch (mvstm->src->kind) {
            case IR::expType::binop: {
                IR::Binop* bistm = static_cast<IR::Binop*>(mvstm->src);
                // update biexp
                int lval = findGV(U2Biexp(bistm->left));
                int rval = findGV(U2Biexp(bistm->right));
                IR::Temp tp1 = IR::Temp(lval), tp2 = IR::Temp(rval);
                Biexp biexp = Biexp(bistm->op, &tp1, &tp2);
                G_map.insert({U2Biexp(*dst), findGV(biexp)});
                dstval = G_map.at(biexp);
            } break;
            case IR::expType::call: {  // must be a phi
                bool same = true, alldef = true;
                int lastval = -1;
                IR::Call* phi = static_cast<IR::Call*>(mvstm->src);
                for (auto it : phi->args) {
                    if (!G_map.count(U2Biexp(it))) {
                        alldef = false;
                        break;
                    }
                    int currval = G_map.at(U2Biexp(it));
                    if (lastval == -1) {
                        lastval = currval;
                        continue;
                    }
                    if (lastval != currval) same = false;
                }
                if (alldef && same)
                    G_map.insert({U2Biexp(*dst), lastval});
                else
                    findGV(U2Biexp(*dst));
                dstval = findGV(U2Biexp(*dst));
            } break;
            default: assert(0);
            }
        }
        else {  // temp <- *
            assert(mvstm->src->kind != IR::expType::constx);
            // tmp_gen[node].insert(static_cast<IR::Temp*>(*dst)->tempid);
            dstval = findGV(U2Biexp(*dst));
        }
        if (!avail_map[node].count(dstval)) {
            avail_map[node][dstval] = tempid;
            avail_out[node].insert(tempid);
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
                if (nw.l.kind == IR::expType::temp) {
                    int tempid = nw.l.val;
                    if (ir->Aphi[succnode].count(tempid)) {
                        nw.l = static_cast<IR::Call*>(
                                   static_cast<IR::Move*>(ir->Aphi[succnode].at(tempid)->stm)->src)
                                   ->args.at(cnt);
                    }
                }
                if (nw.r.kind == IR::expType::temp) {
                    int tempid = nw.r.val;
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
            if (it.l.kind == IR::expType::temp && tmp_gen[node].count(it.l.val)) continue;
            if (it.r.kind == IR::expType::temp && tmp_gen[node].count(it.r.val)) continue;
            newin.push_back(it);
        }
        for (int i = exp_gen[node].size() - 1; i >= 0; i--) {
            auto it = exp_gen[node][i];
            if (it.l.kind == IR::expType::temp && tmp_gen[node].count(it.l.val)) continue;
            if (it.r.kind == IR::expType::temp && tmp_gen[node].count(it.r.val)) continue;
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
    IR::StmList* list = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
    for (; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        IR::Exp** dst = getDef(stm);
        if (!dst) continue;
        assert(stm->kind == IR::stmType::move);
        IR::Move* mvstm = static_cast<IR::Move*>(stm);
        int tempid = static_cast<IR::Temp*>(mvstm->dst)->tempid;
        assert(avail_map[node].count(G_map.at(U2Biexp(*dst))));
        if (avail_map[node].at(G_map.at(U2Biexp(*dst))) != tempid) {
            mvstm->src = new IR::Temp(avail_map[node].at(G_map.at(U2Biexp(*dst))));
            if (mvstm->src->kind == IR::expType::call) {  // phi
                ir->Aphi[node].erase(tempid);
            }
        }
    }
    for (auto dom : ir->dtree->children[node]) deletenode(dom, node);
}
void SSA::Optimizer::deletePRE() { deletenode(0, -1); }
}  // namespace SSA