#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
#include "../structure/ast.h"
static inline IR::Stm* nopStm() { return (new IR::ExpStm(new IR::Const(0))); }
static bool isNop(IR::Stm* x) {
    return x->kind == IR::stmType::exp
           && (static_cast<IR::ExpStm*>(x)->exp->kind == IR::expType::constx
               || static_cast<IR::ExpStm*>(x)->exp->kind == IR::expType::constx);
}

static IR::Stm* seq(IR::Stm* x, IR::Stm* y) {
    if (isNop(x)) return y;
    if (isNop(y)) return x;
    return new IR::Seq(x, y);
}
static void doPatch(IR::PatchList* tList, Temp_Label label) {
    for (; tList; tList = tList->tail) *(tList->head) = label;
}
static IR::PatchList* joinPatch(IR::PatchList* first, IR::PatchList* second) {
    if (!first) return second;
    IR::PatchList* tmp = first;
    for (; tmp->tail; tmp = tmp->tail)
        ;
    tmp->tail = second;
    return first;
}
static IR::StmList* getLast(IR::StmList* list) {
    IR::StmList* last = list;
    while (last->tail->tail) last = last->tail;
    return last;
}
static IR::StmList* getEnd(IR::StmList* list) {
    IR::StmList* last = list;
    assert(list);
    while (last->tail) last = last->tail;
    return last;
}
static float decode(int i) {
    union {
        int i;
        float f;
    } ret;
    ret.i = i;
    return ret.f;
}
static int encode(float f) {
    union {
        int i;
        float f;
    } ret;
    ret.f = f;
    return ret.i;
}
static int digit_i2f(int i) { return encode((float)i); }
static int digit_f2i(int f) { return (int)decode(f); }
static TY::tyType getArrayType(TY::Type* ty) {
    while (ty->kind == TY::tyType::Ty_array) ty = ty->tp;
    return ty->kind;
}
static IR::Exp* ir_i2f(IR::Exp* exp) {
    return new IR::Call(new IR::Name("__aeabi_i2f"), IR::ExpList(1, exp));
}
static IR::Exp* ir_f2i(IR::Exp* exp) {
    return new IR::Call(new IR::Name("__aeabi_f2iz"), IR::ExpList(1, exp));
}
template <typename T> static T cal(IR::binop op, T l, T r) {
    switch (op) {
    case IR::binop::T_plus: return l + r;
    case IR::binop::T_minus: return l - r;
    case IR::binop::T_mul: return l * r;
    case IR::binop::T_div: return l / (r != (T)0 ? r : (T)1);
    default: assert(0);
    }
}
static TY::Type* binopResType(const TY::Type* a, const TY::Type* b, IR::binop op) {
    if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
        int l = *a->value;
        int r = *b->value;
        if(op==IR::binop::T_mod)return TY::intType(new int(l%(r!=0?r:1)), false);
        else return TY::intType(new int(cal(op, l, r)), false);
    } else if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_float) {
        float l = (float)*a->value;
        float r = decode(*b->value);
        return TY::floatType(new int(encode(cal(op, l, r))), false);
    } else if (a->kind == TY::tyType::Ty_float && b->kind == TY::tyType::Ty_int) {
        float l = decode(*a->value);
        float r = (float)*b->value;
        return TY::floatType(new int(encode(cal(op, l, r))), false);
    } else if (a->kind == TY::tyType::Ty_float && b->kind == TY::tyType::Ty_float) {
        float l = decode(*a->value);
        float r = decode(*b->value);
        return TY::floatType(new int(encode(cal(op, l, r))), false);
    } else
        assert(0);
    return nullptr;
}
static TY::Type* typeAst2ir(AST::btype_t btype) {
    switch (btype) {
    case AST::btype_t::INT: return TY::intType(new int(0), false);
    case AST::btype_t::FLOAT: return TY::floatType(new int(0), false);
    case AST::btype_t::VOID: return TY::voidType();
    default: assert(0);
    }
    return nullptr;
}
static IR::Exp* TyIRAssign(IR::Exp* rexp, TY::tyType lty, TY::tyType rty) {
    if (lty == TY::tyType::Ty_float && rty == TY::tyType::Ty_int) {
        return ir_i2f(rexp);
    } else if (lty == TY::tyType::Ty_int && rty == TY::tyType::Ty_float) {
        return ir_f2i(rexp);
    } else
        return rexp;
}
static IR::Exp* calIRfloat(IR::binop bop,IR::Exp* lexp, IR::Exp* rexp){
    IR::ExpList param;
    param.push_back(lexp);
    param.push_back(rexp);
    switch(bop){
        case IR::binop::T_plus:return new IR::Call(new IR::Name("__aeabi_fadd"),param);
        case IR::binop::T_minus:return new IR::Call(new IR::Name("__aeabi_fsub"),param);
        case IR::binop::T_mul:return new IR::Call(new IR::Name("__aeabi_fmul"),param);
        case IR::binop::T_div:return new IR::Call(new IR::Name("__aeabi_fdiv"),param);
        default:assert(0);
    }
    return nullptr;
}
static IR::Exp* TyIRBinop(IR::binop bop, TY::tyType lty, IR::Exp* lexp, TY::tyType rty,
                          IR::Exp* rexp) {
    if (lty == TY::tyType::Ty_int && rty == TY::tyType::Ty_int) {
        return new IR::Binop(bop, lexp, rexp);
    } else if (lty == TY::tyType::Ty_int && rty == TY::tyType::Ty_float) {
        lexp=ir_i2f(lexp);
        return calIRfloat(bop,lexp,rexp);
    } else if (lty == TY::tyType::Ty_float && rty == TY::tyType::Ty_int) {
        rexp=ir_i2f(rexp);
        return calIRfloat(bop,lexp,rexp);
    } else if (lty == TY::tyType::Ty_float && rty == TY::tyType::Ty_float) {
        return calIRfloat(bop,lexp,rexp);
    } else
        assert(0);
}
#endif