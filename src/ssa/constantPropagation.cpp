#include "BuildSSA.hpp"
#include "../structure/treeIR.hpp"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>
#include "../util/utils.hpp"
#include "CDG.hpp"
using std::function;
using std::move;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace SSA {
enum class COND { undefined = 0, constant = 1, indefinite = 2 };
struct TEMP_COND {
    COND cond;
    int val;
};
TEMP_COND udf() {
    TEMP_COND x;
    x.cond = COND::undefined;
    return x;
}
TEMP_COND cst(int y) {
    TEMP_COND x;
    x.cond = COND::constant;
    x.val = y;
    return x;
}
TEMP_COND idf() {
    TEMP_COND x;
    x.cond = COND::indefinite;
    return x;
}
bool evalRel(IR::RelOp op, int lv, int rv) {
    switch (op) {
    case IR::RelOp::T_eq: return lv == rv;
    case IR::RelOp::T_ge: return lv >= rv;
    case IR::RelOp::T_gt: return lv > rv;
    case IR::RelOp::T_le: return lv <= rv;
    case IR::RelOp::T_lt: return lv < rv;
    case IR::RelOp::T_ne: return lv != rv;
    case IR::RelOp::F_eq: return decode(lv) == decode(rv);
    case IR::RelOp::F_ge: return decode(lv) >= decode(rv);
    case IR::RelOp::F_gt: return decode(lv) > decode(rv);
    case IR::RelOp::F_le: return decode(lv) <= decode(rv);
    case IR::RelOp::F_lt: return decode(lv) < decode(rv);
    case IR::RelOp::F_ne: return decode(lv) != decode(rv);
    default: break;
    }
    assert(0);
}
class DSU {
public:
    unordered_map<int, int> ff;
    int f(int x) {
        if (!ff.count(x)) {
            ff[x] = x;
            return x;
        }
        if (ff[x] == x) return x;
        return ff[x] = f(ff[x]);
    }
    void addedge(int x, int y) {
        auto a = f(x), b = f(y);
        ff[a] = b;
    }
    bool sameset(int x, int y) { return f(x) == f(y); }
};
class CP {
public:
    CP(SSAIR* t) {
        ir = t;
        nodes = ir->nodes();
        setup();
        bfsMark();
        replaceTemp();
        cleanup();
        // showmark();
        checkCopy();
        cleanCopy();
    }

private:
    void setup() {
        auto root = nodes->at(0)->mykey;
        blockCondition.insert(root);
        curBlock.push(root);
    }
    void cutEdge(int from, int to) {
        int cnt = 0;
        auto toNode = nodes->at(to);
        for (auto jt : ir->prednode[to]) {
            if (jt == from) { break; }
            cnt++;
        }
        assert(cnt != ir->prednode[to].size());
        ir->prednode[to].erase(ir->prednode[to].begin() + cnt);
        if (ir->Aphi[to].size()) {
            // jt to modify the phi func
            for (auto jt : ir->Aphi[to]) {
                auto src = (static_cast<IR::Move*>(jt.second->stm))->src;
                auto callexp = static_cast<IR::Call*>(src);
                callexp->args.erase(callexp->args.begin() + cnt);
                auto def = (static_cast<IR::Temp*>((static_cast<IR::Move*>(jt.second->stm)->dst))
                                ->tempid);
            }
        }
        ir->rmEdge(nodes->at(from), toNode);
        if (toNode->pred()->empty()) {
            while (!toNode->succ()->empty()) { cutEdge(to, *(toNode->succ()->begin())); }
        }
    }
    bool isTempIndefinite(IR::Exp* exp) {
        if (exp->kind == IR::expType::constx) return false;
        if (exp->kind == IR::expType::temp) {
            auto tmp = static_cast<IR::Temp*>(exp);
            if (tempCondition.count(tmp->tempid)) {
                return tempCondition[tmp->tempid].cond == COND::indefinite;
            } else {
                // SIDE-EFFECT
                if (isRealregister(tmp->tempid)) { return true; }
                tempCondition[tmp->tempid] = idf();
                return true;
            }
            return false;
        }
        return true;
    }
    bool isTempConst(IR::Exp* exp) {
        if (exp->kind == IR::expType::constx) return true;
        if (exp->kind == IR::expType::temp) {
            auto tmp = static_cast<IR::Temp*>(exp);
            if (tempCondition.count(tmp->tempid)) {
                return tempCondition[tmp->tempid].cond == COND::constant;
            }
            return false;
        }
        return false;
    }
    int tempEval(IR::Exp* exp) {
        switch (exp->kind) {
        case IR::expType::binop: {
            auto binopexp = static_cast<IR::Binop*>(exp);
            auto lf = tempEval(binopexp->left), rg = tempEval(binopexp->right);
            switch (binopexp->op) {
            case IR::binop::T_div: return lf / rg;
            case IR::binop::T_minus: return lf - rg;
            case IR::binop::T_mod: return lf % rg;
            case IR::binop::T_mul: return lf * rg;
            case IR::binop::T_plus: return lf + rg;
            case IR::binop::F_div: return encode(decode(lf) / decode(rg));
            case IR::binop::F_minus: return encode(decode(lf) - decode(rg));
            case IR::binop::F_mul: return encode(decode(lf) * decode(rg));
            case IR::binop::F_plus: return encode(decode(lf) + decode(rg));
            default: assert(0);
            }
        }
        case IR::expType::call: {
            auto callexp = static_cast<IR::Call*>(exp);
            return tempEval(callexp->args[0]);
        }
        case IR::expType::constx: return (static_cast<IR::Const*>(exp))->val;
        case IR::expType::eseq: assert(0);
        case IR::expType::mem: assert(0);
        case IR::expType::name: assert(0);
        case IR::expType::temp: {
            auto tmp = static_cast<IR::Temp*>(exp);
            assert(tempCondition.count(tmp->tempid)
                   && tempCondition[tmp->tempid].cond == COND::constant);
            return tempCondition[tmp->tempid].val;
        }
        default: assert(0);
        }
    }
    void doDef(IR::StmList* stml) {
        if (!stml) return;
        auto defid = static_cast<IR::Temp*>(static_cast<IR::Move*>(stml->stm)->dst)->tempid;
        auto src = static_cast<IR::Move*>(stml->stm)->src;
        int base = 0;
        if (tempCondition.count(defid)) { base = static_cast<int>(tempCondition[defid].cond); }
        if (base == 2) return;
        switch (src->kind) {
        case IR::expType::binop: {
            auto bexp = static_cast<IR::Binop*>(src);
            if (isTempConst(bexp->left) && isTempConst(bexp->right)) {
                tempCondition[defid] = cst(tempEval(bexp));
            } else {
                auto lf = isTempIndefinite(bexp->left);
                auto rf = isTempIndefinite(bexp->right);
                assert(lf || rf);
                tempCondition[defid] = idf();
            }
            break;
        }
        case IR::expType::call: {
            auto cal = static_cast<IR::Call*>(src);
            if (cal->fun[0] == '$') {
                int cur = stmlBlockmap[stml];
                int cnt = 0;
                int flag = 0, guess = 0, fall = 0;
                auto& v = (ir->prednode[cur]);
                auto len = v.size();
                for (int i = 0; i < len; i++) {
                    if (blockCondition.count(v[i])) {
                        if (cal->args[i]->kind == IR::expType::temp) {
                            auto tempid = static_cast<IR::Temp*>(cal->args[i])->tempid;
                            if (!tempCondition.count(tempid)) {
                                if (isRealregister(tempid)) {
                                    fall = 1;
                                    tempCondition[defid] = idf();
                                    break;
                                }
                                continue;
                            } else {
                                auto b = isTempIndefinite(cal->args[i]);
                                if (b) {
                                    fall = 1;
                                    tempCondition[defid] = idf();
                                    break;
                                }
                                assert(isTempConst(cal->args[i]));
                                int tval = tempEval(cal->args[i]);
                                if (flag && tval != guess) {
                                    fall = 1;
                                    tempCondition[defid] = idf();
                                    break;
                                } else {
                                    flag = 1;
                                    guess = tval;
                                }
                            }
                        } else if (cal->args[i]->kind == IR::expType::constx) {
                            if (flag) {
                                if (guess != tempEval(cal->args[i])) {
                                    fall = 1;
                                    tempCondition[defid] = idf();
                                    break;
                                }
                            } else {
                                flag = 1;
                                guess = tempEval(cal->args[i]);
                            }
                        } else
                            assert(0);
                    }
                }
                // all cannot reach or without initialization will be 0--origin guess
                if (!fall) tempCondition[defid] = cst(guess);
            } else  // normal call
                tempCondition[defid] = idf();
            break;
        }
        case IR::expType::constx: {
            auto cs = static_cast<IR::Const*>(src);
            tempCondition[defid] = cst(cs->val);
            break;
        }
        case IR::expType::eseq: assert(0);
        case IR::expType::mem:  // fallthrough
        case IR::expType::name: {
            tempCondition[defid] = idf();
            break;
        }
        case IR::expType::temp: {
            auto ot = isTempIndefinite(src);
            if (ot)
                tempCondition[defid] = idf();
            else
                tempCondition[defid] = tempCondition[static_cast<IR::Temp*>(src)->tempid];
            break;
        }
        default: assert(0);
        }
        if (static_cast<int>(tempCondition[defid].cond) > base) curTemp.push(defid);
    }
    void doJump(IR::StmList* stml) {
        auto stm = stml->stm;
        auto curb = stmlBlockmap[stml];
        if (stm->kind == IR::stmType::jump) {
            auto nx = *nodes->at(curb)->succ()->begin();
            if (!blockCondition.count(nx)) {
                blockCondition.insert(nx);
                curBlock.push(nx);
            }
        } else {
            auto cjmp = static_cast<IR::Cjump*>(stm);
            if (isTempConst(cjmp->left) && isTempConst(cjmp->right)) {
                auto lv = tempEval(cjmp->left), rv = tempEval(cjmp->right);
                auto b = evalRel(cjmp->op, lv, rv);
                auto tg = cjmp->falseLabel;
                if (b) tg = cjmp->trueLabel;
                for (auto it : (*(nodes->at(curb))->succ())) {
                    if (getNodeLabel(ir->mynodes[it]) == tg) {
                        auto nx = it;
                        if (!blockCondition.count(nx)) {
                            blockCondition.insert(nx);
                            curBlock.push(nx);
                        }
                    }
                }
                return;
            }
            if ((cjmp->left->kind == IR::expType::temp && isTempIndefinite(cjmp->left))
                || (cjmp->right->kind == IR::expType::temp && isTempIndefinite(cjmp->right))) {
                for (auto it : (*(nodes->at(curb))->succ())) {
                    auto nx = it;
                    if (!blockCondition.count(nx)) {
                        blockCondition.insert(nx);
                        curBlock.push(nx);
                    }
                }
                return;
            }
            assert(0);
        }
    }
    void bfsMark() {
        while (!curBlock.empty() || !curTemp.empty()) {
            while (!curBlock.empty()) {
                auto cur = curBlock.front();
                curBlock.pop();
                auto stml = static_cast<IR::StmList*>(nodes->at(cur)->info);
                while (stml) {
                    auto stm = stml->stm;
                    // set up def-stm mapping
                    auto df = getDef(stm);
                    if (df) {
                        stmlBlockmap[stml] = cur;
                        tempDef[static_cast<IR::Temp*>(*df)->tempid] = stml;
                        doDef(stml);
                    }
                    auto us = getUses(stm);
                    for (auto it : us) {
                        // maybe same stm pushed into info
                        tempUse[static_cast<IR::Temp*>(*it)->tempid].push_back(stml);
                    }
                    if (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump) {
                        stmlBlockmap[stml] = cur;
                        doJump(stml);
                    }
                    stml = stml->tail;
                }
                for (auto nx : *(ir->nodes()->at(cur)->succ())) {
                    if (blockCondition.count(nx)) {
                        for (auto st : ir->Aphi[nx]) { doDef(st.second); }
                    }
                }
            }
            while (!curTemp.empty()) {
                auto cur = curTemp.front();
                curTemp.pop();
                auto def = (static_cast<IR::Move*>(tempDef[cur]->stm))->src;
                for (auto use : tempUse[cur]) {
                    auto df = getDef(use->stm);
                    if (df) doDef(use);
                    if (use->stm->kind == IR::stmType::cjump
                        || use->stm->kind == IR::stmType::jump)
                        doJump(use);
                }
            }
        }
    }
    void replaceTemp() {
        auto nodes = ir->nodes();
        for (auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            if (!blockCondition.count(it->mykey)) {
                while (!it->pred()->empty()) { ir->rmEdge(ir->mynodes[*it->pred()->begin()], it); }
                while (!it->succ()->empty()) { cutEdge(it->mykey, *(it->succ()->begin())); }
                stml->tail = 0;
                continue;
            }
            auto head = stml, tail = stml->tail;
            while (tail) {
                auto def = getDef(tail->stm);
                bool flag = false;
                int tempid = -1;
                if (def) {
                    tempid = static_cast<IR::Temp*>(*def)->tempid;
                    if (tempCondition.count(tempid)) {
                        if (tempCondition[tempid].cond == COND::constant) { flag = true; }
                    }
                }
                if (flag) {
                    auto newtail = tail->tail;
                    tail->tail = 0;
                    if (ir->Aphi[it->mykey].count(tempid) && ir->Aphi[it->mykey][tempid] == tail) {
                        ir->Aphi[it->mykey].erase(tempid);
                    }
                    delete tail;
                    tail = newtail;
                    head->tail = newtail;
                } else {
                    auto uses = getUses(tail->stm);
                    for (auto it : uses) {
                        auto tempid = static_cast<IR::Temp*>(*it)->tempid;
                        if (tempCondition.count(tempid)) {
                            if (tempCondition[tempid].cond == COND::constant) {
                                delete (*it);
                                *it = new IR::Const(tempCondition[tempid].val);
                            }
                        }
                    }
                    head = head->tail;
                    tail = tail->tail;
                }
            }
            if (head && head->stm->kind == IR::stmType::cjump) {
                auto cjmpstm = static_cast<IR::Cjump*>(head->stm);
                bool ht = false, hf = false;
                for (auto jt : *(it->succ())) {
                    if (!blockCondition.count(jt)) continue;
                    if (getNodeLabel(ir->mynodes[jt]) == cjmpstm->trueLabel) {
                        ht = true;
                    } else if (getNodeLabel(ir->mynodes[jt]) == cjmpstm->falseLabel) {
                        hf = true;
                    }
                }
                assert(ht || hf);
                if (!ht) {
                    head->stm = new IR::Jump(cjmpstm->falseLabel);
                    delete cjmpstm;
                }
                if (!hf) {
                    head->stm = new IR::Jump(cjmpstm->trueLabel);
                    delete cjmpstm;
                }
            }
        }
    }

    void cleanup() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            if (it->inDegree() == 1) {
                for (auto jt : ir->Aphi[it->mykey]) {
                    auto mv = static_cast<IR::Move*>(jt.second->stm);
                    auto cl = static_cast<IR::Call*>(mv->src);
                    mv->src = cl->args[0]->quad();
                    delete cl;
                }
                ir->Aphi[it->mykey].clear();
            }
        }
    };
    void checkCopy() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                if (stm->kind == IR::stmType::move) {
                    auto mv = static_cast<IR::Move*>(stm);
                    if (mv->dst->kind == IR::expType::temp && mv->src->kind == IR::expType::temp) {
                        auto from = static_cast<IR::Temp*>(mv->src)->tempid,
                             to = static_cast<IR::Temp*>(mv->dst)->tempid;
                        if (!isRealregister(from) && !isRealregister(to)) {
                            // replace the def temp by the use temp
                            dsu.addedge(to, from);
                        }
                    }
                }
                stml = stml->tail;
            }
        }
    }
    void cleanCopy() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                bool flag = false;
                if (stm->kind == IR::stmType::move) {
                    auto mv = static_cast<IR::Move*>(stm);
                    if (mv->dst->kind == IR::expType::temp && mv->src->kind == IR::expType::temp) {
                        auto from = static_cast<IR::Temp*>(mv->src)->tempid,
                             to = static_cast<IR::Temp*>(mv->dst)->tempid;
                        if (!isRealregister(from) && !isRealregister(to)) {
                            if (dsu.sameset(from, to)) {
                                delete stm;
                                stml->stm = nopStm();
                                flag = true;
                            }
                        }
                    }
                }
                if (!flag) {
                    auto uses = getUses(stm);
                    for (auto it : uses) {
                        auto tmp = static_cast<IR::Temp*>(*it);
                        if (dsu.f(tmp->tempid) != tmp->tempid) {
                            tmp->tempid = dsu.f(tmp->tempid);
                        }
                    }
                    auto df = getDef(stm);
                    if (df) {
                        auto def = static_cast<IR::Temp*>(*df);
                        if (dsu.f(def->tempid) != def->tempid) {
                            def->tempid = dsu.f(def->tempid);
                        }
                    }
                }
                stml = stml->tail;
            }
        }
    }
    void showtemptable() {
        for (auto it : tempCondition) {
            std::cerr << "t" << it.first << ": type" << static_cast<int>(it.second.cond)
                      << "   val:" << it.second.val << std::endl;
        }
    }

private:
    SSAIR* ir;
    unordered_map<Temp_Temp, TEMP_COND> tempCondition;
    unordered_map<IR::StmList*, int> stmlBlockmap;
    unordered_map<Temp_Temp, IR::StmList*> tempDef;
    unordered_map<Temp_Temp, vector<IR::StmList*>> tempUse;
    unordered_set<int> blockCondition;
    queue<int> curBlock;
    queue<Temp_Temp> curTemp;
    queue<Temp_Temp> copyTemp;
    DSU dsu;
    vector<GRAPH::Node*>* nodes;
};
void SSA::Optimizer::constantPropagation() {
    auto tt = CP(ir);
    return;
}
}  // namespace SSA