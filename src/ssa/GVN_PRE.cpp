#include "BuildSSA.hpp"
#include "../util/utils.hpp"
namespace SSA {
void Optimizer::PRE() {
    ir->cut_edge();
    ir->dtree = new DTREE::Dtree(ir, 0);
    // A. build table
    buildTable();
    // B. insert phi, computation
    insertPRE();  // has bug
    // C. delete
    deletePRE();
}
void Optimizer::buildTable() {
    // use dtree
    avail_out = std::vector<std::unordered_set<int>>(ir->nodecount, std::unordered_set<int>());
    avail_map
        = std::vector<std::unordered_map<int, int>>(ir->nodecount, std::unordered_map<int, int>());
    G_map.clear();
    vG_map.clear();
    exp_gen = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    exp_map = std::vector<std::unordered_map<int, Biexp>>(ir->nodecount,
                                                          std::unordered_map<int, Biexp>());
    tmp_gen = std::vector<std::unordered_set<int>>(ir->nodecount, std::unordered_set<int>());
    buildAvail();
    anticIn = std::vector<std::list<Biexp>>(ir->nodecount, std::list<Biexp>());
    anticIn_map = std::vector<std::unordered_map<int, SSA::Optimizer::Biexp>>(
        ir->nodecount, std::unordered_map<int, SSA::Optimizer::Biexp>());
    buildAntic();
}
void Optimizer::buildAvail() { buildAvail(0, -1); }
Optimizer::Biexp Optimizer::U2Biexp(IR::Exp* e) {
    IR::Const const_temp = IR::Const(0);
    return Biexp(IR::binop::T_plus, e, &const_temp);
}
int Optimizer::findGV(const Biexp& biexp) {
    int val = Temp_newtemp();
    if (!G_map.count(biexp)) {
        G_map[biexp] = val;
        IR::Temp temp = IR::Temp(val);
        G_map.insert({U2Biexp(&temp), val});
    }
    return G_map.at(biexp);
}
void Optimizer::buildAvail(int node, int fa) {
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
        bool isphi = false;
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
                if (!exp_map[node].count(lval)) {
                    exp_map[node][lval] = U2Biexp(bistm->left);
                    exp_gen[node].push_back(U2Biexp(bistm->left));
                }
                if (!exp_map[node].count(rval)) {
                    exp_map[node][rval] = U2Biexp(bistm->right);
                    exp_gen[node].push_back(U2Biexp(bistm->right));
                }
                if (!exp_map[node].count(dstval)) {
                    exp_map[node][dstval] = biexp;
                    exp_gen[node].push_back(biexp);
                }
            } break;
            case IR::expType::call: {  // must be a phi
                isphi = true;
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
        } else {  // temp <- *
            isphi = false;
            assert(mvstm->src->kind != IR::expType::constx);
            // tmp_gen[node].insert(static_cast<IR::Temp*>(*dst)->tempid);
            dstval = findGV(U2Biexp(*dst));
        }
        if (!avail_map[node].count(dstval)) {
            avail_map[node][dstval] = tempid;
            avail_out[node].insert(tempid);
        }
        if (!isphi) { tmp_gen[node].insert(tempid); }
    }
    for (auto dom : ir->dtree->children[node]) buildAvail(dom, node);
}
bool Optimizer::buildAntic(int node) {
    std::list<Biexp> newin, newout;
    std::unordered_map<int, Biexp> newin_map, newout_map;
    // origIn
    newin = exp_gen[node];
    newin_map = exp_map[node];
    // Anticout
    if (ir->nodes()->at(node)->outDegree() == 1) {
        int cnt = 0;
        int succnode = *ir->nodes()->at(node)->succs.begin();
        for (auto pred : ir->prednode[succnode]) {
            if (node == pred) break;
            cnt++;
        }
        std::unordered_map<int, int> emval;
        for (auto it : anticIn[succnode]) {
            int oldval = findGV(it);
            if (it.isTemp() && ir->Aphi[succnode].count(it.l.val)) {
                Biexp biexp = U2Biexp(
                    static_cast<IR::Call*>(
                        static_cast<IR::Move*>(ir->Aphi[succnode].at(it.l.val)->stm)->src)
                        ->args.at(cnt));
                int newval = findGV(biexp);
                if (!newout_map.count(newval)) {
                    newout_map.insert({newval, biexp});
                    newout.push_back(biexp);
                }
                if (oldval != newval) emval.insert({oldval, newval});
            } else if (it.isExp()) {
                Biexp nw = it;
                if (emval.count(it.l.val)) { nw.l.val = emval.at(it.l.val); }
                if (emval.count(it.r.val)) { nw.r.val = emval.at(it.r.val); }
                int newval = findGV(nw);
                if (!newout_map.count(newval)) {
                    newout_map.insert({newval, nw});
                    newout.push_back(nw);
                }
                if (oldval != newval) emval.insert({oldval, newval});
            } else {
                if (!newout_map.count(oldval)) {
                    newout_map.insert({oldval, it});
                    newout.push_back(it);
                }
            }
        }
    } else if (ir->nodes()->at(node)->outDegree() == 2) {  //==2
        int cnt = 0;
        int succnode[2];
        for (auto succ : *ir->nodes()->at(node)->succ()) {
            succnode[cnt] = succ;
            cnt++;
        }
        for (auto it : anticIn[succnode[0]]) {
            int val = G_map.at(it);
            if (anticIn_map[node].count(val)) {
                newout_map.insert({val, it});
                newout.push_back(it);
            }
        }

    } else
        assert(node == ir->exitnum);
    // newout-tmp_gen
    for (std::list<Biexp>::iterator it = newout.begin(); it != newout.end();) {
        if (it->isTemp() && tmp_gen[node].count(it->l.val)) {
            newout_map.erase(G_map.at(*it));
            newout.erase(it++);
        } else
            ++it;
    }
    // canon_e
    for (auto it : newout) {
        int val = G_map.at(it);
        if (!newin_map.count(val)) {
            newin_map.insert({val, it});
            newin.push_back(it);
        }
    }
    // clean
    std::unordered_set<int> existval;
    for (std::list<Biexp>::iterator it = newin.begin(); it != newin.end();) {
        int val = G_map.at(*it);
        if (it->r.kind == IR::expType::constx) {  // v:temp(const,name,temp)
            existval.insert(val);
            ++it;
        } else {  // v3:v1+v2
            int lval = it->l.val;
            int rval = it->r.val;
            if (!existval.count(lval) || !existval.count(rval)) {  // should be erased
                newin_map.erase(G_map.at(*it));
                newin.erase(it++);
            } else {
                existval.insert(val);
                ++it;
            }
        }
    }
    // is change
    // std::cerr << "!!!" << newin.begin()->l.val << "\n";
    // std::cerr << "---" << node << " " << anticIn[node].size() << " " << newin.size() << "\n";
    assert(newin.size() == newin_map.size());
    assert(newin.size() >= anticIn[node].size());
    bool change = newin != anticIn[node];
    anticIn[node] = newin;
    anticIn_map[node] = newin_map;
    // std::cerr << "^^^" << node << " " << anticIn[node].size() << " " << newin.size() << " "
    //           << anticIn_map[node].size() << "\n";
    assert(anticIn[node].size() == anticIn_map[node].size());
    for (auto dom : cdg->dtree->children[node]) {
        if (dom >= ir->nodecount) continue;
        change = change | buildAntic(dom);
    }
    return change;
}
/**
 * exp has 3 type: 1.(temp,constx),means a temp;2.(const/name,constx),means a
 * const;3.(temp,temp),means v1+v2
 *
 */
