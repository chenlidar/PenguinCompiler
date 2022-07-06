#include "canon.hpp"
#include <assert.h>
#include <list>
#include <typeinfo>
#include <vector>
#include "../util/utils.hpp"
using namespace IR;
using namespace CANON;
/**
 * Linear
 */
struct ExpRefList {
    Exp** head;
    ExpRefList* tail;
    ExpRefList(Exp** _head, ExpRefList* _tail)
        : head(_head)
        , tail(_tail) {}
};
static StmList* linear(Stm* stm, StmList* right) {
    if (typeid(*stm) == typeid(Seq)) {
        Seq* s = static_cast<Seq*>(stm);
        return linear(s->left, linear(s->right, right));
    } else {
        return new StmList(stm, right);
    }
}
static bool commute(Stm* x, Exp* y) {
    if (isNop(x)) return true;
    if (typeid(*y) == typeid(Name) || typeid(*y) == typeid(ConstInt)
        || typeid(*y) == typeid(ConstFloat))
        return true;
    return false;
}

static Stm* reorder(ExpRefList* rlist) {
    if (!rlist)
        return new ExpStm(new ConstInt(0)); /* nop */
    else if (typeid(**rlist->head) == typeid(Call)) {
        Temp_Temp t = Temp_newtemp();
        *rlist->head = new Eseq(new Move(new Temp(t), *rlist->head), new Temp(t));
        return reorder(rlist);
    } else {
        StmExp hd = do_exp(*rlist->head);
        Stm* s = reorder(rlist->tail);
        if (commute(s, hd.exp)) {
            *rlist->head = hd.exp;
            return seq(hd.stm, s);
        } else {
            Temp_Temp t = Temp_newtemp();
            *rlist->head = new Temp(t);
            return seq(hd.stm, seq(new Move(new Temp(t), hd.exp), s));
        }
    }
}
static ExpRefList* get_call_rlist(Exp* exp) {
    ExpRefList *rlist, *curr;
    ExpList args = static_cast<Call*>(exp)->args;
    curr = rlist = new ExpRefList(&static_cast<Call*>(exp)->fun, NULL);
    for (auto arg : args) { curr = curr->tail = new ExpRefList(&arg, NULL); }
    return rlist;
}
static StmExp do_exp(Exp* exp) {
    if (typeid(*exp) == typeid(Binop)) {
        return StmExp(
            reorder(new ExpRefList(&static_cast<Binop*>(exp)->left,
                                   new ExpRefList(&static_cast<Binop*>(exp)->right, NULL))),
            exp);
    } else if (typeid(*exp) == typeid(Mem)) {
        return StmExp(reorder(new ExpRefList(&static_cast<Mem*>(exp)->mem, NULL)), exp);
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
        return seq(reorder(new ExpRefList(&s->left, new ExpRefList(&s->right, NULL))), stm);
    } else if (typeid(*stm) == typeid(Move)) {
        Move* s = static_cast<Move*>(stm);
        if (typeid(*(s->dst)) == typeid(Temp) && typeid(*(s->src)) == typeid(Call))
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
            return seq(reorder(get_call_rlist(static_cast<ExpStm*>(stm)->exp)), stm);
        else
            return seq(reorder(new ExpRefList(&static_cast<ExpStm*>(stm)->exp, NULL)), stm);
    } else
        return stm;
}
StmList* linearize(Stm* stm) { return linear(do_stm(stm), NULL); }

/**
 * Basic Block
 */

static StmListList* next(StmList* prevstms, StmList* stms, Temp_Label done) {
    if (!stms)
        return next(prevstms, new StmList(new Jump(new Name(done), Temp_LabelList(1, done)), NULL),
                    done);
    if (typeid(*stms->stm) == typeid(Jump) || typeid(*stms->stm) == typeid(Cjump)) {
        StmListList* stmLists;
        prevstms->tail = stms;
        stmLists = mkBlocks(stms->tail, done);
        stms->tail = NULL;
        return stmLists;
    } else if (typeid(*stms->stm) == typeid(Label)) {
        Temp_Label lab = static_cast<Label*>(stms->stm)->label;
        return next(prevstms, new StmList(new Jump(new Name(lab), Temp_LabelList(1, lab)), stms),
                    done);
    } else {
        prevstms->tail = stms;
        return next(stms, stms->tail, done);
    }
}

