// //#include "optimizer.hpp"
// #include "BuildSSA.hpp"
// #include "../structure/treeIR.hpp"
// #include <vector>
// #include <assert.h>
// #include <memory>
// #include <unordered_map>
// #include <unordered_set>
// #include <queue>
// #include <functional>
// #include "../util/utils.hpp"
// #include "CDG.hpp"
// using std::function;
// using std::move;
// using std::queue;
// using std::unordered_map;
// using std::unordered_set;
// using std::vector;

// namespace SSA {
// // enum class COND { undefined, constant, indefinite };
// // struct TEMP_COND {
// //     COND cond;
// //     int val;
// // };
// // TEMP_COND udf() {
// //     TEMP_COND x;
// //     x.cond = COND::undefined;
// //     return x;
// // }
// // TEMP_COND cst(int y) {
// //     TEMP_COND x;
// //     x.cond = COND::constant;
// //     x.val = y;
// //     return x;
// // }
// // TEMP_COND idf() {
// //     TEMP_COND x;
// //     x.cond = COND::indefinite;
// //     return x;
// // }
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
// bool evalRel(IR::RelOp op, int lv, int rv) {
//     switch (op) {
//     case IR::RelOp::T_eq: return lv == rv;
//     case IR::RelOp::T_ge: return lv >= rv;
//     case IR::RelOp::T_gt: return lv > rv;
//     case IR::RelOp::T_le: return lv <= rv;
//     case IR::RelOp::T_lt: return lv < rv;
//     case IR::RelOp::T_ne: return lv != rv;
//     default: break;
//     }
//     assert(0);
// }
// Temp_Temp evalCopy(IR::Exp* exp) {
//     // MUST MAKE SURE EXP IS COPYABLE
//     if (exp->kind == IR::expType::temp) { return static_cast<IR::Temp*>(exp)->tempid; }
//     if (exp->kind == IR::expType::call) {
//         auto callexp = static_cast<IR::Call*>(exp);
//         return static_cast<IR::Temp*>(callexp->args[0])->tempid;
//     }
//     assert(0);
//     return -1;
// }
// void SSA::Optimizer::constantPropagation() {
//     // unordered_map<Temp_Temp, TEMP_COND> tempCondition;
//     unordered_map<IR::StmList*, int> stmlBlockmap;
//     unordered_map<Temp_Temp, IR::StmList*> tempDef;
//     unordered_map<Temp_Temp, vector<IR::StmList*>> tempUse;
//     // unordered_map<int, int> blockCondition;
//     //  queue<int> curBlock;
//     queue<Temp_Temp> curTemp;
//     queue<Temp_Temp> copyTemp;

//     auto nodes = ir->nodes();
//     auto setup = [&]() {
//         // auto root = nodes->at(0)->mykey;
//         // blockCondition.insert({root, 1});
//         // curBlock.push(root);
//         for (const auto& it : (*nodes)) {
//             auto stml = static_cast<IR::StmList*>(it->info);
//             while (stml) {
//                 auto stm = stml->stm;

//                 // set up def-stm mapping
//                 auto df = getDef(stm);
//                 if (df) {
//                     stmlBlockmap[stml] = it->mykey;
//                     tempDef[static_cast<IR::Temp*>(*df)->tempid] = stml;
//                     // set up seed temp
//                     bool cnst = isConstDef(stm);
//                     if (cnst) {
//                         curTemp.push(static_cast<IR::Temp*>(*df)->tempid);
//                     } else {
//                         bool cpy = isCopyDef(stm);
//                         if (cpy) { copyTemp.push(static_cast<IR::Temp*>(*df)->tempid); }
//                     }
//                 }
//                 auto us = getUses(stm);
//                 for (auto it : us) {
//                     // maybe same stm pushed into info
//                     tempUse[static_cast<IR::Temp*>(*it)->tempid].push_back(stml);
//                 }
//                 if (stm->kind == IR::stmType::cjump) stmlBlockmap[stml] = it->mykey;
//                 stml = stml->tail;
//             }
//         }
//     };
//     function<void(int, int)> cutEdge = [&](int from, int to) {
//         int cnt = 0;
//         auto toNode = nodes->at(to);
//         for (auto jt : (*(toNode->pred()))) {
//             if (jt->mykey == from) { break; }
//             cnt++;
//         }
//         // jt to modify the phi func
//         for (auto jt : ir->Aphi[to]) {
//             auto src = (static_cast<IR::Move*>(jt.second->stm))->src;
//             auto callexp = static_cast<IR::Call*>(src);
//             callexp->args.erase(callexp->args.begin() + cnt);
//             auto def
//                 =
//                 (static_cast<IR::Temp*>((static_cast<IR::Move*>(jt.second->stm)->dst))->tempid);
//             if (isConstDef(jt.second->stm)) {
//                 curTemp.push(def);
//             } else if (isCopyDef(jt.second->stm)) {
//                 copyTemp.push(def);
//             }
//         }
//         ir->rmEdge(nodes->at(from), toNode);
//         if (toNode->pred()->empty()) {
//             while (!toNode->succ()->empty()) { cutEdge(to, (*(toNode->succ()->begin()))->mykey);
//             }
//         }
//     };
//     auto branchTest = [&](IR::StmList* stml) {
//         if (stml->stm->kind == IR::stmType::cjump) {
//             auto cjumpstm = static_cast<IR::Cjump*>(stml->stm);
//             if (isConstExp(cjumpstm->left) && isConstExp(cjumpstm->right)) {
//                 auto lv = evalExp(cjumpstm->left), rv = evalExp(cjumpstm->right);
//                 auto b = evalRel(cjumpstm->op, lv, rv);
//                 auto jb = stmlBlockmap[stml];
//                 auto nodes = ir->nodes();
//                 int cnt = 0;
//                 // fixme without free stm
//                 if (b) {
//                     // jb---from  it---block cannot reach
//                     stml->stm = new IR::Jump(cjumpstm->trueLabel);
//                     for (auto it : (*(nodes->at(jb))->succ())) {
//                         if (getNodeLabel(it) == cjumpstm->falseLabel) {
//                             cutEdge(jb, it->mykey);
//                             break;
//                         }
//                     }
//                 } else {
//                     stml->stm = new IR::Jump(cjumpstm->falseLabel);
//                     for (auto it : (*(nodes->at(jb))->succ())) {
//                         if (getNodeLabel(it) == cjumpstm->trueLabel) {
//                             cutEdge(jb, it->mykey);
//                             break;
//                         }
//                     }
//                 }
//             }
//         }
//     };
//     auto branchTestCOPY = [&](IR::StmList* stml) {
//         if (stml->stm->kind == IR::stmType::cjump) {
//             auto cjumpstm = static_cast<IR::Cjump*>(stml->stm);
//             if (cjumpstm->left->kind == IR::expType::temp
//                 && cjumpstm->right->kind == IR::expType::temp
//                 && static_cast<IR::Temp*>(cjumpstm->left)->tempid
//                        == static_cast<IR::Temp*>(cjumpstm->right)->tempid) {

