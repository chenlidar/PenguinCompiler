#include "canon.hpp"
#include <assert.h>
#include <list>
#include <typeinfo>
#include <vector>
#include "../util/utils.hpp"
#include <unordered_map>
#include <queue>
using namespace IR;
using namespace CANON;
/**
 * Linear
 */
static StmExp do_exp(Exp* exp);
static Stm* do_stm(Stm* stm);
struct ExpRefList {
    Exp** head;
    ExpRefList* tail;
    ExpRefList(Exp** _head, ExpRefList* _tail)
        : head(_head)
        , tail(_tail) {}
};
static bool commute(Stm* x, Exp* y) {
    if (isNop(x)) return true;
    if (y->kind == expType::name || y->kind == expType::constx) return true;
    return false;
}

static Stm* reorder(ExpRefList* rlist) {
    if (!rlist)
        return new ExpStm(new Const(0)); /* nop */
    else if ((*rlist->head)->kind == expType::call) {
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
    ExpList& args = static_cast<Call*>(exp)->args;
    curr = rlist = new ExpRefList(NULL, NULL);
    for (auto& arg : args) { curr = curr->tail = new ExpRefList(&arg, NULL); }
    return rlist->tail;
}
static StmExp do_exp(Exp* exp) {
    if (exp->kind == expType::binop) {
        return StmExp(
            reorder(new ExpRefList(&static_cast<Binop*>(exp)->left,
                                   new ExpRefList(&static_cast<Binop*>(exp)->right, NULL))),
            exp);
    } else if (exp->kind == expType::mem) {
        return StmExp(reorder(new ExpRefList(&static_cast<Mem*>(exp)->mem, NULL)), exp);
    } else if (exp->kind == expType::eseq) {
        StmExp x = do_exp(static_cast<Eseq*>(exp)->exp);
        return StmExp(seq(do_stm(static_cast<Eseq*>(exp)->stm), x.stm), x.exp);
    } else if (exp->kind == expType::call) {
        return StmExp(reorder(get_call_rlist(exp)), exp);
    } else
        return StmExp(reorder(NULL), exp);
}
static Stm* do_stm(Stm* stm) {
    if (stm->kind == stmType::seq) {
        Seq* s = static_cast<Seq*>(stm);
        return seq(do_stm(s->left), do_stm(s->right));
    } else if (stm->kind == stmType::jump) {
        return stm;
    } else if (stm->kind == stmType::cjump) {
        Cjump* s = static_cast<Cjump*>(stm);
        return seq(reorder(new ExpRefList(&s->left, new ExpRefList(&s->right, NULL))), stm);
    } else if (stm->kind == stmType::move) {
        Move* s = static_cast<Move*>(stm);
        if ((s->dst->kind == expType::temp || s->dst->kind == expType::name)
            && s->src->kind == expType::call)
            return seq(reorder(get_call_rlist(s->src)), stm);
        else if (s->dst->kind == expType::temp || s->dst->kind == expType::name)
            return seq(reorder(new ExpRefList(&s->src, NULL)), stm);
        else if (s->dst->kind == expType::mem)
            return seq(reorder(new ExpRefList(&static_cast<Mem*>(s->dst)->mem,
                                              new ExpRefList(&s->src, NULL))),
                       stm);
        else if (s->dst->kind == expType::eseq) {
            Stm* ss = static_cast<Eseq*>(s->dst)->stm;
            s->dst = static_cast<Eseq*>(s->dst)->exp;
            return do_stm(new Seq(ss, stm));
        } else
            assert(0);
    } else if (stm->kind == stmType::exp) {
        if (static_cast<ExpStm*>(stm)->exp->kind == expType::call)
            return seq(reorder(get_call_rlist(static_cast<ExpStm*>(stm)->exp)), stm);
        else
            return seq(reorder(new ExpRefList(&static_cast<ExpStm*>(stm)->exp, NULL)), stm);
    } else
        return stm;
}
StmList* CANON::linearize(Stm* stm) {
    // return linear(do_stm(stm), NULL);
    std::vector<Stm*> res, st;
    st.push_back(do_stm(stm));
    while (!st.empty()) {
        auto tp = st.back();
        st.pop_back();
        if (tp->kind == stmType::seq) {
            Seq* s = static_cast<Seq*>(tp);
            st.push_back(s->right);
            st.push_back(s->left);
        } else {
            res.push_back(tp);
        }
    }
    int len = res.size();
    if (len == 0) return 0;
    auto it = new StmList(res.back(), 0);
    for (int i = len - 2; i >= 0; i--) { it = new StmList(res[i], it); }
    return it;
}

/**
 * Basic Block
 */

static StmListList* mkBlocks(StmList* stms, Temp_Label done) {
    assert(stms && stms->stm->kind == stmType::label);
    StmList *head = new StmList(nullptr, nullptr), *tail;  // accumulate list
    tail = head;
    StmListList *lhead = new StmListList(nullptr, nullptr), *ltail;
    ltail = lhead;
    const int IN = 1;
    const int OUT = 2;
    int state = OUT;
    while (stms) {
        IR::Stm* stm = stms->stm;
        switch (state) {
        case IN: {
            if (stm->kind == IR::stmType::label) {  // only add jump,dont add stm;
                state = OUT;
                std::string lab = static_cast<Label*>(stm)->label;
                tail = tail->tail = new StmList(new Jump(lab), nullptr);
                ltail = ltail->tail = new StmListList(head->tail, nullptr);
            } else {
                // state=IN, continue
                tail = tail->tail = new StmList(stm, nullptr);
                stms = stms->tail;
                if (stm->kind == IR::stmType::jump || stm->kind == IR::stmType::cjump) {
                    state = OUT;
                    ltail = ltail->tail = new StmListList(head->tail, nullptr);
                }
            }
        } break;
        case OUT: {
            if (stm->kind == IR::stmType::label) {  // begin block
                state = IN;
                head->tail = nullptr;
                tail = head;
                tail = tail->tail = new StmList(stm, nullptr);
                stms = stms->tail;
            } else {  // no label,only add label,dont add stm
                state = IN;
                head->tail = nullptr;
                tail = head;
                tail = tail->tail = new StmList(new Label(Temp_newlabel()), nullptr);  // add label
            }
        } break;
        default: assert(0);
        }
    }
    if (state == IN) {
        state = OUT;
        tail = tail->tail = new StmList(new Jump(done), nullptr);
        ltail = ltail->tail = new StmListList(head->tail, nullptr);
    }
    return lhead->tail;
}

Block CANON::basicBlocks(StmList* stmList, std::string funcname) {
    Block b;
    b.label = funcname + "_RETURN3124";
    b.llist = mkBlocks(stmList, b.label);
    return b;
}
/**
 * Trace Block
 */

StmList* CANON::traceSchedule(Block b) {
    StmListList* sList;
    std::unordered_map<std::string, StmList*> block_env;
    for (sList = b.llist; sList; sList = sList->tail) {
        Temp_Label label = static_cast<IR::Label*>(sList->head->stm)->label;
        block_env.insert(std::make_pair(label, sList->head));
    }
    StmList *head = new StmList(nullptr, new StmList(nullptr, nullptr)), *tail, *last;
    last = head;
    tail = last->tail;
    for (sList = b.llist; sList; sList = sList->tail) {
        IR::StmList* block = sList->head;
        IR::Stm* labelstm = block->stm;
        assert(labelstm->kind == IR::stmType::label);
        std::string name = static_cast<IR::Label*>(labelstm)->label;
        if (block_env.count(name) == 0) continue;
        block_env.erase(name);
        tail->tail = block;
        last = getLast(tail);
        tail = last->tail;
        IR::Stm* jump = tail->stm;
        while (1) {
            if (jump->kind == IR::stmType::jump) {
                std::string nextlabel = static_cast<IR::Jump*>(jump)->target;
                if (block_env.count(nextlabel) == 0) break;
                block = block_env.at(nextlabel);
                block_env.erase(nextlabel);
                last->tail = block;  // remove jump ,dont remove next label
                last = getLast(last);
                tail = last->tail;
                jump = tail->stm;
            } else if (jump->kind == IR::stmType::cjump) {
                std::string tl = static_cast<IR::Cjump*>(jump)->trueLabel;
                std::string fl = static_cast<IR::Cjump*>(jump)->falseLabel;
                if (block_env.count(fl)) {
                    block = block_env.at(fl);
                    block_env.erase(fl);
                    tail->tail = block;
                    last = getLast(last);
                    tail = last->tail;
                    jump = tail->stm;
                } else if (block_env.count(tl)) {
                    static_cast<IR::Cjump*>(jump)->trueLabel = fl;
                    static_cast<IR::Cjump*>(jump)->falseLabel = tl;
                    IR::RelOp op = static_cast<IR::Cjump*>(jump)->op;
                    static_cast<IR::Cjump*>(jump)->op = IR::notRel(op);

                    block = block_env.at(tl);
                    block_env.erase(tl);
                    tail->tail = block;
                    last = getLast(last);
                    tail = last->tail;
                    jump = tail->stm;
                } else {
                    Temp_Label falselabel = Temp_newlabel();
                    static_cast<IR::Cjump*>(jump)->falseLabel = falselabel;
                    tail->tail = new StmList(new IR::Label(falselabel), nullptr);
                    last = tail;
                    tail = last->tail;
                    tail->tail = new StmList(new IR::Jump(fl), nullptr);
                    last = tail;
                    tail = last->tail;
                    break;
                }
            } else
                assert(0);
        }
    }
    tail->tail = new StmList(new Label(b.label), nullptr);
    return head->tail->tail;
}

//
StmList* CANON::funcEntryExit1(StmList* stmlist) {
    // assert(stmlist);
    // assert(stmlist->stm->kind == IR::stmType::label);
    // StmList *end = stmlist, *end_entry;
    // for (; end->tail; end = end->tail)
    //     ;
    // assert(end->stm->kind == IR::stmType::label);
    // assert(end != stmlist);

    // StmList *entry = new StmList(NULL, NULL), *exit = new StmList(NULL, NULL);
    // for (int i = 4; i <= 10; i++) {
    //     Temp_Temp temp = Temp_newtemp();
    //     entry->tail = new StmList(new Move(new Temp(temp), new Temp(i)), entry->tail);
    //     exit->tail = new StmList(new Move(new Temp(i), new Temp(temp)), exit->tail);
    // }
    // end_entry = entry;
    // for (; end_entry->tail; end_entry = end_entry->tail)
    //     ;
    // end_entry->tail = stmlist->tail;
    // stmlist->tail = entry->tail;
    // end->tail = exit->tail;
    // assert(stmlist->stm->kind == IR::stmType::label);
    return stmlist;
}
ASM::InstrList* CANON::funcEntryExit2(ASM::InstrList* list, bool isvoid, bool ismain) {
    if (!isvoid) {
        list->push_back(
            new ASM::Oper("", Temp_TempList(), Temp_TempList(1, 0), Temp_LabelList()));  // sink
    }
    if (ismain) {

        /* list->push_back(new ASM::Oper(std::string("mov r8, #0xff"), Temp_TempList(),
        Temp_TempList(), Temp_LabelList())); list->push_back(new ASM::Oper(std::string("and r0, r0,
        r8"), Temp_TempList(), Temp_TempList(), Temp_LabelList())); list->push_back(new
        ASM::Oper(std::string("mov r7, r0"), Temp_TempList(), Temp_TempList(), Temp_LabelList()));
        list->push_back(new ASM::Oper(std::string("mov r0, #10"), Temp_TempList(), Temp_TempList(),
                                      Temp_LabelList()));
        list->push_back(
            new ASM::Oper("bl putch", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
        list->push_back(new ASM::Oper(std::string("mov r0, r7"), Temp_TempList(), Temp_TempList(),
                                      Temp_LabelList()));
        list->push_back(
            new ASM::Oper("bl putint", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
        list->push_back(new ASM::Oper(std::string("mov r0, #10"), Temp_TempList(), Temp_TempList(),
                                      Temp_LabelList()));
        list->push_back(
            new ASM::Oper("bl putch", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
        list->push_back(new ASM::Oper(std::string("mov r0, r7"), Temp_TempList(), Temp_TempList(),
                                      Temp_LabelList())); */
    }
    return list;
}
IR::StmList* CANON::transIR(IR::StmList* stmlist) {
    // find temp must use float reg
    std::set<int> fset;
    std::unordered_map<int, std::set<int>> mvmap;
    for (auto list = stmlist; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        if (isttmove(stm)) {
            int dst = static_cast<IR::Temp*>(static_cast<IR::Move*>(stm)->dst)->tempid;
            int src = static_cast<IR::Temp*>(static_cast<IR::Move*>(stm)->src)->tempid;
            if (dst < 15 || src < 15) continue;
            mvmap[dst].insert(src);
            mvmap[src].insert(dst);
        }
    }
    for (auto list = stmlist; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        if (isCall(stm) && getCallName(stm) == "f2i") {
            auto v = getUses(stm);
            for (auto it : v) { fset.insert(static_cast<IR::Temp*>(*it)->tempid); }
        } else if (isCall(stm) && getCallName(stm) == "i2f") {
            auto dstp = getDef(stm);
            if (dstp != 0) { fset.insert(static_cast<IR::Temp*>(*dstp)->tempid); }
        } else if (ismovebi(stm)) {
            IR::binop op = static_cast<IR::Binop*>(static_cast<IR::Move*>(stm)->src)->op;
            if (op == IR::binop::F_plus || op == IR::binop::F_minus || op == IR::binop::F_mul
                || op == IR::binop::F_div) {
                auto dstp = getDef(stm);
                if (dstp != 0) { fset.insert(static_cast<IR::Temp*>(*dstp)->tempid); }
                auto v = getUses(stm);
                for (auto it : v) { fset.insert(static_cast<IR::Temp*>(*it)->tempid); }
            }
        } else if (stm->kind == IR::stmType::cjump) {
            IR::RelOp op = static_cast<IR::Cjump*>(stm)->op;
            if (op == IR::RelOp::F_eq || op == IR::RelOp::F_ge || op == IR::RelOp::F_gt
                || op == IR::RelOp::F_le || op == IR::RelOp::F_lt || op == IR::RelOp::F_ne) {
                auto v = getUses(stm);
                for (auto it : v) { fset.insert(static_cast<IR::Temp*>(*it)->tempid); }
            }
        }
    }
    std::queue<int> worklist;
    for (auto it : fset) worklist.push(it);
    while (!worklist.empty()) {
        int tp = worklist.front();
        worklist.pop();
        if (mvmap.count(tp)) {
            for (auto it : mvmap[tp]) {
                if (!fset.count(it)) {
                    worklist.push(it);
                    fset.insert(it);
                }
            }
        }
    }
    for (auto list = stmlist; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        auto dstp = getDef(stm);
        if (dstp) {
            int& dst = static_cast<IR::Temp*>(*dstp)->tempid;
            if (fset.count(dst)) dst = ~dst;
        }
        auto v = getUses(stm);
        for (auto it : v) {
            int& src = static_cast<IR::Temp*>(*it)->tempid;
            if (fset.count(src)) src = ~src;
        }
    }
    return stmlist;
}