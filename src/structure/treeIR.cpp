#include "treeIR.hpp"
#include <assert.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include "../backend/canon.hpp"
#include "../util/utils.hpp"
#include "../util/muldiv.hpp"
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
#include "assem.h"
using std::endl;
using std::string;
using namespace IR;

RelOp IR::commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
    case RelOp::T_eq: return RelOp::T_eq;
    case RelOp::T_ne: return RelOp::T_ne;
    case RelOp::T_lt: return RelOp::T_gt;
    case RelOp::T_ge: return RelOp::T_le;
    case RelOp::T_gt: return RelOp::T_lt;
    case RelOp::T_le: return RelOp::T_ge;
    case RelOp::F_eq: return RelOp::F_eq;
    case RelOp::F_ne: return RelOp::F_ne;
    case RelOp::F_lt: return RelOp::F_gt;
    case RelOp::F_ge: return RelOp::F_le;
    case RelOp::F_gt: return RelOp::F_lt;
    case RelOp::F_le: return RelOp::F_ge;
    }
}
RelOp IR::notRel(RelOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
    case RelOp::T_eq: return RelOp::T_ne;
    case RelOp::T_ne: return RelOp::T_eq;
    case RelOp::T_lt: return RelOp::T_ge;
    case RelOp::T_ge: return RelOp::T_lt;
    case RelOp::T_gt: return RelOp::T_le;
    case RelOp::T_le: return RelOp::T_gt;
    case RelOp::F_eq: return RelOp::F_ne;
    case RelOp::F_ne: return RelOp::F_eq;
    case RelOp::F_lt: return RelOp::F_ge;
    case RelOp::F_ge: return RelOp::F_lt;
    case RelOp::F_gt: return RelOp::F_le;
    case RelOp::F_le: return RelOp::F_gt;
    }
}
Cx IR::Tr_Exp::unCx() {
    switch (this->kind) {
    case Tr_ty::Tr_cx: {
        return this->cx;
    } break;
    case Tr_ty::Tr_ex: {
        Stm* stm = new Cjump(RelOp::T_ne, this->ex, new Const(0), "", "");
        Cx cx;
        cx.stm = stm;
        cx.falses = new PatchList(&static_cast<Cjump*>(stm)->falseLabel, NULL);
        cx.trues = new PatchList(&static_cast<Cjump*>(stm)->trueLabel, NULL);
        return cx;
    } break;
    default: assert(0);
    }
}
Exp* IR::Tr_Exp::unEx() {
    switch (this->kind) {
    case Tr_ty::Tr_ex: {
        return this->ex;
    } break;
    case Tr_ty::Tr_cx: {
        Temp_Temp r = Temp_newtemp();
        Temp_Label t = Temp_newlabel(), f = Temp_newlabel();
        doPatch(this->cx.trues, t);
        doPatch(this->cx.falses, f);
        return new Eseq(
            new Move(new Temp(r), new Const(1)),
            new Eseq(this->cx.stm,
                     new Eseq(new Label(f), new Eseq(new Move(new Temp(r), new Const(0)),
                                                     new Eseq(new Label(t), new Temp(r))))));
    } break;
    default: assert(0);
    }
}
Stm* IR::Tr_Exp::unNx() {
    switch (this->kind) {
    case Tr_ty::Tr_ex: return new ExpStm(this->ex);
    case Tr_ty::Tr_cx: return new ExpStm(this->unEx());
    case Tr_ty::Tr_nx: return this->nx;
    }
    assert(0);
}

IR::Tr_Exp::Tr_Exp(Exp* ex) {
    this->kind = Tr_ty::Tr_ex;
    this->ex = ex;
}

IR::Tr_Exp::Tr_Exp(PatchList* trues, PatchList* falses, Stm* stm) {
    this->kind = Tr_ty::Tr_cx;
    this->cx.trues = trues;
    this->cx.falses = falses;
    this->cx.stm = stm;
}

IR::Tr_Exp::Tr_Exp(Stm* stm) {
    this->kind = Tr_ty::Tr_nx;
    this->nx = stm;
}

void Label::printIR() { std::cerr << "LABELSTM:    " << label << endl; }
void Jump::printIR() {
    std::cerr << "JUMPSTM:    " + target;
    std::cerr << endl;
}
void Cjump::printIR() {
    std::cerr << "CJUMPSTM:    ";
    left->printIR();
    std::cerr << " " + relop2string(op) + " ";
    right->printIR();
    std::cerr << endl << "true:    " << trueLabel << endl << "false:    " << falseLabel << endl;
}
void Move::printIR() {
    std::cerr << "MOVESTM:    ";
    dst->printIR();
    std::cerr << "    ";
    src->printIR();
    std::cerr << endl;
}
void ExpStm::printIR() {
    std::cerr << "EXPSTM:    ";
    exp->printIR();
    std::cerr << endl;
}
void Const::printIR() { std::cerr << val; }
void Binop::printIR() {
    std::cerr << "( ";
    left->printIR();
    std::cerr << " " + binop2string(op) + " ";
    right->printIR();
    std::cerr << " )";
}
void Temp::printIR() { std::cerr << "t" << tempid; }
void Mem::printIR() {
    std::cerr << "Mem( ";
    mem->printIR();
    std::cerr << " )";
}
void Name::printIR() { std::cerr << name; }
void Call::printIR() {
    std::cerr << "( call " + fun;
    std::cerr << '(';
    for (auto it : args) {
        it->printIR();
        std::cerr << ", ";
    }
    std::cerr << ") )";
}

