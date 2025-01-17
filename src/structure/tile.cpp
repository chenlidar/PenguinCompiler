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

void Label::ir2asm(ASM::InstrList* ls) { ls->push_back(new ASM::Label(label)); }
void Jump::ir2asm(ASM::InstrList* ls) {
    ls->push_back(new ASM::Oper(string("b ") + target, {}, {}, {target}));
}
void Cjump::ir2asm(ASM::InstrList* ls) {
    int flag = 0;
    auto num1 = exp2int(left), num2 = exp2int(right);
    if (num1.first && num2.first) { assert(0); }
    if (op == IR::RelOp::F_eq || op == IR::RelOp::F_ge || op == IR::RelOp::F_gt
        || op == IR::RelOp::F_le || op == IR::RelOp::F_lt || op == IR::RelOp::F_ne) {
        int lf = this->left->ir2asm(ls);
        int rt = this->right->ir2asm(ls);
        if (lf > 0) {
            int newtp = ~Temp_newtemp();
            ls->push_back(new ASM::Oper("vmov `d0, `s0", {newtp}, {lf}, {}));
            lf = newtp;
        }
        if (rt > 0) {
            int newtp = ~Temp_newtemp();
            ls->push_back(new ASM::Oper("vmov `d0, `s0", {newtp}, {rt}, {}));
            rt = newtp;
        }
        ls->push_back(new ASM::Oper("vcmp.f32 `s0, `s1", {}, {lf, rt}, {}));
        ls->push_back(new ASM::Oper("vmrs APSR_nzcv, fpscr", {}, {}, {}));
    } else {
        if (num1.first || num2.first) {
            if (num1.first) {
                op = commute(op);
                std::swap(left, right);
                std::swap(num1, num2);
            }
            auto imm2 = exp2op2(num2.second);
            if (imm2.first) {
                auto lf = this->left->ir2asm(ls);
                ls->push_back(new ASM::Oper(std::string("cmp `s0, " + imm2.second), {}, {lf}, {}));
                flag = 1;
            }
        }
        if (!flag) {
            auto lf = this->left->ir2asm(ls);
            auto rt = this->right->ir2asm(ls);
            ls->push_back(new ASM::Oper(std::string("cmp `s0, `s1"), {}, {lf, rt}, {}));
        }
    }
    std::string branch_type = "b" + relop2string(this->op);
    ls->push_back(
        new ASM::Oper(branch_type + " " + this->trueLabel, {}, {}, {trueLabel, falseLabel}));
}
void Move::ir2asm(ASM::InstrList* ls) {

    static const int N = 32;
    if (this->dst->kind == IR::expType::temp
        && this->src->kind == IR::expType::binop) {  // a=b op c
        IR::Binop* rexp = static_cast<IR::Binop*>(this->src);
        Temp_Temp lexp = static_cast<IR::Temp*>(this->dst)->tempid;

        auto num1 = exp2int(rexp->left), num2 = exp2int(rexp->right);
        if (num1.first && num2.first) { assert(0); }
        if (num1.first || num2.first) {

            auto opsexp = (num1.first) ? rexp->right : rexp->left;
            auto cons = (num1.first) ? num1.second : num2.second;
            auto imm = exp2op2(cons);
            if (imm.first) {
                switch (rexp->op) {
                case IR::binop::T_plus: {
                    auto rtemp1 = opsexp->ir2asm(ls);
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, ") + imm.second, {lexp},
                                                {rtemp1}, {}));
                    return;
                }
                case IR::binop::T_minus: {
                    auto rtemp2 = opsexp->ir2asm(ls);
                    if (cons < 0 && num1.first) { break; }
                    ls->push_back(new ASM::Oper((num1.first ? "rsb" : "sub")
                                                    + std::string(" `d0, `s0, " + imm.second),
                                                {lexp}, {rtemp2}, {}));
                    return;
                }
                default: break;
                }
            }

            if (rexp->op == IR::binop::T_mul) {  //  a=b*const
                if (cons == 0 || cons == 1) {
                    assert(0);
                } else if (num1.second == -1) {
                    auto rtemp = opsexp->ir2asm(ls);
                    ls->push_back(
                        new ASM::Oper(std::string("rsb `d0, `s0, #0"), {lexp}, {rtemp}, {}));
                } else {
                    auto tp = MULoptTest(abs(cons));
                    if (tp) {
                        auto rtemp = opsexp->ir2asm(ls);
                        if (cons > 0) {
                            switch (tp) {
                            case 1: {
                                auto s = MULget1(abs(cons));
                                auto imm1 = exp2op2(s);
                                ls->push_back(
                                    new ASM::Oper(std::string("lsl `d0, `s0, " + imm1.second),
                                                  {lexp}, {rtemp}, {}));
                                break;
                            }
                            case 2: {
                                auto p = MULget2(abs(cons));
                                auto s = p.first, t = p.second;
                                auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                                ls->push_back(new ASM::Oper(
                                    std::string("add `d0, `s0, `s1, lsl " + imm1.second), {lexp},
                                    {rtemp, rtemp}, {}));
                                ls->push_back(
                                    new ASM::Oper(std::string("lsl `d0, `s0, " + imm2.second),
                                                  {lexp}, {lexp}, {}));
                                break;
                            }
                            case 3: {
                                auto p = MULget3(abs(cons));
                                auto s = p.first, t = p.second;
                                auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                                ls->push_back(new ASM::Oper(
                                    std::string("rsb `d0, `s0, `s1, lsl" + imm1.second), {lexp},
                                    {rtemp, rtemp}, {}));
                                ls->push_back(
                                    new ASM::Oper(std::string("lsl `d0, `s0, " + imm2.second),
                                                  {lexp}, {lexp}, {}));
                                break;
                            }
                            default: assert(0);
                            }
                        } else {
                            switch (tp) {
                            case 1: {
                                auto s = MULget1(abs(cons));
                                auto imm1 = exp2op2(s);
                                ls->push_back(
                                    new ASM::Oper(std::string("mov `d0, #0"), {lexp}, {}, {}));
                                ls->push_back(new ASM::Oper(
                                    std::string("sub `d0, `s0, `s1, lsl " + imm1.second), {lexp},
                                    {lexp, rtemp}, {}));
                                break;
                            }
                            case 2: {
                                auto p = MULget2(abs(cons));
                                auto s = p.first, t = p.second;
                                auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                                ls->push_back(new ASM::Oper(
                                    std::string("add `d0, `s0, `s1, lsl " + imm1.second), {lexp},
                                    {rtemp, rtemp}, {}));
                                ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                            {lexp}, {lexp}, {}));
                                ls->push_back(
                                    new ASM::Oper(std::string("lsl `d0, `s0, " + imm2.second),
                                                  {lexp}, {lexp}, {}));
                                break;
                            }
                            case 3: {
                                auto p = MULget3(abs(cons));
                                auto s = p.first, t = p.second;
                                auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                                ls->push_back(new ASM::Oper(
                                    std::string("rsb `d0, `s0, `s1, lsl" + imm1.second), {lexp},
                                    {rtemp, rtemp}, {}));
                                ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                            {lexp}, {lexp}, {}));
                                ls->push_back(
                                    new ASM::Oper(std::string("lsl `d0, `s0, " + imm2.second),
                                                  {lexp}, {lexp}, {}));
                                break;
                            }
                            default: assert(0);
                            }
                        }
                        return;
                    }
                }
            }

            if (num2.first && rexp->op == IR::binop::T_div) {  // a=b/const
                auto rtemp = opsexp->ir2asm(ls);
                assert(cons != 0);
                assert(lexp != rtemp);
                auto mut = chooseMultiplier(abs(cons), N - 1);
                if (abs(cons) == 1) {
                    if (rtemp != lexp) {
                        ls->push_back(
                            new ASM::Oper(std::string("mov `d0, `s0"), {lexp}, {rtemp}, {}));
                    }
                } else if (check2pow(abs(cons))) {
                    auto imm1 = exp2op2(mut.l - 1), imm2 = exp2op2(N - mut.l),
                         imm3 = exp2op2(mut.l);
                    auto t = Temp_newtemp();
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                {lexp}, {rtemp}, {}));
                    ls->push_back(
                        new ASM::Oper(std::string("add `d0, `s0, `s1, lsr") + imm2.second, {t},
                                      Temp_TempList({rtemp, lexp}), {}));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm3.second,
                                                {lexp}, {t}, {}));
                } else if (mut.m < (1ll << (N - 1))) {
                    auto tmp = new IR::Const(mut.m);
                    auto ttmp = tmp->ir2asm(ls);
                    delete tmp;
                    auto imm1 = exp2op2(mut.sh);
                    ls->push_back(new ASM::Oper(std::string("smmul `d0, `s0, `s1"), {ttmp},
                                                Temp_TempList({ttmp, rtemp}), {}));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                {lexp}, {ttmp}, {}));
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1, lsr #31"), {lexp},
                                                {lexp, rtemp}, {}));
                } else {
                    auto tmp = new IR::Const(mut.m - (1ll << N));
                    auto ttmp = tmp->ir2asm(ls);
                    delete tmp;
                    auto imm1 = exp2op2(mut.sh);
                    ls->push_back(new ASM::Oper(std::string("smmla `d0, `s0, `s1, `s2"), {ttmp},
                                                {rtemp, ttmp, rtemp}, {}));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                {lexp}, {ttmp}, {}));
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1, lsr #31"), {lexp},
                                                {lexp, ttmp}, {}));
                }
                if (cons < 0) {
                    ls->push_back(
                        new ASM::Oper(std::string("rsb `d0, `s0, #0"), {lexp}, {lexp}, {}));
                }
                return;
            }
            if (num2.first && rexp->op == IR::binop::T_mod) {  // a=b%const
                auto rtemp = opsexp->ir2asm(ls);
                assert(cons != 0);
                assert(lexp != rtemp);

                if (check2pow(cons)) {
                    int t = ceil(log2(cons));
                    auto imm1 = exp2op2(t), imm2 = exp2op2(N - t);
                    ls->push_back(
                        new ASM::Oper(std::string("asr `d0, `s0, #31"), {lexp}, {rtemp}, {}));
                    ls->push_back(
                        new ASM::Oper(std::string("add `d0, `s0, `s1, lsr") + imm2.second, {lexp},
                                      {rtemp, lexp}, {}));
                    ls->push_back(
                        new ASM::Oper(std::string("bfc `d0, #0, ") + imm1.second, {lexp}, {}, {}));
                    ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"), {lexp},
                                                {rtemp, lexp}, {}));
                    return;
                }

                auto tmp = Temp_newtemp();
                auto s1 = new Move(new Temp(tmp),
                                   new Binop(binop::T_div, new Temp(rtemp), new Const(cons)));
                auto s2 = new Move(new Temp(tmp),
                                   new Binop(binop::T_mul, new Temp(tmp), new Const(cons)));
                auto s3 = new Move(new Temp(lexp),
                                   new Binop(binop::T_minus, new Temp(rtemp), new Temp(tmp)));
                s1->ir2asm(ls);
                s2->ir2asm(ls);
                s3->ir2asm(ls);
                delete s1;
                delete s2;
                delete s3;
                return;
            }
        }
        auto ltemp = rexp->left->ir2asm(ls), rtemp = rexp->right->ir2asm(ls);
        switch (rexp->op) {
        case IR::binop::T_plus:
        case IR::binop::T_minus:
        case IR::binop::T_mul:
        case IR::binop::T_div: {
            ls->push_back(new ASM::Oper(binop2string(rexp->op) + " `d0, `s0, `s1", {lexp},
                                        {ltemp, rtemp}, {}));
        } break;
        case IR::binop::T_mod: {
            Temp_Temp temp = Temp_newtemp();
            ls->push_back(
                new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), {temp}, {ltemp, rtemp}, {}));
            ls->push_back(new ASM::Oper(std::string("mls `d0, `s0, `s1, `s2"), {lexp},
                                        {temp, rtemp, ltemp}, {}));
            break;
        }
        case IR::binop::F_plus:
        case IR::binop::F_minus:
        case IR::binop::F_mul:
        case IR::binop::F_div: {
            assert(lexp < 0);
            if (ltemp > 0) {
                int newtp = ~Temp_newtemp();
                ls->push_back(new ASM::Oper("vmov `d0, `s0", {newtp}, {ltemp}, {}));
                ltemp = newtp;
            }
            if (rtemp > 0) {
                int newtp = ~Temp_newtemp();
                ls->push_back(new ASM::Oper("vmov `d0, `s0", {newtp}, {rtemp}, {}));
                rtemp = newtp;
            }
            ls->push_back(new ASM::Oper(binop2string(rexp->op) + " `d0, `s0, `s1", {lexp},
                                        {ltemp, rtemp}, {}));
        } break;
        default: assert(0); break;
        }
    } else if (this->dst->kind
               == IR::expType::mem) {  // Move(Mem(const/name/temp), const/name/temp)
        auto s1 = this->src->ir2asm(ls);
        auto s2 = static_cast<IR::Mem*>(this->dst)->mem->ir2asm(ls);
        if (s1 < 0)
            ls->push_back(new ASM::Oper(std::string("vstr `s0, [`s1]"), {}, {s1, s2}, {}));
        else
            ls->push_back(new ASM::Oper(std::string("str `s0, [`s1]"), {}, {s1, s2}, {}));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::constx) {  // Move(temp, Const(k))
        int s = static_cast<IR::Const*>(this->src)->val;
        auto d0 = this->dst->ir2asm(ls);
        if (d0 < 0) {
            int tmp = Temp_newtemp();
            ls->push_back(
                new ASM::Oper(std::string("`mov `d0, #") + std::to_string(s), {tmp}, {}, {}));
            ls->push_back(new ASM::Oper(std::string("vmov `d0, `s0"), {d0}, {tmp}, {}));
        } else
            ls->push_back(
                new ASM::Oper(std::string("`mov `d0, #") + std::to_string(s), {d0}, {}, {}));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::name) {  // Move(temp,Name(Label(L)))
        auto d0 = static_cast<IR::Temp*>(this->dst)->tempid;
        assert(d0 >= 0);
        ls->push_back(new ASM::Oper(
            std::string("`mov `d0,@") + static_cast<IR::Name*>(this->src)->name, {d0}, {}, {}));

    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::mem) {  // Move(temp,Mem(e))
        auto d0 = (static_cast<IR::Temp*>(this->dst)->tempid);
        assert(static_cast<IR::Mem*>(this->src)->mem->kind == expType::temp
               || static_cast<IR::Mem*>(this->src)->mem->kind == expType::name);
        auto mempos = static_cast<IR::Mem*>(this->src)->mem->ir2asm(ls);
        assert(mempos >= 0);
        if (d0 < 0)
            ls->push_back(new ASM::Oper(std::string("vldr `d0, [`s0]"), {d0}, {mempos}, {}));
        else
            ls->push_back(new ASM::Oper(std::string("ldr `d0, [`s0]"), {d0}, {mempos}, {}));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::temp) {  // Move(temp, temp)
        auto d0 = static_cast<IR::Temp*>(this->dst)->tempid;
        auto s0 = static_cast<IR::Temp*>(this->src)->tempid;
        if (d0 < 0 && s0 < 0)
            ls->push_back(new ASM::Move("vmov `d0, `s0", {d0}, {s0}));
        else if (d0 >= 0 && s0 >= 0)
            ls->push_back(new ASM::Move("mov `d0, `s0", {d0}, {s0}));
        else
            ls->push_back(new ASM::Oper("vmov `d0, `s0", {d0}, {s0}, {}));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::call) {  // Move(temp, call)
        std::string fun = getCallName(this);
        auto d0 = static_cast<IR::Temp*>(this->dst)->tempid;
        if (fun == "i2f") {
            assert(d0 < 0);
            IR::Exp* exp = getCallParam(this).at(0);
            if (exp->kind == IR::expType::temp) {
                int src = static_cast<IR::Temp*>(exp)->tempid;
                assert(src >= 0);
                ls->push_back(new ASM::Oper("vmov `d0, `s0", {d0}, {src}, {}));
                ls->push_back(new ASM::Oper("vcvt.f32.s32 `d0, `s0", {d0}, {d0}, {}));
            } else if (exp->kind == IR::expType::constx) {
                int num = static_cast<IR::Const*>(exp)->val;
                num = digit_i2f(num);
                int tmp = Temp_newtemp();
                ls->push_back(new ASM::Oper("`mov `d0,#" + std::to_string(num), {tmp}, {}, {}));
                ls->push_back(new ASM::Oper("vmov `d0,`s0", {d0}, {tmp}, {}));
            } else
                assert(0);
        } else if (fun == "f2i") {
            assert(d0 >= 0);
            IR::Exp* exp = getCallParam(this).at(0);
            if (exp->kind == IR::expType::temp) {
                int src = static_cast<IR::Temp*>(exp)->tempid;
                assert(src < 0);
                int tmp = ~Temp_newtemp();
                ls->push_back(new ASM::Oper("vcvt.s32.f32 `d0, `s0", {tmp}, {src}, {}));
                ls->push_back(new ASM::Oper("vmov `d0, `s0", {d0}, {tmp}, {}));
            } else if (exp->kind == IR::expType::constx) {
                int num = static_cast<IR::Const*>(exp)->val;
                num = digit_f2i(num);
                ls->push_back(new ASM::Oper("`mov `d0,#" + std::to_string(num), {d0}, {}, {}));
            } else
                assert(0);
        } else {
            auto s0 = this->src->ir2asm(ls);
            assert(s0 == 0);
            if (d0 < 0)
                ls->push_back(new ASM::Oper(std::string("vmov `d0, `s0"), {d0}, {s0}, {}));
            else
                ls->push_back(new ASM::Move(std::string("mov `d0, `s0"), {d0}, {s0}));
        }

    } else
        assert(0);
}
void ExpStm::ir2asm(ASM::InstrList* ls) {
    if (this->exp->kind == expType::call)
        this->exp->ir2asm(ls);
    else
        assert(isNop(this));
}
Temp_Temp Const::ir2asm(ASM::InstrList* ls) {
    auto d0 = Temp_newtemp();
    ls->push_back(new ASM::Oper(std::string("`mov `d0, #") + std::to_string(val), {d0}, {}, {}));
    return d0;
}
Temp_Temp Binop::ir2asm(ASM::InstrList* ls) {
    assert(0);
    return 0;
}
Temp_Temp Temp::ir2asm(ASM::InstrList* ls) { return this->tempid; }
Temp_Temp Mem::ir2asm(ASM::InstrList* ls) {
    assert(0);
    return 0;
}
Temp_Temp Eseq::ir2asm(ASM::InstrList* ls) {
    assert(0);  // Shoudn't Exist
    return 0;
}
Temp_Temp Name::ir2asm(ASM::InstrList* ls) {
    auto d0 = Temp_newtemp();
    ls->push_back(new ASM::Oper(std::string("`mov `d0,@") + this->name, {d0}, {}, {}));
    return d0;
}
Temp_Temp Call::ir2asm(ASM::InstrList* ls) {
    assert(fun != "i2f" && fun != "f2i");
    int cnt = 0, stksize = 0;
    IR::StmList *head = new IR::StmList(nullptr, nullptr), *tail;
    tail = head;
    for (auto it : this->args) {
        if (cnt < 4) {
            tail = tail->tail = new IR::StmList(new IR::Move(new IR::Temp(cnt), it), nullptr);
            cnt++;
        } else {
            int tmp = Temp_newtemp();
            tail = tail->tail = new IR::StmList(
                new IR::Move(new IR::Temp(tmp), new IR::Binop(IR::binop::T_plus, new IR::Temp(13),
                                                              new IR::Const(stksize))),
                nullptr);
            tail = tail->tail
                = new IR::StmList(new IR::Move(new IR::Mem(new IR::Temp(tmp)), it), nullptr);
            stksize += 4;
        }  // low ..now stack..sp 5 6 7 8 ... high
    }
    if (stksize) {
        (new IR::Move(new IR::Temp(13),
                      new IR::Binop(IR::binop::T_plus, new IR::Temp(13), new IR::Const(-stksize))))
            ->ir2asm(ls);
    }
    for (head = head->tail; head; head = head->tail) head->stm->ir2asm(ls);
    Temp_TempList uses = Temp_TempList();
    for (int i = 0; i < cnt; i++) { uses.push_back(i); }
    Temp_TempList def;
    for (int i = 0; i < 32; i++) def.push_back(~i);
    Temp_TempList tdef{0, 1, 2, 3, 14, 12};
    for (auto it : tdef) def.push_back(it);
    Temp_Temp ftemp = Temp_newtemp();
    if (fun == "putfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov `d0, `s0"), {~0}, {0}, {}));
        uses.push_back(~0);
    }
    if (fun == "putfloat" || fun == "putfarray") {
        ls->push_back(new ASM::Oper(std::string("mov `d0, sp"), Temp_TempList(1, ftemp), {}, {}));
        ls->push_back(new ASM::Oper(std::string("lsr sp, sp, #4"), {}, {}, {}));
        ls->push_back(new ASM::Oper(std::string("lsl sp, sp, #4"), {}, {}, {}));
    }

    ls->push_back(new ASM::Oper(std::string("bl ") + fun, def, uses, {}));

    if (fun == "putfloat" || fun == "putfarray") {
        ls->push_back(new ASM::Oper(std::string("mov sp, `s0"), {}, {ftemp}, {}));
    }
    if (fun == "getfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov `d0, `s0"), {0}, {~0}, {}));
    }

    if (stksize) {
        (new IR::Move(new IR::Temp(13),
                      new IR::Binop(IR::binop::T_plus, new IR::Temp(13), new IR::Const(stksize))))
            ->ir2asm(ls);
    }
    return 0;  // r0
}

