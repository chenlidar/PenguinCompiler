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

string binop2string(binop op) {
    switch (op) {
    case binop::T_div: return "div";
    case binop::T_minus: return "minus";
    case binop::T_mod: return "mod";
    case binop::T_mul: return "mul";
    case binop::T_plus: return "plus";
    default: return "unknown";
    }
}

string relop2string(RelOp op) {
    switch (op) {
    case RelOp::T_eq: return "eq";
    case RelOp::T_ne: return "ne";
    case RelOp::T_lt: return "lt";
    case RelOp::T_ge: return "ge";
    case RelOp::T_gt: return "gt";
    case RelOp::T_le: return "le";
    default: return "unknown";
    }
}
RelOp IR::commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
    case RelOp::T_eq: return RelOp::T_eq;
    case RelOp::T_ne: return RelOp::T_ne;
    case RelOp::T_lt: return RelOp::T_gt;
    case RelOp::T_ge: return RelOp::T_le;
    case RelOp::T_gt: return RelOp::T_lt;
    case RelOp::T_le: return RelOp::T_ge;
    }
}
RelOp IR::notRel(RelOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
    case RelOp::T_eq: return RelOp::T_ne;
    case RelOp::T_ne: return RelOp::T_eq;
    case RelOp::T_lt: return RelOp::T_ge;
    case RelOp::T_ge: return RelOp::T_lt;
    case RelOp::T_gt: return RelOp::T_le;
    case RelOp::T_le: return RelOp::T_gt;
    }
}
Cx IR::Tr_Exp::unCx() {
    switch (this->kind) {
    case Tr_ty::Tr_cx: {
        return this->cx;
    } break;
    case Tr_ty::Tr_ex: {
        Stm* stm = new Cjump(RelOp::T_ne, this->ex, new Const(0), "", "");
        Cx cx;
        cx.stm = stm;
        cx.falses = new PatchList(&static_cast<Cjump*>(stm)->falseLabel, NULL);
        cx.trues = new PatchList(&static_cast<Cjump*>(stm)->trueLabel, NULL);
        return cx;
    } break;
    default: assert(0);
    }
}
Exp* IR::Tr_Exp::unEx() {
    switch (this->kind) {
    case Tr_ty::Tr_ex: {
        return this->ex;
    } break;
    case Tr_ty::Tr_cx: {
        Temp_Temp r = Temp_newtemp();
        Temp_Label t = Temp_newlabel(), f = Temp_newlabel();
        doPatch(this->cx.trues, t);
        doPatch(this->cx.falses, f);
        return new Eseq(
            new Move(new Temp(r), new Const(1)),
            new Eseq(this->cx.stm,
                     new Eseq(new Label(f), new Eseq(new Move(new Temp(r), new Const(0)),
                                                     new Eseq(new Label(t), new Temp(r))))));
    } break;
    default: assert(0);
    }
}
Stm* IR::Tr_Exp::unNx() {
    switch (this->kind) {
    case Tr_ty::Tr_ex: return new ExpStm(this->ex);
    case Tr_ty::Tr_cx: return new ExpStm(this->unEx());
    case Tr_ty::Tr_nx: return this->nx;
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

/////////////////////////////////////////////////////////////////////////////////////////////////
//   TILE
/////////////////////////////////////////////////////////////////////////////////////////////////
void Label::ir2asm(ASM::InstrList* ls) { ls->push_back(new ASM::Label(label)); }
void Jump::ir2asm(ASM::InstrList* ls) {
    ls->push_back(new ASM::Oper(string("b ") + target, Temp_TempList(), Temp_TempList(),
                                ASM::Targets({target})));

    // Jump( temp ,LabelList( NamedLabel("END") ))
}
void Cjump::ir2asm(ASM::InstrList* ls) {
    // TODO
    // Cjump(op , e1 , Binop(mul, e2, Const(2^k)) , Lable(L) , falselabel)
    // Cjump(op , e1 , Binop(mul, Const(2^k), e2) , Lable(L) , falselabel)
    // Cjump(op , Binop(mul, e2, Const(2^k)) , e1 , Lable(L) , falselabel)
    // Cjump(op , Binop(mul, Const(2^k), e2) , e1 ,  Lable(L) ,
    // falselabel) Cjump(op , e1 , Binop(div, Const(2^k), e2) , Lable(L) ,
    // falselabel) Cjump(op, Binop(div, Const(2^k), e2)  , e1 , Lable(L) ,
    // falselabel)
    Temp_Temp tmp[4];
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();

    {  // Most naive tile method

        int flag = 0;
        auto num1 = exp2int(left), num2 = exp2int(right);
        if (num1.first && num2.first) { assert(0); }
        if (num1.first || num2.first) {
            if (num1.first) {
                op = commute(op);
                std::swap(left, right);
                std::swap(num1, num2);
            }
            auto imm2 = exp2op2(num2.second);
            if (imm2.first) {
                tmp[0] = this->left->ir2asm(ls);
                src.push_back(tmp[0]);
                ls->push_back(new ASM::Oper(std::string("cmp `s0, " + imm2.second), dst, src,
                                            ASM::Targets()));
                flag = 1;
                // assert(0);
            }
        }

        if (!flag) {
            tmp[0] = this->left->ir2asm(ls);
            tmp[1] = this->right->ir2asm(ls);
            src.push_back(tmp[0]);
            src.push_back(tmp[1]);
            ls->push_back(new ASM::Oper(std::string("cmp `s0, `s1"), dst, src, ASM::Targets()));
        }
        std::string branch_type;
        if (this->op == RelOp::T_ne)
            branch_type = std::string("bne");
        else if (this->op == RelOp::T_eq)
            branch_type = std::string("beq");
        else if (this->op == RelOp::T_lt)
            branch_type = std::string("blt");
        else if (this->op == RelOp::T_gt)
            branch_type = std::string("bgt");
        else if (this->op == RelOp::T_le)
            branch_type = std::string("ble");
        else if (this->op == RelOp::T_ge)
            branch_type = std::string("bge");
        else
            assert(0);
        auto jumplist = ASM::Targets();
        jumplist.push_back(this->trueLabel);
        jumplist.push_back(this->falseLabel);
        ls->push_back(new ASM::Oper(branch_type + " " + this->trueLabel, Temp_TempList(),
                                    Temp_TempList(), jumplist));
    }
}
void Move::ir2asm(ASM::InstrList* ls) {
    /* TODO
    //Move(Mem(Binop(minus, e1, Const(k))), e2)
    //Move(Mem(Binop(plus, e1, Const(k))), e2)
    //Move(Mem(Binop(plus, Const(k), e1)), e2)
    //Move(Mem(Binop(plus, e1, e2)), e3)
    //Move(Mem(e1), e2)
    //Move(temp,Mem(Binop(plus,e1,Const(k))))
    //Move(temp,Mem(Binop(plus,Const(k),e1)))
    //Move(temp,Mem(Binop(minus,e1,Const(k))))
    //Move(temp, Binop(mul,e1,Const(2^k)))
    //Move(temp, Binop(mul,Const(2^k),e1))
    //Move(temp, Binop(div,e1,Const(2^k)))
    //Move(temp,Mem(Binop(plus,e1,e2)))
    //Move(temp, Const(k))
    //Move(temp,Name(Label(L)))
    ////Move(temp,Mem(e))
    //Move(temp, e1)
    */
    Temp_Temp tmp[4];
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    int int_const;
    static const int N = 32;
    if (this->dst->kind == IR::expType::temp && this->src->kind == IR::expType::binop) {
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
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, ") + imm.second,
                                                Temp_TempList({lexp}), Temp_TempList({rtemp1}),
                                                ASM::Targets()));
                    return;
                }
                case IR::binop::T_minus: {
                    auto rtemp2 = opsexp->ir2asm(ls);
                    if (cons < 0 && num1.first) { break; }
                    ls->push_back(new ASM::Oper(
                        (num1.first ? "rsb" : "sub") + std::string(" `d0, `s0, " + imm.second),
                        Temp_TempList({lexp}), Temp_TempList({rtemp2}), ASM::Targets()));
                    return;
                }

                default: break;
                }
            }

            if (rexp->op == IR::binop::T_mul) {  //  xx*const
                if (cons == 0) {
                    // FIXME rtemp sideeffect
                    ls->push_back(new ASM::Oper(std::string("mov `d0, #0"), Temp_TempList({lexp}),
                                                Temp_TempList(), ASM::Targets()));
                } else if (cons == 1) {
                    auto rtemp = opsexp->ir2asm(ls);
                    if (rtemp != lexp) {
                        ls->push_back(new ASM::Oper(std::string("mov `d0, `s0"),
                                                    Temp_TempList({lexp}), Temp_TempList({rtemp}),
                                                    ASM::Targets()));
                    }
                } else if (num1.second == -1) {
                    auto rtemp = opsexp->ir2asm(ls);
                    ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                Temp_TempList({lexp}), Temp_TempList({rtemp}),
                                                ASM::Targets()));
                }
                auto tp = MULoptTest(abs(cons));
                if (tp) {
                    auto rtemp = opsexp->ir2asm(ls);
                    if (cons > 0) {
                        switch (tp) {
                        case 1: {
                            auto s = MULget1(abs(cons));
                            auto imm1 = exp2op2(s);
                            ls->push_back(new ASM::Oper(
                                std::string("lsl `d0, `s0, " + imm1.second), Temp_TempList({lexp}),
                                Temp_TempList({rtemp}), ASM::Targets()));
                            break;
                        }
                        case 2: {
                            auto p = MULget2(abs(cons));
                            auto s = p.first, t = p.second;
                            auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                            ls->push_back(
                                new ASM::Oper(std::string("add `d0, `s0, `s1, lsl " + imm1.second),
                                              Temp_TempList({lexp}), Temp_TempList({rtemp, rtemp}),
                                              ASM::Targets()));
                            ls->push_back(new ASM::Oper(
                                std::string("lsl `d0, `s0, " + imm2.second), Temp_TempList({lexp}),
                                Temp_TempList({lexp}), ASM::Targets()));
                            break;
                        }
                        case 3: {
                            auto p = MULget3(abs(cons));
                            auto s = p.first, t = p.second;
                            auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                            ls->push_back(
                                new ASM::Oper(std::string("rsb `d0, `s0, `s1, lsl" + imm1.second),
                                              Temp_TempList({lexp}), Temp_TempList({rtemp, rtemp}),
                                              ASM::Targets()));
                            ls->push_back(new ASM::Oper(
                                std::string("lsl `d0, `s0, " + imm2.second), Temp_TempList({lexp}),
                                Temp_TempList({lexp}), ASM::Targets()));
                            break;
                        }
                        default: assert(0);
                        }
                    } else {
                        switch (tp) {
                        case 1: {
                            auto s = MULget1(abs(cons));
                            auto imm1 = exp2op2(s);
                            ls->push_back(new ASM::Oper(std::string("mov `d0, #0"),
                                                        Temp_TempList({lexp}), Temp_TempList(),
                                                        ASM::Targets()));
                            ls->push_back(
                                new ASM::Oper(std::string("sub `d0, `s0, `s1, lsl " + imm1.second),
                                              Temp_TempList({lexp}), Temp_TempList({lexp, rtemp}),
                                              ASM::Targets()));
                            break;
                        }
                        case 2: {
                            auto p = MULget2(abs(cons));
                            auto s = p.first, t = p.second;
                            auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                            ls->push_back(
                                new ASM::Oper(std::string("add `d0, `s0, `s1, lsl " + imm1.second),
                                              Temp_TempList({lexp}), Temp_TempList({rtemp, rtemp}),
                                              ASM::Targets()));
                            ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                        Temp_TempList({lexp}),
                                                        Temp_TempList({lexp}), ASM::Targets()));
                            ls->push_back(new ASM::Oper(
                                std::string("lsl `d0, `s0, " + imm2.second), Temp_TempList({lexp}),
                                Temp_TempList({lexp}), ASM::Targets()));
                            break;
                        }
                        case 3: {
                            auto p = MULget3(abs(cons));
                            auto s = p.first, t = p.second;
                            auto imm1 = exp2op2(s - t), imm2 = exp2op2(t);
                            ls->push_back(
                                new ASM::Oper(std::string("rsb `d0, `s0, `s1, lsl" + imm1.second),
                                              Temp_TempList({lexp}), Temp_TempList({rtemp, rtemp}),
                                              ASM::Targets()));
                            ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                        Temp_TempList({lexp}),
                                                        Temp_TempList({lexp}), ASM::Targets()));
                            ls->push_back(new ASM::Oper(
                                std::string("lsl `d0, `s0, " + imm2.second), Temp_TempList({lexp}),
                                Temp_TempList({lexp}), ASM::Targets()));
                            break;
                        }
                        default: assert(0);
                        }
                    }
                    return;
                }
            }

            if (num2.first && rexp->op == IR::binop::T_div) {
                auto rtemp = opsexp->ir2asm(ls);
                assert(cons != 0);
                assert(lexp != rtemp);
                auto mut = chooseMultiplier(abs(cons), N - 1);
                if (abs(cons) == 1) {
                    if (rtemp != lexp) {
                        ls->push_back(new ASM::Oper(std::string("mov `d0, `s0"),
                                                    Temp_TempList({lexp}), Temp_TempList({rtemp}),
                                                    ASM::Targets()));
                    }
                } else if (check2pow(abs(cons))) {
                    auto imm1 = exp2op2(mut.l - 1), imm2 = exp2op2(N - mut.l),
                         imm3 = exp2op2(mut.l);
                    auto t = Temp_newtemp();
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                Temp_TempList({lexp}), Temp_TempList({rtemp}),
                                                ASM::Targets()));
                    ls->push_back(new ASM::Oper(
                        std::string("add `d0, `s0, `s1, lsr") + imm2.second, Temp_TempList({t}),
                        Temp_TempList({rtemp, lexp}), ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm3.second,
                                                Temp_TempList({lexp}), Temp_TempList({t}),
                                                ASM::Targets()));
                } else if (mut.m < (1ll << (N - 1))) {
                    auto tmp = new IR::Const(mut.m);
                    auto ttmp = tmp->ir2asm(ls);
                    delete tmp;
                    auto imm1 = exp2op2(mut.sh);
                    ls->push_back(new ASM::Oper(std::string("smmul `d0, `s0, `s1"),
                                                Temp_TempList({ttmp}),
                                                Temp_TempList({ttmp, rtemp}), ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                Temp_TempList({lexp}), Temp_TempList({ttmp}),
                                                ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1, lsr #31"),
                                                Temp_TempList({lexp}),
                                                Temp_TempList({lexp, rtemp}), ASM::Targets()));
                } else {
                    auto tmp = new IR::Const(mut.m - (1ll << N));
                    auto ttmp = tmp->ir2asm(ls);
                    delete tmp;
                    auto imm1 = exp2op2(mut.sh);
                    ls->push_back(new ASM::Oper(
                        std::string("smmla `d0, `s0, `s1, `s2"), Temp_TempList({ttmp}),
                        Temp_TempList({rtemp, ttmp, rtemp}), ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, ") + imm1.second,
                                                Temp_TempList({lexp}), Temp_TempList({ttmp}),
                                                ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1, lsr #31"),
                                                Temp_TempList({lexp}), Temp_TempList({lexp, ttmp}),
                                                ASM::Targets()));
                }
                if (cons < 0) {
                    ls->push_back(new ASM::Oper(std::string("rsb `d0, `s0, #0"),
                                                Temp_TempList({lexp}), Temp_TempList({lexp}),
                                                ASM::Targets()));
                }
                return;
            }
            if (num2.first && rexp->op == IR::binop::T_mod) {
                auto rtemp = opsexp->ir2asm(ls);
                assert(cons != 0);
                assert(lexp != rtemp);

                if (check2pow(cons)) {
                    int t = ceil(log2(cons));
                    auto imm1 = exp2op2(t), imm2 = exp2op2(N - t);
                    ls->push_back(new ASM::Oper(std::string("asr `d0, `s0, #31"),
                                                Temp_TempList({lexp}), Temp_TempList({rtemp}),
                                                ASM::Targets()));
                    ls->push_back(new ASM::Oper(
                        std::string("add `d0, `s0, `s1, lsr") + imm2.second, Temp_TempList({lexp}),
                        Temp_TempList({rtemp, lexp}), ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("bfc `d0, #0, ") + imm1.second,
                                                Temp_TempList({lexp}), Temp_TempList(),
                                                ASM::Targets()));
                    ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"),
                                                Temp_TempList({lexp}),
                                                Temp_TempList({rtemp, lexp}), ASM::Targets()));
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
            ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1"), Temp_TempList({lexp}),
                                        Temp_TempList({ltemp, rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_minus:
            ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"), Temp_TempList({lexp}),
                                        Temp_TempList({ltemp, rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_mul:
            ls->push_back(new ASM::Oper(std::string("mul `d0, `s0, `s1"), Temp_TempList({lexp}),
                                        Temp_TempList({ltemp, rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_div:
            ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), Temp_TempList({lexp}),
                                        Temp_TempList({ltemp, rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_mod: {
            Temp_Temp temp = Temp_newtemp();
            ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), Temp_TempList({temp}),
                                        Temp_TempList({ltemp, rtemp}), ASM::Targets()));
            ls->push_back(new ASM::Oper(std::string("mls `d0, `s0, `s1, `s2"),
                                        Temp_TempList({lexp}), Temp_TempList({temp, rtemp, ltemp}),
                                        ASM::Targets()));
            break;
        }
        default: assert(0); break;
        }
    } else if (this->dst->kind == IR::expType::mem)  // Move(Mem(e1), e2)
    {
        tmp[0] = this->src->ir2asm(ls);
        tmp[1] = static_cast<IR::Mem*>(this->dst)->mem->ir2asm(ls);
        src.push_back(tmp[0]);
        src.push_back(tmp[1]);
        ls->push_back(new ASM::Oper(std::string("str `s0, [`s1]"), dst, src, ASM::Targets()));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::constx)  // Move(temp, Const(k))
    {
        int_const = static_cast<IR::Const*>(this->src)->val;
        tmp[0] = this->dst->ir2asm(ls);
        dst.push_back(tmp[0]);
        if (int_const > 256 || int_const < -128) {
            ls->push_back(
                new ASM::Oper(std::string("movw `d0, #:lower16:") + std::to_string(int_const), dst,
                              src, ASM::Targets()));
            ls->push_back(
                new ASM::Oper(std::string("movt `d0, #:upper16:") + std::to_string(int_const), dst,
                              src, ASM::Targets()));
        } else
            ls->push_back(new ASM::Oper(std::string("mov `d0, #") + std::to_string(int_const), dst,
                                        src, ASM::Targets()));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::name)  // Move(temp,Name(Label(L)))
    {
        dst.push_back(static_cast<IR::Temp*>(this->dst)->tempid);
        ls->push_back(new ASM::Oper(std::string("movw `d0,#:lower16:")
                                        + static_cast<IR::Name*>(this->src)->name,
                                    dst, src, ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("movt `d0,#:upper16:")
                                        + static_cast<IR::Name*>(this->src)->name,
                                    dst, src, ASM::Targets()));
    } else if (this->dst->kind == IR::expType::temp
               && this->src->kind == IR::expType::mem)  // Move(temp,Mem(e))
    {
        dst.push_back(static_cast<IR::Temp*>(this->dst)->tempid);
        assert(static_cast<IR::Mem*>(this->src)->mem->kind == expType::temp
               || static_cast<IR::Mem*>(this->src)->mem->kind == expType::name);
        auto mempos = static_cast<IR::Mem*>(this->src)->mem->ir2asm(ls);
        src.push_back(mempos);
        ls->push_back(new ASM::Oper(std::string("ldr `d0, [`s0]"), dst, src, ASM::Targets()));
        // src.push_back(static_cast<IR::Mem*>(this->src)->ir2asm(ls));
        // ls->push_back(new ASM::Move(std::string("mov `d0, `s0"), dst, src));
    } else if (this->dst->kind == IR::expType::temp)  // Move(temp, e1)
    {
        dst.push_back(static_cast<IR::Temp*>(this->dst)->tempid);
        // printf("%s\n",
        //        static_cast<IR::Name*>(static_cast<IR::Call*>(this->src)->fun)->name.c_str());
        src.push_back(this->src->ir2asm(ls));
        ls->push_back(new ASM::Move(std::string("mov `d0, `s0"), dst, src));
    } else if (this->dst->kind == IR::expType::name) {
        Temp_Temp newtemp = Temp_newtemp();
        dst.push_back(newtemp);
        ls->push_back(new ASM::Oper(std::string("movw `d0,#:lower16:")
                                        + static_cast<IR::Name*>(this->dst)->name,
                                    dst, src, ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("movt `d0,#:upper16:")
                                        + static_cast<IR::Name*>(this->dst)->name,
                                    dst, src, ASM::Targets()));
        src.push_back(this->src->ir2asm(ls));
        src.push_back(newtemp);
        ls->push_back(
            new ASM::Oper(std::string("str `s0, [`s1]"), Temp_TempList(), src, ASM::Targets()));
    } else
        assert(0);
}
void ExpStm::ir2asm(ASM::InstrList* ls) {
    //  FIXME
    if (this->exp->kind == expType::call) this->exp->ir2asm(ls);
}

Temp_Temp Const::ir2asm(ASM::InstrList* ls) {
    int int_const = this->val;
    Temp_Temp tmp[4];
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    dst.push_back(Temp_newtemp());
    if (int_const > 65535 || int_const < -257) {
        ls->push_back(
            new ASM::Oper(std::string("movw `d0, #:lower16:") + std::to_string(int_const), dst,
                          src, ASM::Targets()));
        ls->push_back(
            new ASM::Oper(std::string("movt `d0, #:upper16:") + std::to_string(int_const), dst,
                          src, ASM::Targets()));
    } else {
        if (int_const < 0) {
            ls->push_back(new ASM::Oper(std::string("mvn `d0, #") + std::to_string(-int_const - 1),
                                        dst, src, ASM::Targets()));
        } else if (int_const < 257) {
            ls->push_back(new ASM::Oper(std::string("mov `d0, #") + std::to_string(int_const), dst,
                                        src, ASM::Targets()));
        } else {
            ls->push_back(new ASM::Oper(std::string("movw `d0, #") + std::to_string(int_const),
                                        dst, src, ASM::Targets()));
        }
    }
    return dst[0];
}
Temp_Temp Binop::ir2asm(ASM::InstrList* ls) {
    Temp_Temp exp_l = this->left->ir2asm(ls);
    Temp_Temp exp_r = this->right->ir2asm(ls);
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    src.push_back(exp_l);
    src.push_back(exp_r);
    dst.push_back(Temp_newtemp());

    switch (this->op) {
    case IR::binop::T_plus:
        ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1"), dst, src, ASM::Targets()));
        return dst[0];
        break;
    case IR::binop::T_minus:
        ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"), dst, src, ASM::Targets()));
        return dst[0];
    case IR::binop::T_mul:
        ls->push_back(new ASM::Oper(std::string("mul `d0, `s0, `s1"), dst, src, ASM::Targets()));
        return dst[0];
    case IR::binop::T_div:
        ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), dst, src, ASM::Targets()));
        return dst[0];
    case IR::binop::T_mod: {
        // assert(0);  // FIXME
        Temp_Temp temp = Temp_newtemp();
        ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), Temp_TempList(1, temp), src,
                                    ASM::Targets()));
        src.push_back(src[0]);
        src[0] = temp;
        ls->push_back(
            new ASM::Oper(std::string("mls `d0, `s0, `s1, `s2"), dst, src, ASM::Targets()));
        return dst[0];
    }
    default: assert(0); break;
    }
}
Temp_Temp Temp::ir2asm(ASM::InstrList* ls) { return this->tempid; }
Temp_Temp Mem::ir2asm(ASM::InstrList* ls) {
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    src.push_back(this->mem->ir2asm(ls));
    dst.push_back(Temp_newtemp());
    ls->push_back(new ASM::Oper(std::string("ldr `d0, [`s0]"), dst, src, ASM::Targets()));
    return dst[0];
}
Temp_Temp Eseq::ir2asm(ASM::InstrList* ls) {
    assert(0);  // Shoudn't Exist
    return 0;
}
Temp_Temp Name::ir2asm(ASM::InstrList* ls) {
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    dst.push_back(Temp_newtemp());
    ls->push_back(
        new ASM::Oper(std::string("movw `d0,#:lower16:") + this->name, dst, src, ASM::Targets()));
    ls->push_back(
        new ASM::Oper(std::string("movt `d0,#:upper16:") + this->name, dst, src, ASM::Targets()));
    return dst[0];
}
Temp_Temp Call::ir2asm(ASM::InstrList* ls) {
    int cnt = 0, stksize = 0;
    IR::StmList *head = nullptr, *tail = nullptr;
    for (auto it : this->args) {
        //
        IR::Stm* stm;
        if (cnt < 4) {
            stm = new IR::Move(new IR::Temp(cnt), it);
            cnt++;
        } else {
            stm = new IR::Move(new IR::Mem(new IR::Binop(IR::binop::T_plus, new IR::Temp(13),
                                                         new IR::Const(stksize))),
                               it);
            stksize += 4;
        }  // low ..now stack..sp 5 6 7 8 ... high
        if (head == nullptr)
            head = tail = new IR::StmList(stm, nullptr);
        else
            tail = tail->tail = new IR::StmList(stm, nullptr);
    }
    if (stksize) {
        (new IR::Move(new IR::Temp(13),
                      new IR::Binop(IR::binop::T_plus, new IR::Temp(13), new IR::Const(-stksize))))
            ->ir2asm(ls);
    }
    for (; head; head = head->tail) head->stm->ir2asm(ls);
    Temp_TempList uses = Temp_TempList();
    for (int i = 0; i < cnt; i++) { uses.push_back(i); }
#ifndef VFP
    Temp_Temp ftemp = Temp_newtemp();
    if (fun == "putfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("mov `d0, sp"), Temp_TempList(1, ftemp),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsr sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsl sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
    }
    if (fun == "putfarray") {
        ls->push_back(new ASM::Oper(std::string("mov `d0, sp"), Temp_TempList(1, ftemp),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsr sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsl sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
    }
#endif

#ifndef VFP
    if (fun == "__aeabi_fadd") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov s1, r1"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vadd.f32 s0, s0, s1"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    } else if (fun == "__aeabi_fsub") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov s1, r1"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vsub.f32 s0, s0, s1"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    } else if (fun == "__aeabi_fmul") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov s1, r1"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmul.f32 s0, s0, s1"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    } else if (fun == "__aeabi_fdiv") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov s1, r1"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vdiv.f32 s0, s0, s1"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    } else {
        ls->push_back(new ASM::Oper(std::string("bl ") + fun, Temp_TempList({0, 1, 2, 3, 14, 12}),
                                    uses, ASM::Targets()));
    }