/* Create the beginning of a basic block */
static StmListList* mkBlocks(StmList* stms, Temp_Label done) {
    if (!stms) { return NULL; }
    if (typeid(*stms->stm) != typeid(Label)) {
        return mkBlocks(new StmList(new Label(Temp_newlabel()), stms), done);
    }
    /* else there already is a label */
    return new StmListList(stms, next(stms, stms->tail, done));
}

Block basicBlocks(StmList* stmList) {
    Block b;
    b.label = Temp_newlabel();
    b.llist = mkBlocks(stmList, b.label);
    return b;
}
/**
 * Trace Block
 */
// static StmList* getLast(StmList* list) {
//     StmList* last = list;
//     while (last->tail->tail) last = last->tail;
//     return last;
// }

// static void trace(StmList* list) {
//     StmList* last = getLast(list);
//     Stm* lab = list->stm;
//     Stm* s = last->tail->stm;
//     S_enter(block_env, lab->u.LABEL, NULL);
//     if (typeid(*s) == typeid(Jump)) {
//         StmList* target = (StmList*)S_look(block_env, s->u.JUMP.jumps->head);
//         if (static_cast<Jump*>(s)->jumps.size()==1 && target) {
//             last->tail = target; /* merge the 2 lists removing JUMP stm */
//             trace(target);
//         } else
//             last->tail->tail = getNext(); /* merge and keep JUMP stm */
//     }
//     /* we want false label to follow CJUMP */
//     else if (typeid(*s) == typeid(Cjump)) {
//         StmList* truestm = (StmList*)S_look(block_env, s->u.CJUMP.truelabel);
//         StmList* falsestm = (StmList*)S_look(block_env, s->u.CJUMP.falselabel);
//         if (falsestm) {
//             last->tail->tail = falsestm;
//             trace(falsestm);
//         } else if (truestm) { /* convert so that existing label is a false label */
//             last->tail->stm
//                 = new Cjump(IR::notRel(static_cast<Cjump*>(s)->op), static_cast<Cjump*>(s)->left,
//                             static_cast<Cjump*>(s)->right, static_cast<Cjump*>(s)->falseLabel,
//                             static_cast<Cjump*>(s)->trueLabel);
//             last->tail->tail = truestm;
//             trace(truestm);
//         } else {
//             Temp_Label falselabel = Temp_newlabel();
//             last->tail->stm
//                 = new Cjump(static_cast<Cjump*>(s)->op, static_cast<Cjump*>(s)->left,
//                             static_cast<Cjump*>(s)->right, static_cast<Cjump*>(s)->trueLabel,
//                             static_cast<Cjump*>(s)->falseLabel);
//             last->tail->tail = new StmList(new Label(falselabel), getNext());
//         }
//     } else
//         assert(0);
// }

// /* get the next block from the list of stmLists, using only those that have
//  * not been traced yet */
// static StmList* getNext() {
//     if (!global_block.stmLists)
//         return new StmList(new Label(global_block.label), NULL);
//     else {
//         StmList* s = global_block.stmLists->head;
//         if (S_look(block_env, s->stm->u.LABEL)) { /* label exists in the table */
//             trace(s);
//             return s;
//         } else {
//             global_block.stmLists = global_block.stmLists->tail;
//             return getNext();
//         }
//     }
// }
// StmList* traceSchedule(Block b) {
//     StmListList* sList;
//     block_env = S_empty();
//     global_block = b;

//     for (sList = global_block.stmLists; sList; sList = sList->tail) {
//         S_enter(block_env, sList->head->stm->u.LABEL, sList->head);
//     }

//     return getNext();
// }