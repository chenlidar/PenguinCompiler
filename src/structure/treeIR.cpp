#include "treeIR.hpp"
#include <assert.h>
#include "../util/utils.hpp"
#include <algorithm>
#include <string>
#include "assem.h"
using std::string;
using namespace IR;
RelOp commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
    case RelOp::T_eq: return RelOp::T_eq;
    case RelOp::T_ne: return RelOp::T_ne;
    case RelOp::T_lt: return RelOp::T_gt;
    case RelOp::T_ge: return RelOp::T_le;
    case RelOp::T_gt: return RelOp::T_lt;
    case RelOp::T_le: return RelOp::T_ge;
    case RelOp::T_ult: return RelOp::T_ugt;
    case RelOp::T_uge: return RelOp::T_ule;
    case RelOp::T_ule: return RelOp::T_uge;
    case RelOp::T_ugt: return RelOp::T_ult;
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
    case RelOp::T_ult: return RelOp::T_uge;
    case RelOp::T_uge: return RelOp::T_ult;
    case RelOp::T_ule: return RelOp::T_ugt;
    case RelOp::T_ugt: return RelOp::T_ule;
    }
}
Cx IR::Tr_Exp::unCx() {
    switch (this->kind) {
    case Tr_ty::Tr_cx: {
        return this->cx;
    } break;
    case Tr_ty::Tr_ex: {
        Stm* stm = new Cjump(RelOp::T_ne, this->ex, new ConstInt(0), "", "");
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
            new Move(new Temp(r), new ConstInt(1)),
            new Eseq(this->cx.stm,
                     new Eseq(new Label(f), new Eseq(new Move(new Temp(r), new ConstInt(0)),
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
void Label::ir2asm(ASM::InstrList* ls) {
    ls->push_back(new ASM::Label(label));
}
void Jump::ir2asm(ASM::InstrList* ls) {
    if(exp->kind==IR::expType::name)//Jump(Name(Label(L)),LabelList(Lable(L)))
    {
        assert(this->jumps.size()!=0);//seems have no jumps in our language
        auto tname = static_cast<IR::Name*>(exp);
        ls->push_back(new ASM::Oper(string("b ") + tname->name, Temp_TempList(),
                            Temp_TempList(), jumps));
    }
    else if(exp->kind == IR::expType::temp)
    {
        auto ttmp = static_cast<IR::Temp*>(exp);
        ls->push_back(new ASM::Oper(string("bx `s0"), Temp_TempList(),
                            Temp_TempList(1,ttmp->tempid), jumps));
    }
    else
        assert(0);
    //Jump( temp ,LabelList( NamedLabel("END") ))
}
void Cjump::ir2asm(ASM::InstrList* ls) {
    // TODO
    //Cjump(op , e1 , Binop(mul, e2, Const(2^k)) , Lable(L) , falselabel)
        //Cjump(op , e1 , Binop(mul, Const(2^k), e2) , Lable(L) , falselabel)
        //Cjump(op , Binop(mul, e2, Const(2^k)) , e1 , Lable(L) , falselabel)
        //Cjump(op , Binop(mul, Const(2^k), e2) , e1 ,  Lable(L) , falselabel)
        //Cjump(op , e1 , Binop(div, Const(2^k), e2) , Lable(L) , falselabel)
        //Cjump(op, Binop(div, Const(2^k), e2)  , e1 , Lable(L) , falselabel)
    Temp_Temp tmp[4];
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();

    {//Most naive tile method
        tmp[0] = this->left->ir2asm(ls);
        tmp[1] = this->right->ir2asm(ls);
        src.push_back(tmp[0]);src.push_back(tmp[1]);
        ls->push_back(new ASM::Oper(std::string("cmp `s0 `s1"),dst,src,ASM::Targets()));
        std::string branch_type;
        if(this->op == RelOp::T_ne)branch_type=std::string("bne");
        else if(this->op == RelOp::T_eq)branch_type=std::string("beq");
        else if(this->op == RelOp::T_lt)branch_type=std::string("blt");
        else if(this->op == RelOp::T_gt)branch_type=std::string("bgt");
        else if(this->op == RelOp::T_le)branch_type=std::string("ble");
        else if(this->op == RelOp::T_ge)branch_type=std::string("bge");
        else assert(0);
        ls->push_back(new ASM::Oper(branch_type+" "+this->trueLabel,Temp_TempList(),Temp_TempList(),ASM::Targets()));
    }
}
void Move::ir2asm(ASM::InstrList* ls){
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
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();
    int int_const;
    if(this->dst->kind == IR::expType::mem)//Move(Mem(e1), e2)
    {
        tmp[0] = this->src->ir2asm(ls);
        tmp[1] = static_cast<IR::Mem*>(this->dst)->mem->ir2asm(ls);
        src.push_back(tmp[0]);src.push_back(tmp[1]);
        ls->push_back(new ASM::Oper(std::string("str `s0, [`s1]"),dst,src,ASM::Targets()));
    }
    else if(this->dst->kind == IR::expType::temp && this->src->kind == IR::expType::constint)//Move(temp, Const(k))
    {
        int_const = static_cast<IR::ConstInt*>(this->src)->val;
        tmp[0] = this->dst->ir2asm(ls);
        dst.push_back(tmp[0]);
        ls->push_back(new ASM::Oper(std::string("mov `d0, #")+std::to_string(int_const),dst,src,ASM::Targets()));
    }
    else if(this->dst->kind == IR::expType::temp && this->src->kind == IR::expType::name)//Move(temp,Name(Label(L)))
    {
        dst.push_back(static_cast<IR::Temp*> (this->dst)->tempid);
        ls->push_back(new ASM::Oper(std::string("ldr `d0, =")+static_cast<IR::Name*>(this->src)->name,dst,src,ASM::Targets()));
    }
    else if(this->dst->kind == IR::expType::temp && this->src->kind == IR::expType::mem)//Move(temp,Mem(e))
    {
        dst.push_back(static_cast<IR::Temp*> (this->dst)->tempid);
        src.push_back(static_cast<IR::Mem*> (this->src)->ir2asm(ls));
        ls->push_back(new ASM::Move(std::string("mov `d0, `s0"),dst[0],src[0]));
    }
    else if(this->dst->kind == IR::expType::temp)//Move(temp, e1)
    {
        dst.push_back(static_cast<IR::Temp*> (this->dst)->tempid);
        printf("%s\n",static_cast<IR::Name*>(static_cast<IR::Call*>(this->src)->fun)->name.c_str());
        src.push_back(this->src->ir2asm(ls));
        ls->push_back(new ASM::Move(std::string("mov `d0, `s0"),dst[0],src[0]));
    }
    else assert(0);
}
void ExpStm::ir2asm(ASM::InstrList* ls){
    //TODO
    // assert(0);
}

Temp_Temp ConstInt::ir2asm(ASM::InstrList* ls){
    int int_const = this->val;
    Temp_Temp tmp[4];
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();
    dst.push_back(Temp_newtemp());
    ls->push_back(new ASM::Oper(std::string("mov `d0, #")+std::to_string(int_const),dst,src,ASM::Targets()));
    return dst[0];
}
Temp_Temp ConstFloat::ir2asm(ASM::InstrList* ls){
    //TODO
    return Temp_newtemp();
}
Temp_Temp Binop::ir2asm(ASM::InstrList* ls){
    Temp_Temp exp_l = this->left->ir2asm(ls);
    Temp_Temp exp_r = this->right->ir2asm(ls);
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();
    src.push_back(exp_l);src.push_back(exp_r);
    dst.push_back(Temp_newtemp());

    switch (this->op)
    {
    case IR::binop::T_plus:
        ls->push_back(new ASM::Oper(std::string("add `d0, `s0, `s1"),dst,src,ASM::Targets()));
        return dst[0];
        break;
    case IR::binop::T_minus:
        ls->push_back(new ASM::Oper(std::string("sub `d0, `s0, `s1"),dst,src,ASM::Targets()));
        return dst[0];
    case IR::binop::T_mul:
        ls->push_back(new ASM::Oper(std::string("mul `d0, `s0, `s1"),dst,src,ASM::Targets()));
        return dst[0];
    case IR::binop::T_div:
        ls->push_back(new ASM::Oper(std::string("sdiv `d0, `s0, `s1"),dst,src,ASM::Targets()));
        return dst[0];
    case IR::binop::T_mod:
        assert(0);//FIXME
        return dst[0];
    default:
        assert(0);
        break;
    }
}
Temp_Temp Temp::ir2asm(ASM::InstrList* ls){
    return this->tempid;
}
Temp_Temp Mem::ir2asm(ASM::InstrList* ls){
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();
    src.push_back(this->mem->ir2asm(ls));
    dst.push_back(Temp_newtemp());
    ls->push_back(new ASM::Oper(std::string("ldr `d0, [`s0]"),dst,src,ASM::Targets()));
    return dst[0];
}
Temp_Temp Eseq::ir2asm(ASM::InstrList* ls){
    assert(0);//Shoudn't Exist
    return 0;
}
Temp_Temp Name::ir2asm(ASM::InstrList* ls){
    Temp_TempList src=Temp_TempList(),dst=Temp_TempList();
    dst.push_back(Temp_newtemp());
    ls->push_back(new ASM::Oper(std::string("ldr `d0, =")+this->name,dst,src,ASM::Targets()));
    return dst[0];
}
Temp_Temp Call::ir2asm(ASM::InstrList* ls){
    //TODO
    
    return 0;
}