//                 bool b;
//                 if (cjumpstm->op == IR::RelOp::T_eq || cjumpstm->op == IR::RelOp::T_le
//                     || cjumpstm->op == IR::RelOp::T_ge) {
//                     b = true;
//                 } else if (cjumpstm->op == IR::RelOp::T_ne) {
//                     b = false;
//                 } else
//                     return;

//                 auto jb = stmlBlockmap[stml];
//                 auto nodes = ir->nodes();
//                 int cnt = 0;
//                 // fixme without free stm
//                 if (b) {
//                     stml->stm = new IR::Jump(cjumpstm->trueLabel);
//                     for (auto it : (*(nodes->at(jb))->succ())) {
//                         if (getNodeLabel(it) == cjumpstm->falseLabel) {
//                             cutEdge(jb, it->mykey);
//                             break;
//                         }
//                     }
//                 } else {
//                     stml->stm = new IR::Jump(cjumpstm->falseLabel);
//                     for (auto it : (*(nodes->at(jb))->succ())) {
//                         if (getNodeLabel(it) == cjumpstm->trueLabel) {
//                             cutEdge(jb, it->mykey);
//                             break;
//                         }
//                     }
//                 }
//             }
//         }
//     };
//     auto bfsMark = [&]() {
//         while (!copyTemp.empty() && !curTemp.empty()) {
//             while (!curTemp.empty()) {
//                 auto cur = curTemp.front();
//                 curTemp.pop();
//                 auto def = (static_cast<IR::Move*>(tempDef[cur]->stm))->src;
//                 int val = evalExp(def);
//                 for (auto it : tempUse[cur]) {
//                     // constReplace(cur, it->stm, val);
//                     auto uses = getUses(it->stm);
//                     for (auto jt : uses) {
//                         if (static_cast<IR::Temp*>(*jt)->tempid == cur) {
//                             // FIXME need free here
//                             *jt = new IR::Const(val);
//                         }
//                     }
//                     auto nxdf = getDef(it->stm);
//                     if (nxdf) {
//                         // set up next temp
//                         bool necessary = isConstDef(it->stm);
//                         if (necessary) { curTemp.push(static_cast<IR::Temp*>(*nxdf)->tempid); }
//                     }
//                     branchTest(it);
//                 }
//                 tempDef[cur]->stm = nopStm();
//             }
//             while (!copyTemp.empty()) {
//                 auto cur = copyTemp.front();
//                 copyTemp.pop();
//                 auto def = (static_cast<IR::Move*>(tempDef[cur]->stm))->src;
//                 if (isConstExp(def)) {
//                     curTemp.push(cur);
//                     continue;
//                 }
//                 auto val = evalCopy(def);
//                 for (auto it : tempUse[cur]) {
//                     // copyReplace(cur, it->stm, val);
//                     auto uses = getUses(it->stm);
//                     for (auto jt : uses) {
//                         if (static_cast<IR::Temp*>(*jt)->tempid == cur) {
//                             // FIXME need free here
//                             *jt = new IR::Temp(val);
//                         }
//                     }
//                     auto nxdf = getDef(it->stm);
//                     if (nxdf) {
//                         // set up next temp
//                         bool cpy = isCopyDef(it->stm);
//                         if (cpy) { copyTemp.push(static_cast<IR::Temp*>(*nxdf)->tempid); }
//                     }
//                     branchTestCOPY(it);
//                 }
//                 if (def->kind == IR::expType::call) {
//                     ir->Aphi[stmlBlockmap[tempDef[cur]]].erase(cur);
//                 }
//                 tempDef[cur]->stm = nopStm();
//             }
//             //     while (!curBlock.empty()) {}
//         }
//     };
//     setup();
//     bfsMark();
// }
// }  // namespace SSA