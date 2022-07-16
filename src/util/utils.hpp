#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
#include "../structure/ast.h"
static inline IR::Stm* nopStm() { return (new IR::ExpStm(new IR::ConstInt(0))); }
static bool isNop(IR::Stm* x) {
    return x->kind == IR::stmType::exp
           && (static_cast<IR::ExpStm*>(x)->exp->kind == IR::expType::constint
               || static_cast<IR::ExpStm*>(x)->exp->kind == IR::expType::constfloat);
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
static TY::Type* binopResType(const TY::Type* a, const TY::Type* b, IR::binop op) {
    assert(a->kind == TY::tyType::Ty_float || a->kind == TY::tyType::Ty_int);
    assert(b->kind == TY::tyType::Ty_float || b->kind == TY::tyType::Ty_int);
    switch (op) {
    case IR::binop::T_plus: {
        if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
            if (a->value && b->value) return TY::intType(new int(*a->value + *b->value), false);
        } else
            assert(0);  // FIXME
    } break;
    case IR::binop::T_minus: {
        if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
            if (a->value && b->value) return TY::intType(new int(*a->value - *b->value), false);
        } else
            assert(0);  // FIXME
    } break;
    case IR::binop::T_mul: {
        if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
            if (a->value && b->value) return TY::intType(new int(*a->value * *b->value), false);
        } else
            assert(0);  // FIXME
    } break;
    case IR::binop::T_div: {
        if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
            if (a->value && b->value)
                return TY::intType(new int(*a->value / (*b->value ? *b->value : 1)), false);
        } else
            assert(0);  // FIXME
    } break;
    case IR::binop::T_mod: {
        assert(a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int);
        if (a->value && b->value)
            return TY::intType(new int(*a->value % (*b->value ? *b->value : 1)), false);
    } break;
    default: assert(0);
    }
    assert(0);
    return NULL;
}
static TY::Type* typeAst2ir(AST::btype_t btype) {
    switch (btype) {
    case AST::btype_t::INT: {
        return TY::intType(new int(0), false);
    } break;
    case AST::btype_t::FLOAT: {
        return TY::floatType(new int(0), false);
    } break;
    case AST::btype_t::VOID: {
        return TY::voidType();
    } break;
    default: assert(0);
    }
}
#endif