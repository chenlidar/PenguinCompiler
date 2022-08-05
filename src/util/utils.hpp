#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
#include "../structure/ast.h"
#include "../backend/graph.hpp"
#include <memory>
#include <vector>
#include <assert.h>
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
    assert(last->tail);
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
static IR::Exp* ir_i2f(IR::Exp* exp) { return new IR::Call("__aeabi_i2f", IR::ExpList(1, exp)); }
static IR::Exp* ir_f2i(IR::Exp* exp) { return new IR::Call("__aeabi_f2iz", IR::ExpList(1, exp)); }
template <typename T> static T cal(IR::binop op, T l, T r) {
    switch (op) {
    case IR::binop::T_plus: return l + r;
    case IR::binop::T_minus: return l - r;
    case IR::binop::T_mul: return l * r;
    case IR::binop::T_div: return l / (r != (T)0 ? r : (T)1);
    default: assert(0);
    }
}
static TY::Type* binopResType(TY::Type* a, TY::Type* b, IR::binop op) {
    if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_int) {
        int l = *a->value;
        int r = *b->value;
        if (op == IR::binop::T_mod)
            return TY::intType(new int(l % (r != 0 ? r : 1)), false);
        else
            return TY::intType(new int(cal(op, l, r)), false);
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
    } else if (a->kind == TY::tyType::Ty_int && b->kind == TY::tyType::Ty_array) {
        return b;
    } else if (a->kind == TY::tyType::Ty_array && b->kind == TY::tyType::Ty_int) {
        return a;
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
static IR::Exp* calIRfloat(IR::binop bop, IR::Exp* lexp, IR::Exp* rexp) {
    IR::ExpList param;
    param.push_back(lexp);
    param.push_back(rexp);
    switch (bop) {
    case IR::binop::T_plus: return new IR::Call("__aeabi_fadd", param);
    case IR::binop::T_minus: return new IR::Call("__aeabi_fsub", param);
    case IR::binop::T_mul: return new IR::Call("__aeabi_fmul", param);
    case IR::binop::T_div: return new IR::Call("__aeabi_fdiv", param);
    default: assert(0);
    }
    return nullptr;
}
static IR::Exp* TyIRBinop(IR::binop bop, TY::Type* lty, IR::Exp* lexp, TY::Type* rty,
                          IR::Exp* rexp) {
    if (lty->kind == TY::tyType::Ty_int && rty->kind == TY::tyType::Ty_int) {
        return new IR::Binop(bop, lexp, rexp);
    } else if (lty->kind == TY::tyType::Ty_int && rty->kind == TY::tyType::Ty_float) {
        lexp = ir_i2f(lexp);
        return calIRfloat(bop, lexp, rexp);
    } else if (lty->kind == TY::tyType::Ty_float && rty->kind == TY::tyType::Ty_int) {
        rexp = ir_i2f(rexp);
        return calIRfloat(bop, lexp, rexp);
    } else if (lty->kind == TY::tyType::Ty_float && rty->kind == TY::tyType::Ty_float) {
        return calIRfloat(bop, lexp, rexp);
    } else if (lty->kind == TY::tyType::Ty_int && rty->kind == TY::tyType::Ty_array) {
        assert(bop == IR::binop::T_plus);
        return new IR::Binop(
            IR::binop::T_plus, rexp,
            new IR::Binop(IR::binop::T_mul, lexp, new IR::Const(rty->tp->arraysize * 4)));
    } else if (lty->kind == TY::tyType::Ty_array && rty->kind == TY::tyType::Ty_int) {
        assert(bop == IR::binop::T_plus || bop == IR::binop::T_minus);
        return new IR::Binop(
            bop, lexp,
            new IR::Binop(IR::binop::T_mul, rexp, new IR::Const(lty->tp->arraysize * 4)));
    } else
        assert(0);
}
static inline bool isRealregister(Temp_Temp temp) {
    return temp < 1000;  // according to temptemp.hpp
}
static IR::Exp** getDef(IR::Stm* stm) {  // return 0 means no def,else return the def temp_temp
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movstm = static_cast<IR::Move*>(stm);
        if (movstm->dst->kind == IR::expType::temp) {
            auto tempexp = static_cast<IR::Temp*>(movstm->dst);
            if (isRealregister(tempexp->tempid)) return 0;
            return &(movstm->dst);
        } else
            return 0;
    }
    case IR::stmType::exp: return 0;
    case IR::stmType::cjump: return 0;
    case IR::stmType::label: return 0;
    case IR::stmType::seq: assert(0);
    case IR::stmType::jump: return 0;
    default: assert(0);
    }
    assert(0);
    return 0;
}
static std::vector<IR::Exp**> getUses(IR::Stm* stm) {
    std::vector<IR::Exp**> uses;
    auto processTempExp = [&](IR::Exp** exp) {
        if ((*exp)->kind != IR::expType::temp) return;
        auto tempexp = static_cast<IR::Temp*>(*exp);
        if (isRealregister(tempexp->tempid)) return;
        uses.push_back(exp);
    };
    auto processExpInMove = [&](IR::Exp*& exp) {
        switch (exp->kind) {
        case IR::expType::binop: {
            auto binopexp = static_cast<IR::Binop*>(exp);
            processTempExp(&binopexp->left);
            processTempExp(&binopexp->right);
            break;
        }
        case IR::expType::call: {
            auto callexp = static_cast<IR::Call*>(exp);
            for (auto& it : (callexp->args)) processTempExp(&it);
            break;
        }
        case IR::expType::constx: break;
        case IR::expType::eseq: assert(0);
        case IR::expType::mem: {
            auto memexp = static_cast<IR::Mem*>(exp);
            processTempExp(&memexp->mem);
            break;
        }
        case IR::expType::name: break;
        case IR::expType::temp: {
            processTempExp(&exp);
            break;
        }
        }
    };

    switch (stm->kind) {
    case IR::stmType::move: {
        auto movstm = static_cast<IR::Move*>(stm);
        processExpInMove(movstm->src);
        if (movstm->dst->kind == IR::expType::mem) {
            auto memexp = static_cast<IR::Mem*>(movstm->dst);
            processTempExp(&(memexp->mem));
        }
        break;
    }
    case IR::stmType::exp: {
        auto expstm = static_cast<IR::ExpStm*>(stm);
        if (expstm->exp->kind == IR::expType::call) {
            auto callexp = static_cast<IR::Call*>(expstm->exp);
            for (auto& it : (callexp->args)) { processTempExp(&it); }
        }
        break;
    }
    case IR::stmType::cjump: {
        auto cjumpstm = static_cast<IR::Cjump*>(stm);
        processTempExp(&cjumpstm->left);
        processTempExp(&cjumpstm->right);
        break;
    }
    case IR::stmType::label: break;
    case IR::stmType::seq: assert(0);
    case IR::stmType::jump: break;
    default: assert(0);
    }
    return std::move(uses);
}

static Temp_Label getNodeLabel(GRAPH::Node* node) {
    return static_cast<IR::Label*>(((IR::StmList*)(node->info))->stm)->label;
}
static IR::Label* getNodeLabelStm(GRAPH::Node* node) {
    return static_cast<IR::Label*>(((IR::StmList*)(node->info))->stm);
}
static bool isphifunc(IR::Stm* stm) {
    return stm->kind == IR::stmType::move
           && static_cast<IR::Move*>(stm)->src->kind == IR::expType::call
           && static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->fun[0] == '$';
}
static bool ismovebi(IR::Stm* stm) {
    return stm->kind == IR::stmType::move
           && static_cast<IR::Move*>(stm)->src->kind == IR::expType::binop;
}
#endif