ASM::Proc* IR::ir2asm(StmList* stmlist) {
    ASM::Proc* proc = new ASM::Proc();
    stmlist = CANON::funcEntryExit1(stmlist);
    stmlist = CANON::transIR(stmlist);
    // for (; stmlist; stmlist = stmlist->tail) stmlist->stm->ir2asm(&proc->body);

    std::unordered_map<Temp_Temp, int> tempcur;
    auto p = stmlist;
    for (; p; p = p->tail) {
        auto uses = getUses(p->stm);
        for (auto it : uses) { tempcur[static_cast<IR::Temp*>(*it)->tempid]++; }
    }

    StmList *s1, *s2, *s3;
    s1 = stmlist;
    while (s1) {
        s2 = s1->tail;
        if (s2)
            s3 = s2->tail;
        else
            s3 = 0;

        if (s2 && s3) {}
        if (s2) {
            auto w1 = s1->stm, w2 = s2->stm;
            if (w1->kind == stmType::move && w2->kind == stmType::move) {
                auto m1 = static_cast<Move*>(w1), m2 = static_cast<Move*>(w2);
                if (m1->dst->kind == expType::mem && m2->src->kind == expType::mem
                    && expEqual(m1->dst, m2->src)) {
                    delete s2->stm;
                    s2->stm = new IR::Move(m2->dst->quad(), m1->src->quad());
                }
                // useless
                // if (expEqual(m1->dst, m2->dst)) {
                //     if (m1->src->kind != expType::call) {
                //         static int cnt = 0;
                //         std::cerr << ++cnt << std::endl;
                //         delete s1->stm;
                //         s1->stm = nopStm();
                //     }
                // }

                // ldr x,[y+z]
                if (m1->src->kind == expType::binop && m1->dst->kind == expType::temp
                    && m2->src->kind == expType::mem && m2->dst->kind == expType::temp) {
                    auto memexp = static_cast<IR::Mem*>(m2->src);
                    if (memexp->mem->kind == expType::temp) {
                        auto dtid = static_cast<IR::Temp*>(m2->dst)->tempid;
                        auto tid1 = static_cast<IR::Temp*>(memexp->mem)->tempid;
                        auto tid2 = static_cast<IR::Temp*>(m1->dst)->tempid;
                        if (tid1 == tid2 && tempcur[tid1] == 1 && dtid >= 0) {
                            auto bop = static_cast<IR::Binop*>(m1->src);
                            auto num1 = exp2int(bop->left), num2 = exp2int(bop->right);
                            if (num1.first && num2.first) { assert(0); }
                            if (num1.first || num2.first) {
                                auto opsexp = (num1.first) ? bop->right : bop->left;
                                auto cons = (num1.first) ? num1.second : num2.second;
                                auto imm = exp2offset(cons);
                                int flag = 0;
                                if (imm.first
                                    && (opsexp->kind == expType::temp
                                        || opsexp->kind == expType::name)) {
                                    switch (bop->op) {
                                    case IR::binop::T_plus: {
                                        auto tid3 = (opsexp)->ir2asm(&proc->body);
                                        proc->body.push_back(new ASM::Oper(
                                            std::string("ldr `d0, [`s0, " + imm.second + "]"),
                                            {dtid}, {tid3}, {}));
                                        s1->stm = nopStm();
                                        s2->stm = nopStm();
                                        s1 = s3;
                                        flag = 1;
                                        break;
                                    }
                                    default: break;
                                    }
                                }
                                if (flag) continue;
                            } else {
                                if (bop->op == binop::T_plus) {
                                    auto tid3 = (bop->left)->ir2asm(&proc->body);
                                    auto tid4 = (bop->right)->ir2asm(&proc->body);
                                    proc->body.push_back(
                                        new ASM::Oper(std::string("ldr `d0, [`s0, `s1]"), {dtid},
                                                      {tid3, tid4}, {}));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;
                                } else if (bop->op == binop::T_minus) {
                                    assert(0);
                                    auto tid3 = (bop->left)->ir2asm(&proc->body);
                                    auto tid4 = (bop->right)->ir2asm(&proc->body);
                                    proc->body.push_back(
                                        new ASM::Oper(std::string("ldr `d0, [`s0, -`s1]"), {dtid},
                                                      {tid3, tid4}, {}));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;
                                }
                            }
                        }
                    }
                }

                // str x,[y+z]
                if (m1->src->kind == expType::binop && m1->dst->kind == expType::temp
                    && m2->dst->kind == expType::mem) {
                    auto memexp = static_cast<IR::Mem*>(m2->dst);
                    if (memexp->mem->kind == expType::temp) {
                        auto dtid = (m2->src)->ir2asm(&proc->body);
                        auto tid1 = static_cast<IR::Temp*>(memexp->mem)->tempid;
                        auto tid2 = static_cast<IR::Temp*>(m1->dst)->tempid;
                        if (tid1 == tid2 && tempcur[tid1] == 1 && dtid >= 0) {
                            auto bop = static_cast<IR::Binop*>(m1->src);
                            auto num1 = exp2int(bop->left), num2 = exp2int(bop->right);
                            if (num1.first && num2.first) { assert(0); }
                            if (num1.first || num2.first) {
                                auto opsexp = (num1.first) ? bop->right : bop->left;
                                auto cons = (num1.first) ? num1.second : num2.second;
                                auto imm = exp2offset(cons);
                                int flag = 0;
                                if (imm.first
                                    && (opsexp->kind == expType::temp
                                        || opsexp->kind == expType::name)) {

                                    switch (bop->op) {
                                    case IR::binop::T_plus: {
                                        auto tid3 = (opsexp)->ir2asm(&proc->body);
                                        proc->body.push_back(new ASM::Oper(
                                            std::string("str `s0, [`s1, " + imm.second + "]"), {},
                                            {dtid, tid3}, {}));
                                        s1->stm = nopStm();
                                        s2->stm = nopStm();
                                        s1 = s3;
                                        flag = 1;
                                        break;
                                    }
                                    default: break;
                                    }
                                } else {
                                    if (bop->op == binop::T_plus) {
                                        auto tid3 = (bop->left)->ir2asm(&proc->body);
                                        auto tid4 = (bop->right)->ir2asm(&proc->body);
                                        proc->body.push_back(
                                            new ASM::Oper(std::string("str `s0, [`s1, `s2]"), {},
                                                          {dtid, tid3, tid4}, {}));
                                        s1->stm = nopStm();
                                        s2->stm = nopStm();
                                        s1 = s3;
                                        continue;
                                    } else if (bop->op == binop::T_minus) {
                                        assert(0);
                                        auto tid3 = (bop->left)->ir2asm(&proc->body);
                                        auto tid4 = (bop->right)->ir2asm(&proc->body);
                                        proc->body.push_back(
                                            new ASM::Oper(std::string("str `s0, [`s1, -`s2]"), {},
                                                          {dtid, tid3, tid4}, {}));
                                        s1->stm = nopStm();
                                        s2->stm = nopStm();
                                        s1 = s3;
                                        continue;
                                    }
                                }
                                if (flag) continue;
                            }
                        }
                    }
                }

                // mla mls
                if (m1->src->kind == expType::binop && m1->dst->kind == expType::temp
                    && m2->src->kind == expType::binop && m2->dst->kind == expType::temp) {
                    auto bi1 = static_cast<IR::Binop*>(m1->src),
                         bi2 = static_cast<IR::Binop*>(m2->src);
                    auto tid1 = static_cast<IR::Temp*>(m1->dst)->tempid;
                    if (bi1->op == binop::T_mul
                        && (bi2->op == binop::T_plus || bi2->op == binop::T_minus)
                        && tempcur[tid1] == 1) {
                        int flag = 0;
                        auto uses = getUses(m2);
                        for (auto it : uses) {
                            if (static_cast<IR::Temp*>(*it)->tempid == tid1) {
                                flag = 1;
                                break;
                            }
                        }
                        if (flag) {
                            auto c1 = exp2int(bi1->left), c2 = exp2int(bi1->right);
                            if (c1.first || c2.first) {
                                auto opsexp = (c1.first) ? bi1->right : bi1->left;
                                auto cons = (c1.first) ? c1.second : c2.second;
                                auto conopt = MULoptTest(cons);
                                if (conopt) flag = 0;
                            }
                        }
                        if (flag) {
                            auto tid2 = exp2tempid(bi2->left), tid3 = exp2tempid(bi2->right);
                            if (tid2.first && tid2.second == tid1) {
                                if (bi2->op == binop::T_plus) {
                                    auto r1 = static_cast<IR::Temp*>(m2->dst)->tempid;
                                    auto r2 = bi1->left->ir2asm(&proc->body);
                                    auto r3 = bi1->right->ir2asm(&proc->body);
                                    auto r4 = bi2->right->ir2asm(&proc->body);
                                    proc->body.push_back(
                                        new ASM::Oper(std::string("mla `d0, `s0, `s1, `s2"), {r1},
                                                      {r2, r3, r4}, {}));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;
                                }
                            } else if (tid3.first && tid3.second == tid1) {
                                if (bi2->op == binop::T_plus) {
                                    auto r1 = static_cast<IR::Temp*>(m2->dst)->tempid;
                                    auto r2 = bi1->left->ir2asm(&proc->body);
                                    auto r3 = bi1->right->ir2asm(&proc->body);
                                    auto r4 = bi2->left->ir2asm(&proc->body);
                                    proc->body.push_back(
                                        new ASM::Oper(std::string("mla `d0, `s0, `s1, `s2"), {r1},
                                                      {r2, r3, r4}, {}));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;

                                } else if (bi2->op == binop::T_minus) {
                                    auto r1 = static_cast<IR::Temp*>(m2->dst)->tempid;
                                    auto r2 = bi1->left->ir2asm(&proc->body);
                                    auto r3 = bi1->right->ir2asm(&proc->body);
                                    auto r4 = bi2->left->ir2asm(&proc->body);
                                    proc->body.push_back(
                                        new ASM::Oper(std::string("mls `d0, `s0, `s1, `s2"), {r1},
                                                      {r2, r3, r4}, {}));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }
        s1->stm->ir2asm(&proc->body);
        s1 = s1->tail;
    }
    // for (auto it : proc->body) it->print();
    return proc;
}