#else
    ls->push_back(new ASM::Oper(std::string("bl ") + fun, Temp_TempList({0, 1, 2, 3, 14, 12}),
                                uses, ASM::Targets()));
#endif
#ifndef VFP
    if (fun == "putfloat" || (fun == "putfarray")) {
        ls->push_back(new ASM::Oper(std::string("mov sp, `s0"), Temp_TempList(),
                                    Temp_TempList(1, ftemp), ASM::Targets()));
    }
    if (fun == "getfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    }
#endif
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
                    // static int cnt = 0;
                    // std::cerr << ++cnt << std::endl;
                    s2->stm = new IR::Move(m2->dst->quad(), m1->src->quad());
                    // can continue to do other optimize
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
                        if (tid1 == tid2 && tempcur[tid1] == 1) {
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
                                            Temp_TempList({dtid}), Temp_TempList({tid3}),
                                            ASM::Targets()));
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
                                    proc->body.push_back(new ASM::Oper(
                                        std::string("ldr `d0, [`s0, `s1]"), Temp_TempList({dtid}),
                                        Temp_TempList({tid3, tid4}), ASM::Targets()));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;
                                } else if (bop->op == binop::T_minus) {
                                    assert(0);
                                    auto tid3 = (bop->left)->ir2asm(&proc->body);
                                    auto tid4 = (bop->right)->ir2asm(&proc->body);
                                    proc->body.push_back(new ASM::Oper(
                                        std::string("ldr `d0, [`s0, -`s1]"), Temp_TempList({dtid}),
                                        Temp_TempList({tid3, tid4}), ASM::Targets()));
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
                        auto tid1 = static_cast<IR::Temp*>(memexp->mem)->tempid;
                        auto tid2 = static_cast<IR::Temp*>(m1->dst)->tempid;
                        if (tid1 == tid2 && tempcur[tid1] == 1) {
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
                                        auto dtid = (m2->src)->ir2asm(&proc->body);
                                        proc->body.push_back(new ASM::Oper(
                                            std::string("str `s0, [`s1, " + imm.second + "]"),
                                            Temp_TempList(), Temp_TempList({dtid, tid3}),
                                            ASM::Targets()));
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
                                        auto dtid = (m2->src)->ir2asm(&proc->body);
                                        auto tid3 = (bop->left)->ir2asm(&proc->body);
                                        auto tid4 = (bop->right)->ir2asm(&proc->body);
                                        proc->body.push_back(new ASM::Oper(
                                            std::string("str `s0, [`s1, `s2]"), Temp_TempList(),
                                            Temp_TempList({dtid, tid3, tid4}), ASM::Targets()));
                                        s1->stm = nopStm();
                                        s2->stm = nopStm();
                                        s1 = s3;
                                        continue;
                                    } else if (bop->op == binop::T_minus) {
                                        assert(0);
                                        auto dtid = (m2->src)->ir2asm(&proc->body);
                                        auto tid3 = (bop->left)->ir2asm(&proc->body);
                                        auto tid4 = (bop->right)->ir2asm(&proc->body);
                                        proc->body.push_back(new ASM::Oper(
                                            std::string("str `s0, [`s1, -`s2]"), Temp_TempList(),
                                            Temp_TempList({dtid, tid3, tid4}), ASM::Targets()));
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
                                    proc->body.push_back(new ASM::Oper(
                                        std::string("mla `d0, `s0, `s1, `s2"), Temp_TempList({r1}),
                                        Temp_TempList({r2, r3, r4}), ASM::Targets()));
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
                                    proc->body.push_back(new ASM::Oper(
                                        std::string("mla `d0, `s0, `s1, `s2"), Temp_TempList({r1}),
                                        Temp_TempList({r2, r3, r4}), ASM::Targets()));
                                    s1->stm = nopStm();
                                    s2->stm = nopStm();
                                    s1 = s3;
                                    continue;

                                } else if (bi2->op == binop::T_minus) {
                                    auto r1 = static_cast<IR::Temp*>(m2->dst)->tempid;
                                    auto r2 = bi1->left->ir2asm(&proc->body);
                                    auto r3 = bi1->right->ir2asm(&proc->body);
                                    auto r4 = bi2->left->ir2asm(&proc->body);
                                    proc->body.push_back(new ASM::Oper(
                                        std::string("mls `d0, `s0, `s1, `s2"), Temp_TempList({r1}),
                                        Temp_TempList({r2, r3, r4}), ASM::Targets()));
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

// quad
Stm* Label::quad() { return new Label(label); }
Stm* Jump::quad() { return new Jump(target); }
Stm* Cjump::quad() { return new Cjump(op, left->quad(), right->quad(), trueLabel, falseLabel); }
Stm* Move::quad() {
    if (src->kind == expType::mem && dst->kind == expType::mem) {
        Temp_Temp ntp = Temp_newtemp();
        return new Seq(new Move(new Temp(ntp), new Mem(static_cast<Mem*>(src)->mem->quad())),
                       new Move(new Mem(static_cast<Mem*>(dst)->mem->quad()), new Temp(ntp)));
    }
    if (dst->kind == expType::mem) {
        return new Move(new Mem(static_cast<Mem*>(dst)->mem->quad()), src->quad());
    }
    if (src->kind == expType::mem) {
        return new Move(dst->quad(), new Mem(static_cast<Mem*>(src)->mem->quad()));
    }
    return new Move(dst->quad(), src->quad());
}
Stm* ExpStm::quad() { return new ExpStm(exp->quad()); }
Exp* Const::quad() { return new Const(val); }
Exp* Binop::quad() {
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Binop(op, left->quad(), right->quad())),
                    new Temp(ntp));
}
Exp* Temp::quad() { return new Temp(tempid); }
Exp* Mem::quad() {
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Mem(mem->quad())), new Temp(ntp));
}
Exp* Name::quad() { return new Name(name); }
Exp* Call::quad() {
    vector<Exp*> tm;
    for (auto it : args) tm.push_back(it->quad());
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Call(fun, tm)), new Temp(ntp));
}