void Optimizer::buildAntic() {
    cdg = new CDG::CDgraph(ir);
    for (auto node : *ir->nodes()) {  // predo exp_gen
        for (std::list<Biexp>::iterator it = exp_gen[node->mykey].begin();
             it != exp_gen[node->mykey].end();) {
            if (it->l.kind == IR::expType::temp && it->r.kind == IR::expType::constx
                && tmp_gen[node->mykey].count(it->l.val)) {
                exp_map[node->mykey].erase(G_map.at(*it));
                exp_gen[node->mykey].erase(it++);
            } else
                ++it;
        }
        // std::cerr << "@@@" << exp_gen[node->mykey].size() << "\n";
    }
    bool change = true;
    int ccnt = 0;
    while (change) {
        ccnt++;
        change = buildAntic(cdg->exitnum);
    }
    // std::cerr << "CUR TIMES:" << ccnt << "\n";
}
void SSA::Optimizer::insertPRE(int node) {
    if (ir->nodes()->at(node)->inDegree() > 1) {
        std::vector<std::unordered_map<int, int>> phi_trans(ir->nodes()->at(node)->inDegree(),
                                                            std::unordered_map<int, int>());
        for (auto it : anticIn[node]) {
            std::vector<int> valv;
            std::vector<Biexp> expv;
            bool allhas = true, onehas = false;
            int cnt = -1;
            int oldval = findGV(it);
            for (auto pred : ir->prednode[node]) {
                cnt++;
                int newval;
                Biexp biexp;
                if (it.isTemp() && ir->Aphi[node].count(it.l.val)) {
                    biexp = U2Biexp(
                        static_cast<IR::Call*>(
                            static_cast<IR::Move*>(ir->Aphi[node].at(it.l.val)->stm)->src)
                            ->args.at(cnt));
                    newval = G_map.at(biexp);
                    if (oldval != newval) phi_trans[cnt].insert({oldval, newval});
                    assert(vG_map.count(newval) || avail_map[pred].count(newval));
                } else if (it.isExp()) {
                    biexp = it;
                    int lval = it.l.val;
                    int rval = it.r.val;
                    if (phi_trans[cnt].count(lval)) biexp.l.val = phi_trans[cnt].at(lval);
                    if (phi_trans[cnt].count(rval)) biexp.r.val = phi_trans[cnt].at(rval);
                    newval = G_map.at(biexp);
                    if (oldval != newval) phi_trans[cnt].insert({oldval, newval});

                    if (avail_map[pred].count(newval)) {
                        onehas = true;
                    } else {
                        allhas = false;
                    }
                    valv.push_back(newval);
                    expv.push_back(biexp);
                } else {
                    if (it.isTemp() && it.l.val > 15) {
                        assert(vG_map.count(oldval) || avail_map[pred].count(oldval));
                    }
                }
            }
            if (onehas && !allhas) {  // PRE
                cnt = -1;
                std::vector<IR::Exp*> param;
                for (auto pred : ir->prednode[node]) {
                    cnt++;
                    Temp_Temp dsttemp;
                    assert(valv[cnt] != 1243);
                    if (avail_map[pred].count(valv[cnt])) {  // already has
                        dsttemp = avail_map[pred].at(valv[cnt]);
                    } else {
                        dsttemp = Temp_newtemp();
                        IR::Exp *lf, *rt = nullptr;
                        if (avail_map[pred].count(expv[cnt].l.val))
                            lf = new IR::Temp(avail_map[pred].at(expv[cnt].l.val));
                        else {
                            Biexp bp = vG_map.at(expv[cnt].l.val);
                            assert(bp.l.kind == IR::expType::constx
                                   || bp.l.kind == IR::expType::name
                                   || (bp.l.kind == IR::expType::temp && bp.l.val < 15));
                            lf = bp.l.toExp();
                        }
                        if (avail_map[pred].count(expv[cnt].r.val))
                            rt = new IR::Temp(avail_map[pred].at(expv[cnt].r.val));
                        else {
                            Biexp bp = vG_map.at(expv[cnt].r.val);
                            assert(bp.r.kind == IR::expType::constx
                                   || bp.r.kind == IR::expType::name
                                   || (bp.r.kind == IR::expType::temp && bp.r.val < 15));
                            rt = bp.r.toExp();
                        }
                        IR::Temp* dtmp = new IR::Temp(dsttemp);
                        ir->blockjump[pred]->tail
                            = new IR::StmList(ir->blockjump[pred]->stm, nullptr);
                        ir->blockjump[pred]->stm
                            = new IR::Move(dtmp, new IR::Binop(expv[cnt].op, lf, rt));
                        ir->blockjump[pred] = ir->blockjump[pred]->tail;
                        G_map.insert({U2Biexp(dtmp), valv[cnt]});
                        avail_map[pred].insert({valv[cnt], dsttemp});
                        avail_out[pred].insert(dsttemp);
                    }
                    param.push_back(new IR::Temp(dsttemp));
                }
                int phi_temp = Temp_newtemp();
                IR::Temp* dtmp = new IR::Temp(phi_temp);
                G_map.insert({U2Biexp(dtmp), oldval});
                avail_map[node].insert({oldval, phi_temp});
                avail_out[node].insert(phi_temp);
                IR::Move* phifunc = new IR::Move(dtmp, new IR::Call("$", param));
                IR::StmList* stmlist = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
                stmlist->tail = new IR::StmList(phifunc, stmlist->tail);
                ir->Aphi[node].insert(std::make_pair(phi_temp, stmlist->tail));
            }
        }
    }
    for (auto dom : ir->dtree->children[node]) insertPRE(dom);
}
void SSA::Optimizer::insertPRE() {
    for (auto it : G_map) {
        if (it.first.l.kind == IR::expType::constx || it.first.l.kind == IR::expType::name) {
            vG_map[it.second] = it.first;
            // std::cerr << "@@@" << it.second << "\n";
        }
    }
    vG_map.insert({findGV(U2Biexp(new IR::Temp(11))), U2Biexp(new IR::Temp(11))});
    vG_map.insert({findGV(U2Biexp(new IR::Temp(13))), U2Biexp(new IR::Temp(13))});
    insertPRE(0);
}
void SSA::Optimizer::deletenode(int node, int fa) {
    IR::StmList* list = (IR::StmList*)ir->nodes()->at(node)->nodeInfo();
    for (; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        IR::Exp** dst = getDef(stm);
        if (!dst) continue;
        assert(stm->kind == IR::stmType::move);
        IR::Move* mvstm = static_cast<IR::Move*>(stm);
        assert(mvstm->dst->kind == IR::expType::temp);
        int tempid = static_cast<IR::Temp*>(mvstm->dst)->tempid;
        assert(avail_map[node].count(G_map.at(U2Biexp(*dst))));
        if (avail_map[node].at(G_map.at(U2Biexp(*dst))) != tempid) {
            if (mvstm->src->kind == IR::expType::call) {  // phi
                assert(static_cast<IR::Call*>(mvstm->src)->fun[0] == '$');
                ir->Aphi[node].erase(tempid);
            }
            mvstm->src = new IR::Temp(avail_map[node].at(G_map.at(U2Biexp(*dst))));
        }
    }
    for (auto dom : ir->dtree->children[node]) deletenode(dom, node);
}
void SSA::Optimizer::deletePRE() { deletenode(0, -1); }
}  // namespace SSA