#include "optimizer.hpp"
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

namespace SSAOPT {
bool isNecessaryStm(IR::Stm* stm) {
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind == IR::expType::mem) return true;
        // fixme: all funcs have side-effect?
        if (movestm->src->kind == IR::expType::call) return true;
        if (movestm->dst->kind == IR::expType::temp
            && isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid))
            return true;
        return false;
    }
    case IR::stmType::exp: {
        auto expstm = static_cast<IR::ExpStm*>(stm);
        // fixme: all funcs have side-effect?
        if (expstm->exp->kind == IR::expType::call) return true;
        return false;
    }
    case IR::stmType::cjump: return false;
    case IR::stmType::label: return false;
    case IR::stmType::seq: assert(0);
    case IR::stmType::jump: return false;
    default: assert(0);
    }
    return false;
}
Temp_Label getNodeLabel(GRAPH::Node* node) {
    return static_cast<IR::Label*>(((IR::StmList*)(node->info))->stm)->label;
}
SSA::SSAIR* SSAOPT::Optimizer::deadCodeElimilation(SSA::SSAIR* ir) {
    unordered_map<Temp_Temp, IR::Stm*> tempDef;
    unordered_map<IR::Stm*, int> stmBlockmap;
    unordered_set<IR::Stm*> ActivatedStm;
    unordered_set<int> ActivatedBlock;
    unordered_map<int, IR::StmList*> blockJumpStm;
    queue<IR::Stm*> Curstm;
    auto nodesz = ir->nodes()->size();
    vector<GRAPH::NodeList> newpred(nodesz), newsucc(nodesz);

    CDG::CDgraph gp(ir);

    auto setup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                // set up stm-block mapping
                stmBlockmap[stm] = it->mykey;

                // set up def-stm mapping
                auto df = getDef(stm);
                if (df) tempDef[static_cast<IR::Temp*>(*df)->tempid] = stm;
                // set up seed stm
                bool necessary = isNecessaryStm(stm);
                if (necessary) {
                    ActivatedStm.insert(stm);
                    Curstm.push(stm);
                    // set up activated block
                    ActivatedBlock.insert(it->mykey);
                }
                // auto us = getUses(stm);
                // for(auto it:us){
                //     //maybe same stm pushed into info
                //     tempDef[*it].use.push_back(stm);
                // }

                // set up jump
                if (!stml->tail) {
                    if (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump) {
                        blockJumpStm[it->mykey] = stml;
                    }
                }
                stml = stml->tail;
            }
        }
    };
    auto bfsMark = [&]() {
        while (!Curstm.empty()) {
            auto stm = Curstm.front();
            Curstm.pop();
            // upload to the def of uses
            auto uses = getUses(stm);
            for (auto it : uses) {
                auto def = tempDef[static_cast<IR::Temp*>(*it)->tempid];
                if (!ActivatedStm.count(def)) {
                    // set up activated block
                    ActivatedBlock.insert(stmBlockmap[def]);
                    ActivatedStm.insert(def);
                    Curstm.push(def);
                }
            }
            // upload to branch stm
            auto block = ir->nodes()->at(stmBlockmap[stm]);
            auto labelstm = ((IR::StmList*)(block->info))->stm;
            if (!ActivatedStm.count(labelstm)) {
                ActivatedStm.insert(labelstm);
                for (auto it : gp.CDnode[block->mykey]) {
                    auto jmp = blockJumpStm[it]->stm;
                    if (jmp->kind == IR::stmType::cjump) {
                        if (!ActivatedStm.count(jmp)) {
                            ActivatedBlock.insert(it);
                            ActivatedStm.insert(jmp);
                            Curstm.push(jmp);
                        }
                    }
                }
            }
        }
    };
    auto FindNextAcitveNode = [&](GRAPH::Node* node) {
        if (ActivatedBlock.count(node->mykey)) return node;
        unordered_set<GRAPH::Node*> vis;
        vis.insert(node);
        GRAPH::Node* ans = 0;
        function<void(GRAPH::Node*)> dfs = [&](GRAPH::Node* cur) {
            if (ans) return;
            if (ActivatedBlock.count(cur->mykey)) {
                ans = cur;
                return;
            }
            GRAPH::NodeList* succs = cur->succ();
            for (GRAPH::NodeList::const_iterator it = succs->begin(); it != succs->end(); ++it) {
                if (!vis.count(*it)) {
                    vis.insert(*it);
                    dfs(*it);
                }
            }
        };
        dfs(node);
        return ans;
    };
    auto elimilation = [&]() {
        queue<GRAPH::Node*> CurNode;
        auto nodes = ir->nodes();
        CurNode.push((nodes->at(0)));  // push the first node
        while (!CurNode.empty()) {
            auto cur = CurNode.front();
            CurNode.pop();
            auto stml = (IR::StmList*)(cur->info);
            auto head = stml, tail = stml->tail;
            while (tail) {
                if (!tail->tail) {  // the last stm
                    if (tail->stm->kind == IR::stmType::cjump) {
                        auto cjumpstm = static_cast<IR::Cjump*>(tail->stm);
                        if (ActivatedStm.count(tail->stm)) {
                            GRAPH::Node *trueNode = 0, *falseNode = 0;
                            for (auto& it : (*cur->succ())) {
                                auto nodelabel = getNodeLabel(it);
                                if (nodelabel == cjumpstm->trueLabel) {
                                    trueNode = FindNextAcitveNode(it);
                                    cjumpstm->trueLabel = getNodeLabel(trueNode);
                                } else if (nodelabel == cjumpstm->falseLabel) {
                                    falseNode = FindNextAcitveNode(it);
                                    cjumpstm->falseLabel = getNodeLabel(falseNode);
                                }
                            }
                            newsucc[cur->mykey].insert(trueNode);
                            newsucc[cur->mykey].insert(falseNode);
                            newpred[trueNode->mykey].insert(cur);
                            newpred[falseNode->mykey].insert(cur);
                            CurNode.push(trueNode);
                            CurNode.push(falseNode);
                        } else {
                            GRAPH::Node* nextNode = 0;
                            for (auto& it : (*cur->succ())) {
                                nextNode = FindNextAcitveNode(it);
                                if (nextNode) break;
                            }
                            newsucc[cur->mykey].insert(nextNode);
                            newpred[nextNode->mykey].insert(cur);
                            CurNode.push(nextNode);
                            // fixme :delete ,memory leak
                            blockJumpStm[cur->mykey]->stm = new IR::Jump(getNodeLabel(nextNode));
                        }
                    } else if (tail->stm->kind == IR::stmType::jump) {
                        GRAPH::Node* nextNode = 0;
                        for (auto& it : (*cur->succ())) {
                            nextNode = FindNextAcitveNode(it);
                            if (nextNode) break;
                        }
                        newsucc[cur->mykey].insert(nextNode);
                        newpred[nextNode->mykey].insert(cur);
                        CurNode.push(nextNode);
                        // fixme :delete ,memory leak
                        blockJumpStm[cur->mykey]->stm = new IR::Jump(getNodeLabel(nextNode));
                    } else {
                        // return no jump TODO
                    }
                    break;
                }
                if (!ActivatedStm.count(tail->stm)) {
                    // auto todelete=tail;
                    auto newtail = tail->tail;
                    tail->tail = 0;
                    tail = newtail;
                    head->tail = newtail;
                    // delete todelete
                } else {
                    head = head->tail;
                    tail = tail->tail;
                }
            }
        }
        for (int i = 0; i < nodesz; i++) {
            ir->nodes()->at(i)->preds = move(newpred[i]);
            ir->nodes()->at(i)->succs = move(newsucc[i]);
        }
    };
    setup();
    bfsMark();
    elimilation();
}
// enum class COND { undefined, constant, indefinite };
// struct TEMP_COND {
//     COND cond;
//     int val;
// };
// TEMP_COND udf() {
//     TEMP_COND x;
//     x.cond = COND::undefined;
//     return x;
// }
// TEMP_COND cst(int y) {
//     TEMP_COND x;
//     x.cond = COND::constant;
//     x.val = y;
//     return x;
// }
// TEMP_COND idf() {
//     TEMP_COND x;
//     x.cond = COND::indefinite;
//     return x;
// }
int evalExp(IR::Exp* exp) {
    // MUST MAKE SURE EXP IS CONSTEXP
    switch (exp->kind) {
    case IR::expType::binop: {
        auto binopexp = static_cast<IR::Binop*>(exp);
        auto lf = evalExp(binopexp->left), rg = evalExp(binopexp->right);
        switch (binopexp->op) {
        case IR::binop::T_div: return lf / rg;
        case IR::binop::T_minus: return lf - rg;
        case IR::binop::T_mod: return lf % rg;
        case IR::binop::T_mul: return lf * rg;
        case IR::binop::T_plus: return lf + rg;
        default: assert(0);
        }
    }
    case IR::expType::call: {
        auto callexp = static_cast<IR::Call*>(exp);
        return evalExp(callexp->args[0]);
    }
    case IR::expType::constx: return (static_cast<IR::Const*>(exp))->val;
    case IR::expType::eseq: assert(0);
    case IR::expType::mem: assert(0);
    case IR::expType::name: assert(0);
    case IR::expType::temp: assert(0);
    default: assert(0);
    }
}
bool isConstExp(IR::Exp* exp) {
    switch (exp->kind) {
    case IR::expType::binop: {
        auto binopexp = static_cast<IR::Binop*>(exp);
        return isConstExp(binopexp->left) && isConstExp(binopexp->right);
    }
    case IR::expType::call: {
        auto callexp = static_cast<IR::Call*>(exp);
        if (callexp->fun[0] == '$') {
            int len = callexp->args.size();
            if (!isConstExp(callexp->args[0])) return false;
            int guess = evalExp(callexp->args[0]);
            for (int i = 1; i < len; i++) {
                if (!isConstExp(callexp->args[i])) return false;
                if (evalExp(callexp->args[i]) != guess) return false;
            }
            return true;
        }
        return false;
    }
    case IR::expType::constx: return true;
    case IR::expType::eseq: assert(0);
    case IR::expType::mem: return false;
    case IR::expType::name: return false;
    case IR::expType::temp: return false;
    default: assert(0);
    }
    return false;
}
bool isConstDef(IR::Stm* stm) {
    if (stm->kind == IR::stmType::move) {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind != IR::expType::temp) return false;
        if (isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid)) return false;
        auto src = movestm->src;
        return isConstExp(src);
    }
    return false;
}
void constReplace(Temp_Temp tempid, IR::Stm* stm, int val) {
    switch (stm->kind) {
    case IR::stmType::cjump: {
        auto cjumpexp = static_cast<IR::Cjump*>(stm);
        auto lexp = cjumpexp->left;
        if (lexp->kind == IR::expType::temp && (static_cast<IR::Temp*>(lexp))->tempid == tempid) {
            // fixme free the temp!!
            cjumpexp->left = new IR::Const(val);
        }
        auto rexp = cjumpexp->right;
        if (rexp->kind == IR::expType::temp && (static_cast<IR::Temp*>(rexp))->tempid == tempid) {
            // fixme free the temp!!
            cjumpexp->right = new IR::Const(val);
        }
        break;
    }
    case IR::stmType::exp: {
        auto expstm = static_cast<IR::ExpStm*>(stm);
        assert(expstm->exp->kind == IR::expType::call);
        auto callexp = static_cast<IR::Call*>(expstm->exp);
        int len = callexp->args.size();
        for (int i = 0; i < len; i++) {
            auto exp = callexp->args[i];
            if (exp->kind == IR::expType::temp
                && (static_cast<IR::Temp*>(exp))->tempid == tempid) {
                // fixme free the temp!!
                callexp->args[i] = new IR::Const(val);
            }
        }
        break;
    }
    case IR::stmType::jump: assert(0);
    case IR::stmType::label: assert(0);
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        switch (movestm->src->kind) {
        case IR::expType::binop: {
            auto binopexp = static_cast<IR::Binop*>(movestm->src);
            auto lexp = binopexp->left;
            if (lexp->kind == IR::expType::temp
                && (static_cast<IR::Temp*>(lexp))->tempid == tempid) {
                // fixme free the temp!!
                binopexp->left = new IR::Const(val);
            }
            auto rexp = binopexp->right;
            if (rexp->kind == IR::expType::temp
                && (static_cast<IR::Temp*>(rexp))->tempid == tempid) {
                // fixme free the temp!!
                binopexp->right = new IR::Const(val);
            }
            break;
        }
        case IR::expType::call: {
            auto callexp = static_cast<IR::Call*>(movestm->src);
            int len = callexp->args.size();
            for (int i = 0; i < len; i++) {
                auto exp = callexp->args[i];
                if (exp->kind == IR::expType::temp
                    && (static_cast<IR::Temp*>(exp))->tempid == tempid) {
                    // fixme free the temp!!
                    callexp->args[i] = new IR::Const(val);
                }
            }
            break;
        }
        case IR::expType::constx: assert(0);
        case IR::expType::eseq: assert(0);
        case IR::expType::mem: {
            auto memexp = static_cast<IR::Mem*>(movestm->src);
            auto exp = memexp->mem;
            if (exp->kind == IR::expType::temp
                && (static_cast<IR::Temp*>(exp))->tempid == tempid) {
                // fixme free the temp!!
                memexp->mem = new IR::Const(val);
            }
            break;
        }
        case IR::expType::name: assert(0);
        case IR::expType::temp: {
            if ((static_cast<IR::Temp*>(movestm->src))->tempid == tempid) {
                // fixme free the temp!!
                movestm->src = new IR::Const(val);
            }
            break;
        }
        default: assert(0);
        }
        break;
    }
    case IR::stmType::seq: assert(0);
    default: assert(0);
    }
}
bool evalRel(IR::RelOp op, int lv, int rv) {
    switch (op) {
    case IR::RelOp::T_eq: return lv == rv;
    case IR::RelOp::T_ge: return lv >= rv;
    case IR::RelOp::T_gt: return lv > rv;
    case IR::RelOp::T_le: return lv <= rv;
    case IR::RelOp::T_lt: return lv < rv;
    case IR::RelOp::T_ne: return lv != rv;
    default: break;
    }
    assert(0);
}