// DEEPCOPY

StmList* StmList::deepCopy(unordered_set<Temp_Label>& venv, int offset,
                           unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                           unordered_map<string, string>& lbmp) {
    vector<Stm*> ls;
    auto tt = Temp_newtemp();
    tpmp[11] = tt;
    StmList* ret = 0;
    auto p = this;
    int flag = 0;
    while (p) {
        p->stm->deepCopy(tpmp, lbmp, venv, ls);
        p = p->tail;
        if (flag == 0) {
            ls.push_back(new IR::Move(new Temp(tt),
                                      new Binop(binop::T_plus, new Temp(11), new Const(offset))));
            flag = 1;
        }
    }
    int len = ls.size();
    for (int i = len - 1; i >= 0; i--) { ret = new StmList(ls[i], ret); }
    return ret;
}
void Seq::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                   unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    left->deepCopy(tpmp, lbmp, venv, ls);
    right->deepCopy(tpmp, lbmp, venv, ls);
}
void Label::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    if (venv.count(label)) {
        ls.push_back(new Label(label));
        return;
    }
    if (lbmp.count(label)) {
        ls.push_back(new Label(lbmp[label]));
        return;
    }
    auto nw = Temp_newlabel();
    lbmp[label] = nw;
    ls.push_back(new Label(nw));
}
void Jump::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (venv.count(target)) {
        ls.push_back(new Jump(target));
        return;
    }
    if (lbmp.count(target)) {
        ls.push_back(new Jump(lbmp[target]));
        return;
    }
    auto nw = Temp_newlabel();
    lbmp[target] = nw;
    ls.push_back(new Jump(nw));
}
void Cjump::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    string ntl, nfl;
    if (venv.count(trueLabel))
        ntl = trueLabel;
    else if (lbmp.count(trueLabel))
        ntl = lbmp[trueLabel];
    else {
        ntl = Temp_newlabel();
        lbmp[trueLabel] = ntl;
    }
    if (venv.count(falseLabel))
        nfl = falseLabel;
    else if (lbmp.count(falseLabel))
        nfl = lbmp[falseLabel];
    else {
        nfl = Temp_newlabel();
        lbmp[falseLabel] = nfl;
    }
    ls.push_back(new Cjump(op, left->deepCopy(tpmp, lbmp, venv, ls),
                           right->deepCopy(tpmp, lbmp, venv, ls), ntl, nfl));
}
void Move::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    ls.push_back(
        new Move(dst->deepCopy(tpmp, lbmp, venv, ls), src->deepCopy(tpmp, lbmp, venv, ls)));
}
void ExpStm::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                      unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                      vector<Stm*>& ls) {
    ls.push_back(new ExpStm(exp->deepCopy(tpmp, lbmp, venv, ls)));
}
Exp* Const::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    return new Const(val);
}
Exp* Binop::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    return new Binop(op, left->deepCopy(tpmp, lbmp, venv, ls),
                     right->deepCopy(tpmp, lbmp, venv, ls));
}
Exp* Temp::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (tpmp.count(tempid)) return new Temp(tpmp[tempid]);
    if (tempid == 11 || tempid == 13) {
        if (tempid == 11) { return new Temp(tpmp[11]); }
        return new Temp(tempid);
    }
    auto nw = Temp_newtemp();
    tpmp[tempid] = nw;
    return new Temp(nw);
}
Exp* Mem::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                   unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    return new Mem(mem->deepCopy(tpmp, lbmp, venv, ls));
}
Exp* Eseq::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    assert(0);
    return 0;
}
Exp* Name::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (venv.count(name)) return new Name(name);
    if (lbmp.count(name)) return new Name(lbmp[name]);
    auto nw = Temp_newlabel();
    lbmp[name] = nw;
    return new Name(nw);
}
Exp* Call::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    vector<Exp*> nwa;
    for (auto it : args) nwa.push_back(it->deepCopy(tpmp, lbmp, venv, ls));
    return new Call(fun, nwa);
}

//
Stm* Move::dCopy() { return new Move(dst->dCopy(), src->dCopy()); }
Stm* ExpStm::dCopy() { return new ExpStm(exp->dCopy()); }
Exp* Const::dCopy() { return new Const(val); }
Exp* Binop::dCopy() { return new Binop(op, left->dCopy(), right->dCopy()); }
Exp* Temp::dCopy() { return new Temp(tempid); }
Exp* Mem::dCopy() { return new Mem(mem->dCopy()); }
Exp* Name::dCopy() { return new Name(name); }
Exp* Call::dCopy() {
    vector<Exp*> nwa;
    for (auto it : args) nwa.push_back(it->dCopy());
    return new Call(fun, nwa);
}