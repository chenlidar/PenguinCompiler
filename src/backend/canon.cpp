#include "canon.hpp"
#include <assert.h>
#include <list>
#include <typeinfo>
#include <vector>
#include "src/util/utils.hpp"
using namespace IR;
using namespace CANON;
struct ExpRefList {
    Exp** head;
    ExpRefList* tail;
    ExpRefList(Exp** _head, ExpRefList* _tail) : head(_head), tail(_tail) {}
};
static StmList* linear(Stm* stm, StmList* right) {
    if (typeid(*stm) == typeid(Seq)) {
        Seq* s = static_cast<Seq*>(stm);
        return linear(s->left, linear(s->right, right));
    } else {
        StmList* l = new StmList();
        l->push_back(stm);
        l->splice(l->end(), *right);
        return l;
    }
}
static bool commute(Stm* x, Exp* y) {
    if (isNop(x))
        return true;
    if (typeid(*y) == typeid(Name) || typeid(*y) == typeid(ConstInt) ||
        typeid(*y) == typeid(ConstFloat))
        return true;
    return false;
}
struct StmExp {
    Stm* stm;
    Exp* exp;
    StmExp(Stm* _stm, Exp* _exp) : stm(_stm), exp(_exp) {}
};
static Stm* reorder(ExpRefList* rlist) {
    if (!rlist)
        return new ExpStm(new ConstInt(0)); /* nop */
    else if (typeid(**rlist->head) == typeid(Call)) {
        Temp_temp t = Temp_newtemp();
        *rlist->head =
            new Eseq(new Move(new Temp(t), *rlist->head), new Temp(t));
        return reorder(rlist);
    } else {
        StmExp hd = do_exp(*rlist->head);
        Stm* s = reorder(rlist->tail);
        if (commute(s, hd.exp)) {
            *rlist->head = hd.exp;
            return seq(hd.stm, s);
        } else {
            Temp_temp t = Temp_newtemp();
            *rlist->head = new Temp(t);
            return seq(hd.stm, seq(new Move(new Temp(t), hd.exp), s));
        }
    }
}
static ExpRefList* get_call_rlist(Exp* exp) {
    ExpRefList *rlist, *curr;
    ExpList args = static_cast<Call*>(exp)->args;
    curr = rlist = new ExpRefList(&static_cast<Call*>(exp)->fun, NULL);
    for (auto arg : args) {
        curr = curr->tail = new ExpRefList(&arg, NULL);
    }
    return rlist;
}
static StmExp do_exp(Exp* exp) {
    if (typeid(*exp) == typeid(Binop)) {
        return StmExp(
            reorder(new ExpRefList(
                &static_cast<Binop*>(exp)->left,
                new ExpRefList(&static_cast<Binop*>(exp)->right, NULL))),
            exp);
    } else if (typeid(*exp) == typeid(Mem)) {
        return StmExp(
            reorder(new ExpRefList(&static_cast<Mem*>(exp)->mem, NULL)), exp);
    } else if (typeid(*exp) == typeid(Eseq)) {
        StmExp x = do_exp(static_cast<Eseq*>(exp)->exp);
        return StmExp(seq(do_stm(static_cast<Eseq*>(exp)->stm), x.stm), x.exp);
    } else if (typeid(*exp) == typeid(Call)) {
        return StmExp(reorder(get_call_rlist(exp)), exp);
    } else
        return StmExp(reorder(NULL), exp);
}
static Stm* do_stm(Stm* stm) {
    if (typeid(*stm) == typeid(Seq)) {
        Seq* s = static_cast<Seq*>(stm);
        return seq(do_stm(s->left), do_stm(s->right));
    } else if (typeid(*stm) == typeid(Jump)) {
        Jump* s = static_cast<Jump*>(stm);
        return seq(reorder(new ExpRefList(&s->exp, NULL)), stm);
    } else if (typeid(*stm) == typeid(Cjump)) {
        Cjump* s = static_cast<Cjump*>(stm);
        return seq(
            reorder(new ExpRefList(&s->left, new ExpRefList(&s->right, NULL))),
            stm);
    } else if (typeid(*stm) == typeid(Move)) {
        Move* s = static_cast<Move*>(stm);
        if (typeid(*(s->dst)) == typeid(Temp) &&
            typeid(*(s->src)) == typeid(Call))
            return seq(reorder(get_call_rlist(s->src)), stm);
        else if (typeid(*s->dst) == typeid(Temp))
            return seq(reorder(new ExpRefList(&s->src, NULL)), stm);
        else if (typeid(*s->dst) == typeid(Mem))
            return seq(reorder(new ExpRefList(&static_cast<Mem*>(s->dst)->mem,
                                              new ExpRefList(&s->src, NULL))),
                       stm);
        else if (typeid(*s->dst) == typeid(Eseq)) {
            Stm* ss = static_cast<Eseq*>(s->dst)->stm;
            s->dst = static_cast<Eseq*>(s->dst)->exp;
            return do_stm(new Seq(ss, stm));
        } else
            assert(0);
    } else if (typeid(*stm) == typeid(ExpStm)) {
        if (typeid(static_cast<ExpStm*>(stm)) == typeid(Call))
            return seq(reorder(get_call_rlist(static_cast<ExpStm*>(stm)->exp)),
                       stm);
        else
            return seq(
                reorder(new ExpRefList(&static_cast<ExpStm*>(stm)->exp, NULL)),
                stm);
    } else
        return stm;
}
StmList* linearize(Stm* stm) {
    return linear(do_stm(stm), NULL);
}