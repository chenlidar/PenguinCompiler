#include "treeIR.hpp"
#include <assert.h>
#include <algorithm>
#include <string>
#include <iostream>
#include "../backend/canon.hpp"
#include "../util/utils.hpp"
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
RelOp commute(RelOp op) {  // a op b    ==    b commute(op) a
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
void Label::ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) {
    ls->push_back(new ASM::Label(label));
}
void Jump::ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) {
    if (exp->kind == IR::expType::name)  // Jump(Name(Label(L)),LabelList(Lable(L)))
    {
        assert(this->jumps.size() == 1);  // must have jumps in our language
        auto tname = static_cast<IR::Name*>(exp);
        if (tname->name == "RETURN") {
            jumps[0] = exitlabel;
            ls->push_back(
                new ASM::Oper(string("b ") + exitlabel, Temp_TempList(), Temp_TempList(), jumps));
        } else
            ls->push_back(new ASM::Oper(string("b ") + tname->name, Temp_TempList(),
                                        Temp_TempList(), jumps));
    } else if (exp->kind == IR::expType::temp) {
        auto ttmp = static_cast<IR::Temp*>(exp);
        ls->push_back(new ASM::Oper(string("bx `s0"), Temp_TempList(),
                                    Temp_TempList(1, ttmp->tempid), jumps));
    } else
        assert(0);
    // Jump( temp ,LabelList( NamedLabel("END") ))
}
void Cjump::ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) {
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
        tmp[0] = this->left->ir2asm(ls);
        tmp[1] = this->right->ir2asm(ls);
        src.push_back(tmp[0]);
        src.push_back(tmp[1]);
        ls->push_back(new ASM::Oper(std::string("cmp `s0, `s1"), dst, src, ASM::Targets()));
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
void Move::ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) {
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
    if (this->dst->kind == IR::expType::temp && this->src->kind == IR::expType::binop
            && static_cast<IR::Binop*>(this->src)->left->kind == IR::expType::temp
        && static_cast<IR::Binop*>(this->src)->right->kind == IR::expType::temp) {
        IR::Binop* rexp = static_cast<IR::Binop*>(this->src);
        Temp_Temp lexp = static_cast<IR::Temp*>(this->dst)->tempid;
        Temp_Temp ltemp = static_cast<IR::Temp*>(rexp->left)->tempid;
        Temp_Temp rtemp = static_cast<IR::Temp*>(rexp->right)->tempid;
        switch (rexp->op) {
        case IR::binop::T_plus:
            ls->push_back(
                new ASM::Oper(std::string("add `d0, `s0, `s1"), Temp_TempList({lexp}), Temp_TempList({ltemp,rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_minus:
            ls->push_back(
                new ASM::Oper(std::string("sub `d0, `s0, `s1"), Temp_TempList({lexp}), Temp_TempList({ltemp,rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_mul:
            ls->push_back(
                new ASM::Oper(std::string("mul `d0, `s0, `s1"), Temp_TempList({lexp}), Temp_TempList({ltemp,rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_div:
            ls->push_back(
                new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), Temp_TempList({lexp}), Temp_TempList({ltemp,rtemp}), ASM::Targets()));
            break;
        case IR::binop::T_mod: {
            Temp_Temp temp = Temp_newtemp();
            ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"), Temp_TempList({temp}),
                                        Temp_TempList({ltemp,rtemp}), ASM::Targets()));
            ls->push_back(new ASM::Oper(std::string("mul `d0, `s0, `s1"), Temp_TempList(1, temp),
                                        Temp_TempList({temp,rtemp}), ASM::Targets()));
            ls->push_back(
                new ASM::Oper(std::string("sub `d0, `s0, `s1"), Temp_TempList({lexp}), Temp_TempList({ltemp,temp}), ASM::Targets()));
            break;
        }
        default: assert(0); break;
        }
    }
    else if (this->dst->kind == IR::expType::mem)  // Move(Mem(e1), e2)
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
        src.push_back(static_cast<IR::Mem*>(this->src)->ir2asm(ls));
        ls->push_back(new ASM::Move(std::string("mov `d0, `s0"), dst, src));
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
void ExpStm::ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) {
    //  FIXME
    if (this->exp->kind == expType::call) this->exp->ir2asm(ls);
}

Temp_Temp Const::ir2asm(ASM::InstrList* ls) {
    int int_const = this->val;
    Temp_Temp tmp[4];
    Temp_TempList src = Temp_TempList(), dst = Temp_TempList();
    dst.push_back(Temp_newtemp());
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
        src[0] = temp;
        ls->push_back(new ASM::Oper(std::string("mul `d0, `s0, `s1"), Temp_TempList(1, temp), src,
                                    ASM::Targets()));
        src[0] = exp_l;
        src[1] = temp;
        ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"), dst, src, ASM::Targets()));
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
    assert(this->fun->kind == expType::name);
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
            ->ir2asm(ls, "");
    }
    for (; head; head = head->tail) head->stm->ir2asm(ls, "");
    Temp_TempList uses = Temp_TempList();
    for (int i = 0; i < cnt; i++) { uses.push_back(i); }
#ifndef VFP
    Temp_Temp ftemp = Temp_newtemp();
    if (static_cast<IR::Name*>(this->fun)->name == "putfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov s0, r0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("mov `d0, sp"), Temp_TempList(1, ftemp),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsr sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsl sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
    }
    if (static_cast<IR::Name*>(this->fun)->name == "putfarray") {
        ls->push_back(new ASM::Oper(std::string("mov `d0, sp"), Temp_TempList(1, ftemp),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsr sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
        ls->push_back(new ASM::Oper(std::string("lsl sp, sp, #4"), Temp_TempList(),
                                    Temp_TempList(), ASM::Targets()));
    }
#endif
    ls->push_back(new ASM::Oper(std::string("bl ") + static_cast<IR::Name*>(this->fun)->name,
                                Temp_TempList({0, 1, 2, 3, 14, 12}), uses, ASM::Targets()));
#ifndef VFP
    if (static_cast<IR::Name*>(this->fun)->name == "putfloat"
        || (static_cast<IR::Name*>(this->fun)->name == "putfarray")) {
        ls->push_back(new ASM::Oper(std::string("mov sp, `s0"), Temp_TempList(),
                                    Temp_TempList(1, ftemp), ASM::Targets()));
    }
    if (static_cast<IR::Name*>(this->fun)->name == "getfloat") {
        ls->push_back(new ASM::Oper(std::string("vmov r0, s0"), Temp_TempList(), Temp_TempList(),
                                    ASM::Targets()));
    }
#endif
    if (stksize) {
        (new IR::Move(new IR::Temp(13),
                      new IR::Binop(IR::binop::T_plus, new IR::Temp(13), new IR::Const(stksize))))
            ->ir2asm(ls, "");
    }
    return 0;  // r0
}
ASM::Proc* IR::ir2asm(StmList* stmlist) {
    ASM::Proc* proc = new ASM::Proc();
    IR::Stm* label = getLast(stmlist)->tail->stm;
    assert(label->kind == stmType::label);
    Temp_Label exitlabel = static_cast<IR::Label*>(label)->label;
    stmlist = CANON::funcEntryExit1(stmlist);
    for (; stmlist; stmlist = stmlist->tail) stmlist->stm->ir2asm(&proc->body, exitlabel);
    return proc;
}

// quad
Stm* Label::quad() { return new Label(label); }
Stm* Jump::quad() { return new Jump(exp->quad(), jumps); }
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
    return new Eseq(new Move(new Temp(ntp), new Call(fun->quad(), tm)), new Temp(ntp));
}

void Label::printIR() { std::cerr << "LABELSTM:    " << label << endl; }
void Jump::printIR() {
    std::cerr << "JUMPSTM:    ";
    exp->printIR();
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
    std::cerr << "( call ";
    fun->printIR();
    std::cerr << '(';
    for (auto it : args) {
        it->printIR();
        std::cerr << ", ";
    }
    std::cerr << ") )";
}