void Label::printIR() { std::cerr << "LABELSTM:    " << label << endl; }
void Jump::printIR() {
    std::cerr << "JUMPSTM:    " + target;
    std::cerr << endl;
}
void Cjump::printIR() {
    std::cerr << "CJUMPSTM:    ";
    left->printIR();
    std::cerr << " " + relop2string(op) + " ";
    right->printIR();
    std::cerr << endl << "true:    " << trueLabel << endl << "false:    " << falseLabel << endl;
}
void Move::printIR() {
    std::cerr << "MOVESTM:    ";
    dst->printIR();
    std::cerr << "    ";
    src->printIR();
    std::cerr << endl;
}
void ExpStm::printIR() {
    std::cerr << "EXPSTM:    ";
    exp->printIR();
    std::cerr << endl;
}
void Const::printIR() { std::cerr << val; }
void Binop::printIR() {
    std::cerr << "( ";
    left->printIR();
    std::cerr << " " + binop2string(op) + " ";
    right->printIR();
    std::cerr << " )";
}
void Temp::printIR() { std::cerr << "t" << tempid; }
void Mem::printIR() {
    std::cerr << "Mem( ";
    mem->printIR();
    std::cerr << " )";
}
void Name::printIR() { std::cerr << name; }
void Call::printIR() {
    std::cerr << "( call " + fun;
    std::cerr << '(';
    for (auto it : args) {
        it->printIR();
        std::cerr << ", ";
    }
    std::cerr << ") )";
}

