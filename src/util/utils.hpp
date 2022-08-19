#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
#include "../structure/ast.h"
#include "../backend/graph.hpp"
#include <memory>
#include <vector>
#include <assert.h>
#include <string>
static inline IR::Stm* nopStm() { return (new IR::ExpStm(new IR::Const(0))); }
static bool isNop(IR::Stm* x) {
    return x->kind == IR::stmType::exp
           && (static_cast<IR::ExpStm*>(x)->exp->kind == IR::expType::constx);
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
static IR::Exp* ir_i2f(IR::Exp* exp) {
    if (exp->kind == IR::expType::constx)
        return new IR::Const(digit_i2f(static_cast<IR::Const*>(exp)->val));
    else
        return new IR::Call("i2f", IR::ExpList(1, exp));
}
static IR::Exp* ir_f2i(IR::Exp* exp) {
    if (exp->kind == IR::expType::constx)
        return new IR::Const(digit_f2i(static_cast<IR::Const*>(exp)->val));
    else
        return new IR::Call("f2i", IR::ExpList(1, exp));
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
    switch (bop) {
    case IR::binop::T_plus: return new IR::Binop(IR::binop::F_plus, lexp, rexp);
    case IR::binop::T_minus: return new IR::Binop(IR::binop::F_minus, lexp, rexp);
    case IR::binop::T_mul: return new IR::Binop(IR::binop::F_mul, lexp, rexp);
    case IR::binop::T_div: return new IR::Binop(IR::binop::F_div, lexp, rexp);
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
    return (uses);
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
static bool isttmove(IR::Stm* stm) {
    return stm->kind == IR::stmType::move
           && static_cast<IR::Move*>(stm)->dst->kind == IR::expType::temp
           && static_cast<IR::Move*>(stm)->src->kind == IR::expType::temp;
}
static void cleanExpStm(IR::StmList* stmlist) {
    IR::StmList* last = nullptr;
    for (auto list = stmlist; list; list = list->tail) {
        if (list->stm->kind == IR::stmType::exp
            && static_cast<IR::ExpStm*>(list->stm)->exp->kind == IR::expType::constx) {
            last->tail = list->tail;
            continue;
        }
        last = list;
    }
}
static bool isLdr(IR::Stm* stm) {
    return stm->kind == IR::stmType::move
           && static_cast<IR::Move*>(stm)->src->kind == IR::expType::mem;
}
static bool isStr(IR::Stm* stm) {
    return stm->kind == IR::stmType::move
           && static_cast<IR::Move*>(stm)->dst->kind == IR::expType::mem;
}
static bool isCall(IR::Stm* stm) {
    return (stm->kind == IR::stmType::move
            && static_cast<IR::Move*>(stm)->src->kind == IR::expType::call)
           || (stm->kind == IR::stmType::exp
               && static_cast<IR::ExpStm*>(stm)->exp->kind == IR::expType::call);
}
static std::string getCallName(IR::Exp* callexp) {
    assert(callexp->kind == IR::expType::call);
    return static_cast<IR::Call*>(callexp)->fun;
}
static std::string getCallName(IR::Stm* callstm) {
    IR::Exp* exp = nullptr;
    if (callstm->kind == IR::stmType::exp) {
        exp = static_cast<IR::ExpStm*>(callstm)->exp;
    } else if (callstm->kind == IR::stmType::move) {
        exp = static_cast<IR::Move*>(callstm)->src;
    } else
        assert(0);
    return getCallName(exp);
}
static IR::ExpList getCallParam(IR::Exp* callexp) {
    assert(callexp->kind == IR::expType::call);
    return static_cast<IR::Call*>(callexp)->args;
}
static IR::ExpList getCallParam(IR::Stm* callstm) {
    IR::Exp* exp = nullptr;
    if (callstm->kind == IR::stmType::exp) {
        exp = static_cast<IR::ExpStm*>(callstm)->exp;
    } else if (callstm->kind == IR::stmType::move) {
        exp = static_cast<IR::Move*>(callstm)->src;
    } else
        assert(0);
    return getCallParam(exp);
}
static bool isReturn(IR::Stm* stm) {
    return stm->kind == IR::stmType::jump
           && static_cast<IR::Jump*>(stm)->target.find("RETURN3124") != -1;
}
static bool isReturn(IR::Stm* stm1, IR::Stm* stm2, IR::Stm* stm3) {
    if (stm2->kind != IR::stmType::move) return false;
    IR::Move* mv = static_cast<IR::Move*>(stm2);
    if (mv->src->kind != IR::expType::temp || mv->dst->kind != IR::expType::temp) return false;
    int dst = static_cast<IR::Temp*>(mv->dst)->tempid;
    int src = static_cast<IR::Temp*>(mv->src)->tempid;
    if (dst != 0) return false;
    if (stm1->kind != IR::stmType::move) return false;
    IR::Move* mv1 = static_cast<IR::Move*>(stm1);
    if (mv1->dst->kind != IR::expType::temp || static_cast<IR::Temp*>(mv1->dst)->tempid != src)
        return false;
    return isReturn(stm3);
}
static std::pair<int, int> exp2int(IR::Exp* x) {
    if (x->kind == IR::expType::constx) return {1, static_cast<IR::Const*>(x)->val};
    if (x->kind == IR::expType::binop) {
        auto bi = static_cast<IR::Binop*>(x);
        auto t1 = exp2int(bi->left), t2 = exp2int(bi->right);
        if (t1.first && t2.first) {
            switch (bi->op) {
            case IR::binop::T_plus: return {1, t1.second + t2.second};
            case IR::binop::T_mul: return {1, t1.second * t2.second};
            case IR::binop::T_mod: return {1, t1.second % t2.second};
            case IR::binop::T_minus: return {1, t1.second - t2.second};
            case IR::binop::T_div: return {1, t1.second / t2.second};
            default: return {0, 0};
            }
        }
    }
    return {0, 0};
}
static std::pair<int, int> exp2tempid(IR::Exp* x) {
    if (x->kind == IR::expType::temp) return {1, static_cast<IR::Temp*>(x)->tempid};
    if (x->kind == IR::expType::binop) {
        auto bi = static_cast<IR::Binop*>(x);
        auto t1 = exp2tempid(bi->left);
        auto t2 = exp2int(bi->right);
        if (t1.first && t2.first) {
            switch (bi->op) {
            case IR::binop::T_plus:
                if (t2.second == 0) return {1, t1.second};
            case IR::binop::T_mul:
                if (t2.second == 1) return {1, t1.second};
            case IR::binop::T_mod: return {0, 0};
            case IR::binop::T_minus:
                if (t2.second == 0) return {1, t1.second};
            case IR::binop::T_div:
                if (t2.second == 1) return {1, t1.second};
            default: return {0, 0};
            }
        }
        t1 = exp2tempid(bi->right);
        t2 = exp2int(bi->left);
        if (t1.first && t2.first) {
            switch (bi->op) {
            case IR::binop::T_plus:
                if (t2.second == 0) return {1, t1.second};
            case IR::binop::T_mul:
                if (t2.second == 1) return {1, t1.second};
            case IR::binop::T_mod: return {0, 0};
            case IR::binop::T_minus: return {0, 0};
            case IR::binop::T_div: return {0, 0};
            default: return {0, 0};
            }
        }
    }
    return {0, 0};
}
static bool expEqual(IR::Exp* a, IR::Exp* b) {
    if (a->kind == IR::expType::constx && b->kind == IR::expType::constx) {
        auto aa = static_cast<IR::Const*>(a), bb = static_cast<IR::Const*>(b);
        return aa->val == bb->val;
    }
    if (a->kind == IR::expType::temp && b->kind == IR::expType::temp) {
        auto aa = static_cast<IR::Temp*>(a), bb = static_cast<IR::Temp*>(b);
        return aa->tempid == bb->tempid;
    }
    if (a->kind == IR::expType::mem && b->kind == IR::expType::mem) {
        auto aa = static_cast<IR::Mem*>(a), bb = static_cast<IR::Mem*>(b);
        return expEqual(aa->mem, bb->mem);
    }
    if (a->kind == IR::expType::name && b->kind == IR::expType::name) {
        auto aa = static_cast<IR::Name*>(a), bb = static_cast<IR::Name*>(b);
        return aa->name == bb->name;
    }
    if (a->kind == IR::expType::binop && b->kind == IR::expType::binop) {
        auto aa = static_cast<IR::Binop*>(a), bb = static_cast<IR::Binop*>(b);
        if (aa->op != bb->op) return false;
        if ((aa->op == IR::binop::T_minus || aa->op == IR::binop::T_plus)
            && (expEqual(aa->left, bb->right) && expEqual(aa->right, bb->right))) {
            return true;
        }
        return (expEqual(aa->left, bb->left) && expEqual(aa->right, bb->right));
    }
    auto t1 = exp2int(a), t2 = exp2int(b);
    if (t1.first && t2.first) return t1.second == t2.second;
    t1 = exp2tempid(a), t2 = exp2tempid(b);
    if (t1.first && t2.first) return t1.second == t2.second;
    return false;
}

static std::pair<int, std::string> exp2op2(int x) {
    if (x <= 256 && x >= -128) { return {1, "#" + std::to_string(x)}; }
    return {0, std::string()};
}
static std::pair<int, std::string> exp2offset(int x) {
    if (x <= 4095 && x > 0) { return {1, "#+" + std::to_string(x)}; }
    if (x >= -4095 && x < 0) { return {1, "#-" + std::to_string(x)}; }
    return {0, std::string()};
}
static bool check2pow(int x) { return x == (-x & x); }
static std::pair<int, std::string> exp2op2(IR::Binop* bop) { assert(0); }

static string binop2string(IR::binop op) {
    switch (op) {
    case IR::binop::T_div: return "sdiv";
    case IR::binop::T_minus: return "sub";
    case IR::binop::T_mod: return "mod";
    case IR::binop::T_mul: return "mul";
    case IR::binop::T_plus: return "add";
    case IR::binop::F_div: return "vdiv.f32";
    case IR::binop::F_minus: return "vsub.f32";
    case IR::binop::F_mul: return "vmul.f32";
    case IR::binop::F_plus: return "vadd.f32";
    default: return "unknown";
    }
}

static string relop2string(IR::RelOp op) {
    switch (op) {
    case IR::RelOp::T_eq: return "eq";
    case IR::RelOp::T_ne: return "ne";
    case IR::RelOp::T_lt: return "lt";
    case IR::RelOp::T_ge: return "ge";
    case IR::RelOp::T_gt: return "gt";
    case IR::RelOp::T_le: return "le";
    case IR::RelOp::F_eq: return "eq";
    case IR::RelOp::F_ne: return "ne";
    case IR::RelOp::F_lt: return "lt";
    case IR::RelOp::F_ge: return "pl";
    case IR::RelOp::F_gt: return "hi";
    case IR::RelOp::F_le: return "le";
    default: return "unknown";
    }
}
static bool opCommutable(IR::binop bop) {
    return bop == IR::binop::T_plus || bop == IR::binop::T_mul || bop == IR::binop::F_plus
           || bop == IR::binop::F_mul;
}
#endif