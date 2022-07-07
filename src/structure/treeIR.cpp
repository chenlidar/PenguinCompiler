#include "treeIR.hpp"
#include <assert.h>
using namespace IR;
RelOp commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
    case RelOp::T_eq: return RelOp::T_eq;
    case RelOp::T_ne: return RelOp::T_ne;
    case RelOp::T_lt: return RelOp::T_gt;
    case RelOp::T_ge: return RelOp::T_le;
    case RelOp::T_gt: return RelOp::T_lt;
    case RelOp::T_le: return RelOp::T_ge;
    case RelOp::T_ult: return RelOp::T_ugt;
    case RelOp::T_uge: return RelOp::T_ule;
    case RelOp::T_ule: return RelOp::T_uge;
    case RelOp::T_ugt: return RelOp::T_ult;
    }
}
RelOp notRel(RelOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
    case RelOp::T_eq: return RelOp::T_ne;
    case RelOp::T_ne: return RelOp::T_eq;
    case RelOp::T_lt: return RelOp::T_ge;
    case RelOp::T_ge: return RelOp::T_lt;
    case RelOp::T_gt: return RelOp::T_le;
    case RelOp::T_le: return RelOp::T_gt;
    case RelOp::T_ult: return RelOp::T_uge;
    case RelOp::T_uge: return RelOp::T_ult;
    case RelOp::T_ule: return RelOp::T_ugt;
    case RelOp::T_ugt: return RelOp::T_ule;
    }
}
static void doPatch(PatchList* tList, Temp_Label label) {
    for (; tList; tList = tList->tail) tList->head = label;
}
Cx IR::unCx(Tr_Exp* e) {
    switch (e->kind) {
    case Tr_ty::Tr_cx: {
        return e->cx;
    } break;
    case Tr_ty::Tr_ex: {
        Stm* stm = new Cjump(RelOp::T_ne, e->ex, new ConstInt(0), NULL, NULL);
        Cx cx;
        cx.stm = stm;
        cx.falses = new PatchList(static_cast<Cjump*>(stm)->falseLabel, NULL);
        cx.trues = new PatchList(static_cast<Cjump*>(stm)->trueLabel, NULL);
        return cx;
    } break;
    default: assert(0);
    }
}
Exp* IR::unEx(Tr_Exp* e) {
    if (e == 0) return 0;
    switch (e->kind) {
    case Tr_ty::Tr_ex: {
        return e->ex;
    } break;
    case Tr_ty::Tr_cx: {
        Temp_Temp r = Temp_newtemp();
        Temp_Label t = Temp_newlabel(), f = Temp_newlabel();
        doPatch(e->cx.trues, t);
        doPatch(e->cx.falses, f);
        return new Eseq(
            new Move(new Temp(r), new ConstInt(1)),
            new Eseq(e->cx.stm,
                     new Eseq(new Label(f), new Eseq(new Move(new Temp(r), new ConstInt(0)),
                                                     new Eseq(new Label(t), new Temp(r))))));
    } break;
    default: assert(0);
    }
}
Stm* IR::unNx(Tr_Exp* exp) {
    switch (exp->kind) {
    case Tr_ty::Tr_ex: return new ExpStm(exp->ex);
    case Tr_ty::Tr_cx: return new ExpStm(unEx(exp));
    case Tr_ty::Tr_nx: return exp->nx;
    }
    assert(0);
}

Tr_Exp* IR::Tr_Ex(Exp* ex) {
    Tr_Exp* texp = new Tr_Exp();
    texp->kind = Tr_ty::Tr_ex;
    texp->ex = ex;
    return texp;
}

Tr_Exp* IR::Tr_Cx(PatchList* trues, PatchList* falses, Stm* stm) {
    Tr_Exp* texp = new Tr_Exp();
    texp->kind = Tr_ty::Tr_cx;
    texp->cx.trues = trues;
    texp->cx.falses = falses;
    texp->cx.stm = stm;
    return texp;
}

Tr_Exp* IR::Tr_Nx(Stm* stm) {
    Tr_Exp* tmp = new Tr_Exp();
    tmp->kind = Tr_ty::Tr_nx;
    tmp->nx = stm;
    return tmp;
}