// DEEPCOPY

StmList* StmList::deepCopy(unordered_set<Temp_Label>& venv, int offset,
                           unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                           unordered_map<string, string>& lbmp) {
    vector<Stm*> ls;
    auto tt = Temp_newtemp();
    tpmp[11] = tt;
    StmList* ret = 0;
    auto p = this;
    int flag = 0;
    while (p) {
        p->stm->deepCopy(tpmp, lbmp, venv, ls);
        p = p->tail;
        if (flag == 0) {
            ls.push_back(new IR::Move(new Temp(tt),
                                      new Binop(binop::T_plus, new Temp(11), new Const(offset))));
            flag = 1;
        }
    }
    int len = ls.size();
    for (int i = len - 1; i >= 0; i--) { ret = new StmList(ls[i], ret); }
    return ret;
}
void Seq::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                   unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    left->deepCopy(tpmp, lbmp, venv, ls);
    right->deepCopy(tpmp, lbmp, venv, ls);
}
void Label::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    if (venv.count(label)) {
        ls.push_back(new Label(label));
        return;
    }
    if (lbmp.count(label)) {
        ls.push_back(new Label(lbmp[label]));
        return;
    }
    auto nw = Temp_newlabel();
    lbmp[label] = nw;
    ls.push_back(new Label(nw));
}
void Jump::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (venv.count(target)) {
        ls.push_back(new Jump(target));
        return;
    }
    if (lbmp.count(target)) {
        ls.push_back(new Jump(lbmp[target]));
        return;
    }
    auto nw = Temp_newlabel();
    lbmp[target] = nw;
    ls.push_back(new Jump(nw));
}
void Cjump::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    string ntl, nfl;
    if (venv.count(trueLabel))
        ntl = trueLabel;
    else if (lbmp.count(trueLabel))
        ntl = lbmp[trueLabel];
    else {
        ntl = Temp_newlabel();
        lbmp[trueLabel] = ntl;
    }
    if (venv.count(falseLabel))
        nfl = falseLabel;
    else if (lbmp.count(falseLabel))
        nfl = lbmp[falseLabel];
    else {
        nfl = Temp_newlabel();
        lbmp[falseLabel] = nfl;
    }
    ls.push_back(new Cjump(op, left->deepCopy(tpmp, lbmp, venv, ls),
                           right->deepCopy(tpmp, lbmp, venv, ls), ntl, nfl));
}
void Move::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    ls.push_back(
        new Move(dst->deepCopy(tpmp, lbmp, venv, ls), src->deepCopy(tpmp, lbmp, venv, ls)));
}
void ExpStm::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                      unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                      vector<Stm*>& ls) {
    ls.push_back(new ExpStm(exp->deepCopy(tpmp, lbmp, venv, ls)));
}
Exp* Const::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    return new Const(val);
}
Exp* Binop::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                     unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                     vector<Stm*>& ls) {
    return new Binop(op, left->deepCopy(tpmp, lbmp, venv, ls),
                     right->deepCopy(tpmp, lbmp, venv, ls));
}
Exp* Temp::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (tpmp.count(tempid)) return new Temp(tpmp[tempid]);
    if (tempid == 11 || tempid == 13) {
        if (tempid == 11) { return new Temp(tpmp[11]); }
        return new Temp(tempid);
    }
    auto nw = Temp_newtemp();
    tpmp[tempid] = nw;
    return new Temp(nw);
}
Exp* Mem::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                   unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    return new Mem(mem->deepCopy(tpmp, lbmp, venv, ls));
}
Exp* Eseq::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    assert(0);
    return 0;
}
Exp* Name::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    if (venv.count(name)) return new Name(name);
    if (lbmp.count(name)) return new Name(lbmp[name]);
    auto nw = Temp_newlabel();
    lbmp[name] = nw;
    return new Name(nw);
}
Exp* Call::deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                    unordered_set<Temp_Label>& venv, vector<Stm*>& ls) {
    vector<Exp*> nwa;
    for (auto it : args) nwa.push_back(it->deepCopy(tpmp, lbmp, venv, ls));
    return new Call(fun, nwa);
}