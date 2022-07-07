#include "treeIR.hpp"
#include <assert.h>
using namespace IR;
RelOp commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
        case RelOp::T_eq:
            return RelOp::T_eq;
        case RelOp::T_ne:
            return RelOp::T_ne;
        case RelOp::T_lt:
            return RelOp::T_gt;
        case RelOp::T_ge:
            return RelOp::T_le;
        case RelOp::T_gt:
            return RelOp::T_lt;
        case RelOp::T_le:
            return RelOp::T_ge;
        case RelOp::T_ult:
            return RelOp::T_ugt;
        case RelOp::T_uge:
            return RelOp::T_ule;
        case RelOp::T_ule:
            return RelOp::T_uge;
        case RelOp::T_ugt:
            return RelOp::T_ult;
    }
}
RelOp notRel(RelOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
        case RelOp::T_eq:
            return RelOp::T_ne;
        case RelOp::T_ne:
            return RelOp::T_eq;
        case RelOp::T_lt:
            return RelOp::T_ge;
        case RelOp::T_ge:
            return RelOp::T_lt;
        case RelOp::T_gt:
            return RelOp::T_le;
        case RelOp::T_le:
            return RelOp::T_gt;
        case RelOp::T_ult:
            return RelOp::T_uge;
        case RelOp::T_uge:
            return RelOp::T_ult;
        case RelOp::T_ule:
            return RelOp::T_ugt;
        case RelOp::T_ugt:
            return RelOp::T_ule;
    }
}
static void doPatch(PatchList* tList, Temp_Label label) {
    for (; tList; tList = tList->tail)
        tList->head = label;
}
Cx IR::Tr_Exp::unCx(Tr_Exp* e) {
    switch (e->kind) {
        case Tr_ty::Tr_cx: {
            return e->cx;
        } break;
        case Tr_ty::Tr_ex: {
            Stm* stm =
                new Cjump(RelOp::T_ne, e->ex, new ConstInt(0), NULL, NULL);
            Cx cx;
            cx.stm = stm;
            cx.falses =
                new PatchList(static_cast<Cjump*>(stm)->falseLabel, NULL);
            cx.trues = new PatchList(static_cast<Cjump*>(stm)->trueLabel, NULL);
            return cx;
        } break;
        default:
            assert(0);
    }
}
Exp* IR::Tr_Exp::unEx(Tr_Exp* e) {
    if (e == 0)
        return 0;
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
                new Eseq(
                    e->cx.stm,
                    new Eseq(new Label(f),
                             new Eseq(new Move(new Temp(r), new ConstInt(0)),
                                      new Eseq(new Label(t), new Temp(r))))));
        } break;
        default:
            assert(0);
    }
}
Stm* IR::Tr_Exp::unNx(Tr_Exp* exp) {
    switch (exp->kind) {
        case Tr_ty::Tr_ex:
            return new ExpStm(exp->ex);
        case Tr_ty::Tr_cx:
            return new ExpStm(unEx(exp));
        case Tr_ty::Tr_nx:
            return exp->nx;
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