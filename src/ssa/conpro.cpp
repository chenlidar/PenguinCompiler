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
// int evalExp(IR::Exp* exp) {
//     // MUST MAKE SURE EXP IS CONSTEXP
//     // calculate the value of given const exp
//     switch (exp->kind) {
//     case IR::expType::binop: {
//         auto binopexp = static_cast<IR::Binop*>(exp);
//         auto lf = evalExp(binopexp->left), rg = evalExp(binopexp->right);
//         switch (binopexp->op) {
//         case IR::binop::T_div: return lf / rg;
//         case IR::binop::T_minus: return lf - rg;
//         case IR::binop::T_mod: return lf % rg;
//         case IR::binop::T_mul: return lf * rg;
//         case IR::binop::T_plus: return lf + rg;
//         default: assert(0);
//         }
//     }
//     case IR::expType::call: {
//         auto callexp = static_cast<IR::Call*>(exp);
//         return evalExp(callexp->args[0]);
//     }
//     case IR::expType::constx: return (static_cast<IR::Const*>(exp))->val;
//     case IR::expType::eseq: assert(0);
//     case IR::expType::mem: assert(0);
//     case IR::expType::name: assert(0);
//     case IR::expType::temp: assert(0);
//     default: assert(0);
//     }
// }
// bool isConstExp(IR::Exp* exp) {
//     switch (exp->kind) {
//     case IR::expType::binop: {
//         auto binopexp = static_cast<IR::Binop*>(exp);
//         return isConstExp(binopexp->left) && isConstExp(binopexp->right);
//     }
//     case IR::expType::call: {
//         auto callexp = static_cast<IR::Call*>(exp);
//         if (callexp->fun[0] == '$') {
//             int len = callexp->args.size();
//             if (!isConstExp(callexp->args[0])) return false;
//             int guess = evalExp(callexp->args[0]);
//             for (int i = 1; i < len; i++) {
//                 if (!isConstExp(callexp->args[i])) return false;
//                 if (evalExp(callexp->args[i]) != guess) return false;
//             }
//             return true;
//         }
//         return false;
//     }
//     case IR::expType::constx: return true;
//     case IR::expType::eseq: assert(0);
//     case IR::expType::mem: return false;
//     case IR::expType::name: return false;
//     case IR::expType::temp: return false;
//     default: assert(0);
//     }
//     return false;
// }
// bool isConstDef(IR::Stm* stm) {
//     // check the stm moving a const value to a temp
//     if (stm->kind == IR::stmType::move) {
//         auto movestm = static_cast<IR::Move*>(stm);
//         if (movestm->dst->kind != IR::expType::temp) return false;
//         if (isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid)) return false;
//         auto src = movestm->src;
//         return isConstExp(src);
//     }
//     return false;
// }
// bool isCopyDef(IR::Stm* stm) {
//     // check the stm moving a temp or a single phi to a temp
//     if (stm->kind == IR::stmType::move) {
//         auto movestm = static_cast<IR::Move*>(stm);
//         if (movestm->dst->kind != IR::expType::temp) return false;
//         if (isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid)) return false;
//         auto src = movestm->src;
//         if (src->kind == IR::expType::temp) {
//             if (!isRealregister(static_cast<IR::Temp*>(movestm->src)->tempid)) return true;
//             return false;
//         }
//         if (src->kind == IR::expType::call) {
//             auto callexp = static_cast<IR::Call*>(src);
//             if (callexp->fun[0] == '$') {
//                 int len = callexp->args.size();
//                 if (!len) return false;
//                 if (callexp->args[0]->kind == IR::expType::temp
//                     && !isRealregister(static_cast<IR::Temp*>(callexp->args[0])->tempid)) {
//                     auto guess = static_cast<IR::Temp*>(callexp->args[0])->tempid;
//                     for (int i = 1; i < len; i++) {
//                         if (callexp->args[i]->kind != IR::expType::temp
//                             || static_cast<IR::Temp*>(callexp->args[i])->tempid != guess) {
//                             return false;
//                         }
//                     }
//                     return true;
//                 }
//                 return true;
//             }
//         }
//     }
//     return false;
// }
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
// Temp_Temp evalCopy(IR::Exp* exp) {
//     // MUST MAKE SURE EXP IS COPYABLE
//     if (exp->kind == IR::expType::temp) { return static_cast<IR::Temp*>(exp)->tempid; }
//     if (exp->kind == IR::expType::call) {
//         auto callexp = static_cast<IR::Call*>(exp);
//         return static_cast<IR::Temp*>(callexp->args[0])->tempid;
//     }
// }
void SSA::Optimizer::constantPropagation() {
    unordered_map<Temp_Temp, TEMP_COND> tempCondition;
    unordered_map<IR::StmList*, int> stmlBlockmap;
    unordered_map<Temp_Temp, IR::StmList*> tempDef;
    unordered_map<Temp_Temp, vector<IR::StmList*>> tempUse;
    unordered_set<int> blockCondition;
    queue<int> curBlock;
    queue<Temp_Temp> curTemp;
    queue<Temp_Temp> copyTemp;

    auto nodes = ir->nodes();
    auto setup = [&]() {
        auto root = nodes->at(0)->mykey;
        blockCondition.insert(root);
        curBlock.push(root);
        // for (const auto& it : (*nodes)) {
        //     auto stml = static_cast<IR::StmList*>(it->info);
        //     while (stml) {
        //         auto stm = stml->stm;

        //         // set up def-stm mapping
        //         auto df = getDef(stm);
        //         if (df) {
        //             stmlBlockmap[stml] = it->mykey;
        //             tempDef[static_cast<IR::Temp*>(*df)->tempid] = stml;
        //             // set up seed temp
        //             bool cnst = isConstDef(stm);
        //             if (cnst) {
        //                 curTemp.push(static_cast<IR::Temp*>(*df)->tempid);
        //             } else {
        //                 bool cpy = isCopyDef(stm);
        //                 if (cpy) { copyTemp.push(static_cast<IR::Temp*>(*df)->tempid); }
        //             }
        //         }
        //         auto us = getUses(stm);
        //         for (auto it : us) {
        //             // maybe same stm pushed into info
        //             tempUse[static_cast<IR::Temp*>(*it)->tempid].push_back(stml);
        //         }
        //         if (stm->kind == IR::stmType::cjump) stmlBlockmap[stml] = it->mykey;
        //         stml = stml->tail;
        //     }
        // }
    };

    function<void(int, int)> cutEdge = [&](int from, int to) {
        // assert(0);
        int cnt = 0;
        auto toNode = nodes->at(to);
        for (auto jt : ir->prednode[to]) {
            if (jt == from) { break; }
            cnt++;
        }
        assert(cnt != ir->prednode[to].size());
        ir->prednode[to].erase(ir->prednode[to].begin() + cnt);
        // jt to modify the phi func
        for (auto jt : ir->Aphi[to]) {
            auto src = (static_cast<IR::Move*>(jt.second->stm))->src;
            auto callexp = static_cast<IR::Call*>(src);
            callexp->args.erase(callexp->args.begin() + cnt);
            auto def
                = (static_cast<IR::Temp*>((static_cast<IR::Move*>(jt.second->stm)->dst))->tempid);
        }
        ir->rmEdge(nodes->at(from), toNode);
        if (toNode->pred()->empty()) {
            while (!toNode->succ()->empty()) { cutEdge(to, (*(toNode->succ()->begin()))->mykey); }
        }
    };
    // auto branchTest = [&](IR::StmList* stml) {
    //     if (stml->stm->kind == IR::stmType::cjump) {
    //         auto cjumpstm = static_cast<IR::Cjump*>(stml->stm);
    //         if (isConstExp(cjumpstm->left) && isConstExp(cjumpstm->right)) {
    //             auto lv = evalExp(cjumpstm->left), rv = evalExp(cjumpstm->right);
    //             auto b = evalRel(cjumpstm->op, lv, rv);
    //             auto jb = stmlBlockmap[stml];
    //             auto nodes = ir->nodes();
    //             int cnt = 0;
    //             // fixme without free stm
    //             if (b) {
    //                 // jb---from  it---block cannot reach
    //                 stml->stm = new IR::Jump(cjumpstm->trueLabel);
    //                 for (auto it : (*(nodes->at(jb))->succ())) {
    //                     if (getNodeLabel(it) == cjumpstm->falseLabel) {
    //                         cutEdge(jb, it->mykey);
    //                         break;
    //                     }
    //                 }
    //             } else {
    //                 stml->stm = new IR::Jump(cjumpstm->falseLabel);
    //                 for (auto it : (*(nodes->at(jb))->succ())) {
    //                     if (getNodeLabel(it) == cjumpstm->trueLabel) {
    //                         cutEdge(jb, it->mykey);
    //                         break;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // };
    // auto branchTestCOPY = [&](IR::StmList* stml) {
    //     if (stml->stm->kind == IR::stmType::cjump) {
    //         auto cjumpstm = static_cast<IR::Cjump*>(stml->stm);
    //         if (cjumpstm->left->kind == IR::expType::temp
    //             && cjumpstm->right->kind == IR::expType::temp
    //             && static_cast<IR::Temp*>(cjumpstm->left)->tempid
    //                    == static_cast<IR::Temp*>(cjumpstm->right)->tempid) {

    //             bool b;
    //             if (cjumpstm->op == IR::RelOp::T_eq || cjumpstm->op == IR::RelOp::T_le
    //                 || cjumpstm->op == IR::RelOp::T_ge) {
    //                 b = true;
    //             } else if (cjumpstm->op == IR::RelOp::T_ne) {
    //                 b = false;
    //             } else
    //                 return;

    //             auto jb = stmlBlockmap[stml];
    //             auto nodes = ir->nodes();
    //             int cnt = 0;
    //             // fixme without free stm
    //             if (b) {
    //                 stml->stm = new IR::Jump(cjumpstm->trueLabel);
    //                 for (auto it : (*(nodes->at(jb))->succ())) {
    //                     if (getNodeLabel(it) == cjumpstm->falseLabel) {
    //                         cutEdge(jb, it->mykey);
    //                         break;
    //                     }
    //                 }
    //             } else {
    //                 stml->stm = new IR::Jump(cjumpstm->falseLabel);
    //                 for (auto it : (*(nodes->at(jb))->succ())) {
    //                     if (getNodeLabel(it) == cjumpstm->trueLabel) {
    //                         cutEdge(jb, it->mykey);
    //                         break;
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // };
    auto isTempIndefinite = [&](IR::Exp* exp) {
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
    };
    auto isTempConst = [&](IR::Exp* exp) {
        if (exp->kind == IR::expType::constx) return true;
        if (exp->kind == IR::expType::temp) {
            auto tmp = static_cast<IR::Temp*>(exp);
            if (tempCondition.count(tmp->tempid)) {
                return tempCondition[tmp->tempid].cond == COND::constant;
            }
            return false;
        }
        return false;
    };
    function<int(IR::Exp*)> tempEval = [&](IR::Exp* exp) {
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
    };
    auto doDef = [&](IR::StmList* stml) {
        auto defid = static_cast<IR::Temp*>(static_cast<IR::Move*>(stml->stm)->dst)->tempid;
        auto src = static_cast<IR::Move*>(stml->stm)->src;
        // std::cerr << "dodef" << defid << std::endl;
        // stml->stm->printIR();
        int base = 0, nxcond = 0;
        if (tempCondition.count(defid)) { base = static_cast<int>(tempCondition[defid].cond); }
        if (base == 2) return;

        switch (src->kind) {
        case IR::expType::binop: {
            auto bexp = static_cast<IR::Binop*>(src);
            if (isTempConst(bexp->left) && isTempConst(bexp->right)) {
                nxcond = 1;
                tempCondition[defid] = cst(tempEval(bexp));
            } else {
                auto lf = isTempIndefinite(bexp->left);
                auto rf = isTempIndefinite(bexp->right);
                if (lf || rf) {
                    nxcond = 2;
                    tempCondition[defid] = idf();
                } else {
                    assert(0);
                }
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
                        // if (defid == 1011) { std::cerr << "8888\n"; }
                        if (cal->args[i]->kind == IR::expType::temp) {
                            auto tempid = static_cast<IR::Temp*>(cal->args[i])->tempid;
                            if (!tempCondition.count(tempid)) {
                                if (isRealregister(tempid)) {
                                    nxcond = 2;
                                    tempCondition[defid] = idf();
                                    break;
                                }
                                continue;
                            } else {
                                auto b = isTempIndefinite(cal->args[i]);
                                if (b) {
                                    nxcond = 2;
                                    tempCondition[defid] = idf();
                                    break;
                                }
                                assert(isTempConst(cal->args[i]));

                                int tval = tempEval(cal->args[i]);
                                // if (defid == 1017) std::cerr << tval << "$$$$\n";
                                if (flag && tval != guess) {
                                    nxcond = 2;
                                    // if (defid == 1017) std::cerr << nxcond << "$$\n";
                                    // tempCondition[defid] = idf();
                                    tempCondition[defid] = idf();
                                    // if (defid == 1017)
                                    //     std::cerr << static_cast<int>(idf().cond) << "$$$$\n";
                                    break;
                                } else {
                                    flag = 1;
                                    guess = tval;
                                }
                            }
                        } else if (cal->args[i]->kind == IR::expType::constx) {
                            if (flag) {
                                if (guess != tempEval(cal->args[i])) {
                                    nxcond = 2;
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
                if (nxcond < 2) {
                    if (flag) {
                        nxcond = 1;
                        tempCondition[defid] = cst(guess);
                    } else {
                        // all cannot reach or without initialization
                        nxcond = 1;
                        tempCondition[defid] = cst(0);
                    }
                }
            } else {
                tempCondition[defid] = idf();
                nxcond = 2;
            }
            break;
        }
        case IR::expType::constx: {
            auto cs = static_cast<IR::Const*>(src);
            tempCondition[defid] = cst(cs->val);
            nxcond = 1;
            break;
        }
        case IR::expType::eseq: assert(0);
        case IR::expType::mem: {
            tempCondition[defid] = idf();
            nxcond = 2;
            break;
        }
        case IR::expType::name: {
            tempCondition[defid] = idf();
            nxcond = 2;
            break;
        }
        case IR::expType::temp: {
            auto ot = isTempIndefinite(src);
            if (ot) {
                tempCondition[defid] = idf();
                nxcond = 2;
            } else {
                tempCondition[defid] = tempCondition[static_cast<IR::Temp*>(src)->tempid];
                nxcond = 1;
            }
            break;
        }
        default: assert(0);
        }
        // if (defid == 1017) std::cerr << static_cast<int>(tempCondition[1017].cond) << "$$$$\n";
        if (nxcond > base) {
            // FIXME
            curTemp.push(defid);
        }
    };
    auto doJump = [&](IR::StmList* stml) {
        auto stm = stml->stm;
        auto curb = stmlBlockmap[stml];
        // stm->printIR();
        if (stm->kind == IR::stmType::jump) {
            // assert(nodes->at(curb)->succ()->size() == 1);
            // for (auto it : *(nodes->at(curb)->succ())) {
            //     std::cerr << getNodeLabel(it) << std::endl;
            // }
            assert(nodes->at(curb)->succ()->size() == 1);
            auto nx = (*nodes->at(curb)->succ()->begin())->mykey;
            if (!blockCondition.count(nx)) {
                blockCondition.insert(nx);
                curBlock.push(nx);
            }
        } else {
            auto cjmp = static_cast<IR::Cjump*>(stm);
            if (isTempConst(cjmp->left) && isTempConst(cjmp->right)) {
                auto lv = tempEval(cjmp->left), rv = tempEval(cjmp->right);
                auto b = evalRel(cjmp->op, lv, rv);
                int todelete = -1;
                if (b) {
                    // stml->stm = new IR::Jump(cjmp->trueLabel);
                    for (auto it : (*(nodes->at(curb))->succ())) {
                        if (getNodeLabel(it) == cjmp->trueLabel) {
                            auto nx = it->mykey;
                            if (!blockCondition.count(nx)) {
                                blockCondition.insert(nx);
                                curBlock.push(nx);
                            }

                        } else {
                            todelete = it->mykey;
                        }
                    }
                } else {
                    // stml->stm = new IR::Jump(cjmp->falseLabel);
                    for (auto it : (*(nodes->at(curb))->succ())) {
                        if (getNodeLabel(it) == cjmp->falseLabel) {
                            auto nx = it->mykey;
                            if (!blockCondition.count(nx)) {
                                blockCondition.insert(nx);
                                curBlock.push(nx);
                            }

                        } else {
                            todelete = it->mykey;
                        }
                    }
                }
                // cutEdge(curb, todelete);
                return;
            }
            if ((cjmp->left->kind == IR::expType::temp && isTempIndefinite(cjmp->left))
                || (cjmp->right->kind == IR::expType::temp && isTempIndefinite(cjmp->right))) {
                for (auto it : (*(nodes->at(curb))->succ())) {
                    auto nx = it->mykey;
                    if (!blockCondition.count(nx)) {
                        blockCondition.insert(nx);
                        curBlock.push(nx);
                    }
                }
                return;
            }
            assert(0);
        }
    };
    auto bfsMark = [&]() {
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
                        // std::cerr << "tid:" << static_cast<IR::Temp*>(*it)->tempid;
                        // stml->stm->printIR();
                    }
                    if (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump) {
                        stmlBlockmap[stml] = cur;
                        doJump(stml);
                    }
                    stml = stml->tail;
                }
                for (auto nx : *(ir->nodes()->at(cur)->succ())) {
                    if (blockCondition.count(nx->mykey)) {
                        for (auto st : ir->Aphi[nx->mykey]) { doDef(st.second); }
                    }
                }
            }
            while (!curTemp.empty()) {
                auto cur = curTemp.front();
                curTemp.pop();
                auto def = (static_cast<IR::Move*>(tempDef[cur]->stm))->src;

                for (auto use : tempUse[cur]) {
                    auto df = getDef(use->stm);

                    if (df) {
                        // if (cur == 1016 && static_cast<IR::Temp*>(*df)->tempid == 1011) {
                        //     std::cerr << "aaaaaaa\n";
                        //     std::cerr << 1011 << ":" <<
                        //     static_cast<int>(tempCondition[1011].cond)
                        //               << std::endl;
                        // }
                        doDef(use);
                        // if (cur == 1016 && static_cast<IR::Temp*>(*df)->tempid == 1011) {
                        //     std::cerr << "bbbbbb\n";
                        //     std::cerr << 1011 << ":" <<
                        //     static_cast<int>(tempCondition[1011].cond)
                        //               << std::endl;
                        //     std::cerr << 1016 << ":" <<
                        //     static_cast<int>(tempCondition[1016].cond)
                        //               << std::endl;
                        // }
                    }
                    if (use->stm->kind == IR::stmType::cjump
                        || use->stm->kind == IR::stmType::jump) {
                        if (cur == 1017) {
                            // std::cerr << 1017 << ":" <<
                            // static_cast<int>(tempCondition[1017].cond)
                            //           << std::endl;
                            // use->stm->printIR();
                        }
                        doJump(use);
                    }
                }
            }
        }
    };
    // auto paphi = [&]() {
    //     std::cerr << "---------------------------------\n";
    //     for (int i = 0; i < ir->nodecount; i++) {
    //         if (ir->nodes()->at(i)->inDegree() <= 1) continue;
    //         for (auto it : ir->Aphi[i])
    //             std::cerr << i << " "
    //                       << static_cast<IR::Call*>(static_cast<IR::Move*>(it.second->stm)->src)
    //                              ->args.size()
    //                       << "\n ";
    //     }
    // };
    auto replaceTemp = [&]() {
        auto nodes = ir->nodes();
        for (auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            if (!blockCondition.count(it->mykey)) {
                while (!it->pred()->empty()) { ir->rmEdge((*(it->pred()->begin())), it); }
                while (!it->succ()->empty()) {
                    cutEdge(it->mykey, (*(it->succ()->begin()))->mykey);
                }
                it->info = 0;
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
                    if (!blockCondition.count(jt->mykey)) continue;
                    if (getNodeLabel(jt) == cjmpstm->trueLabel) {
                        ht = true;
                    } else if (getNodeLabel(jt) == cjmpstm->falseLabel) {
                        hf = true;
                    }
                }
                assert(ht || hf);
                if (!ht) {
                    head->stm = new IR::Jump(cjmpstm->falseLabel);
                    // delete cj
                }
                if (!hf) {
                    head->stm = new IR::Jump(cjmpstm->trueLabel);
                    // delete cj
                }
            }
        }
    };
    auto showmark = [&]() {  // func that can output ssa for debuging
        std::cerr << "CON###\n";
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                stm->printIR();
                // if(stm->kind==IR::stmType::move){
                //     auto mv=static_cast<IR::Move*>(stm);

                // }
                stml = stml->tail;
            }
        }
    };
    auto cleanup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            if (it->inDegree() == 1) {
                for (auto jt : ir->Aphi[it->mykey]) {
                    auto mv = static_cast<IR::Move*>(jt.second->stm);
                    auto cl = static_cast<IR::Call*>(mv->src);
                    mv->src = cl->args[0]->quad();
                    // delete cl
                }
                ir->Aphi[it->mykey].clear();
            }
        }
    };
    auto showtemptable = [&]() {
        for (auto it : tempCondition) {
            std::cerr << "t" << it.first << ": type" << static_cast<int>(it.second.cond)
                      << "   val:" << it.second.val << std::endl;
        }
    };

    // showmark();

    setup();
    bfsMark();
    replaceTemp();
    cleanup();
    // showmark();
    // showtemptable();
};

}  // namespace SSA