SSA::SSAIR* SSAOPT::Optimizer::constantPropagation(SSA::SSAIR* ir) {
    // unordered_map<Temp_Temp, TEMP_COND> tempCondition;
    unordered_map<IR::StmList*, int> stmlBlockmap;
    unordered_map<Temp_Temp, IR::StmList*> tempDef;
    unordered_map<Temp_Temp, vector<IR::StmList*>> tempUse;
    unordered_map<int, int> blockCondition;
    // queue<int> curBlock;
    queue<Temp_Temp> curTemp;

    auto nodes = ir->nodes();
    auto setup = [&]() {
        // auto root = nodes->at(0)->mykey;
        // blockCondition.insert({root, 1});
        // curBlock.push(root);
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;

                // set up def-stm mapping
                auto df = getDef(stm);
                if (df) {
                    tempDef[static_cast<IR::Temp*>(*df)->tempid] = stml;
                    // set up seed temp
                    bool necessary = isConstDef(stm);
                    if (necessary) { curTemp.push(static_cast<IR::Temp*>(*df)->tempid); }
                }
                auto us = getUses(stm);
                for (auto it : us) {
                    // maybe same stm pushed into info
                    tempUse[static_cast<IR::Temp*>(*it)->tempid].push_back(stml);
                }
                if (stm->kind == IR::stmType::cjump) stmlBlockmap[stml] = it->mykey;
                stml = stml->tail;
            }
        }
    };
    auto branchTest = [&](IR::StmList* stml) {
        if (stml->stm->kind == IR::stmType::cjump) {
            auto cjumpstm = static_cast<IR::Cjump*>(stml->stm);
            if (isConstExp(cjumpstm->left) && isConstExp(cjumpstm->right)) {
                auto lv = evalExp(cjumpstm->left), rv = evalExp(cjumpstm->right);
                auto b = evalRel(cjumpstm->op, lv, rv);
                auto jb = stmlBlockmap[stml];
                auto nodes = ir->nodes();
                int cnt = 0;
                // fixme without free stm
                if (b) {
                    stml->stm = new IR::Jump(cjumpstm->trueLabel);

                    for (auto it : (*(nodes->at(jb))->succ())) {
                        if (getNodeLabel(it) == cjumpstm->falseLabel) {
                            for (auto jt : (*(it->pred()))) {
                                if (jt->mykey == jb) { break; }
                                cnt++;
                            }
                            // for(auto jt:)
                            ir->rmEdge(nodes->at(jb), it);
                            // TODO
                            break;
                        }
                    }
                } else {
                    stml->stm = new IR::Jump(cjumpstm->falseLabel);
                    for (auto it : (*(nodes->at(jb))->succ())) {
                        if (getNodeLabel(it) == cjumpstm->trueLabel) {
                            ir->rmEdge(nodes->at(jb), it);
                            // TODO
                            break;
                        }
                    }
                }
            }
        }
    };
    auto bfsMark = [&]() {
        // while (!curBlock.empty() && !curTemp.empty()) {
        while (!curTemp.empty()) {
            auto cur = curTemp.front();
            curTemp.pop();
            auto def = (static_cast<IR::Move*>(tempDef[cur]->stm))->src;
            int val = evalExp(def);
            for (auto it : tempUse[cur]) {
                constReplace(cur, it->stm, val);
                auto nxdf = getDef(it->stm);
                if (nxdf) {
                    // set up next temp
                    bool necessary = isConstDef(it->stm);
                    if (necessary) { curTemp.push(static_cast<IR::Temp*>(*nxdf)->tempid); }
                }
                branchTest(it);
            }
            tempDef[cur]->stm = nopStm();
        }
        //     while (!curBlock.empty()) {}
        // }
    };
    setup();
    bfsMark();
}
}  // namespace SSAOPT