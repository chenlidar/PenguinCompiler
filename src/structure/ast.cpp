#include "ast.h"
#include "../util/utils.hpp"
#include "treeIR.hpp"
#include <assert.h>

IR::StmList* AST::CompUnitList::ast2ir(Table::Stable<TY::Entry*>* venv,
                                       Table::Stable<TY::EnFunc*>* fenv) {
    IR::StmList *ret, *tail;
    ret = tail = NULL;
    for (const auto& it : list) {
        if (ret == NULL)
            ret = tail = new IR::StmList(it->ast2ir(venv, fenv, "", "", ""), NULL);
        else
            tail = tail->tail = new IR::StmList(it->ast2ir(venv, fenv, "", "", ""), NULL);
    }
    return ret;
}
IR::Stm* AST::ConstDecl::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    for (auto it : defs->list) { it->binder(btype, venv, fenv, name); }
    return nopStm();
}
void AST::ConstDef::binder(AST::btype_t btype, Table::Stable<TY::Entry*>* venv,
                           Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    switch (btype) {
    case AST::btype_t::INT: {  // TODO insert table
        if (!index_list) {  // int

        } else {  // int []
        }
    } break;
    default: assert(0);
    }
}

IR::Stm* AST::FuncDef::ast2ir() {
    // TODO
}
IR::Stm* AST::Block::ast2ir() {
    // auto ls = items->list;
    // auto nowseq = nopStm();
    // for (const auto& p : ls) nowseq = seq(nowseq, p->ast2ir());
    // return nowseq;
}
IR::Stm* AST::AssignStmt::ast2ir() {
    // TODO
}
IR::Stm* AST::ExpStmt::ast2ir() {
    // TODO
}
IR::Stm* AST::IfStmt::ast2ir() {
    // TODO
}
IR::Stm* AST::BreakStmt::ast2ir() {}
IR::Stm* AST::ContinueStmt::ast2ir() {}
IR::Stm* AST::ReturnStmt::ast2ir() {}

IR::ExpTy AST::Lval::ast2ir() {}
IR::ExpTy AST::IntNumber::ast2ir() {
    auto exp = new IR::Tr_Exp(new IR::ConstInt(value));
    return IR::ExpTy(exp, TY::intType(new int(this->value)));
}
IR::ExpTy AST::FloatNumber::ast2ir() {
    // TODO
    assert(0);
    return IR::ExpTy();
}
IR::ExpTy AST::UnaryExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label name) {
    IR::ExpTy body = exp->ast2ir(venv, fenv, name);
    IR::Exp* exp;
    switch (op) {
    case unaryop_t::ADD: return body;
    case unaryop_t::SUB:
        exp = new IR::Binop(IR::binop::T_minus, new IR::ConstInt(0), body.exp->ex);
        return IR::ExpTy(new IR::Tr_Exp(exp), body.ty);
    case unaryop_t::NOT: {
        IR::Cx cx = body.exp->unCx();
        return IR::ExpTy(new IR::Tr_Exp(cx.falses, cx.trues, cx.stm), body.ty);
    }
    default: assert(0);
    }
}
IR::ExpTy AST::CallExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                               Temp_Label name) {
    assert(fenv->exist(*this->id->str));
    TY::EnFunc* func = fenv->look(*this->id->str);
    IR::ExpList list;
    for (auto it : this->params->list) {
        list.push_back(it->ast2ir(venv, fenv, name).exp->unEx());
    }
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(new IR::Name(func->label), list)), func->ty);
}
IR::ExpTy AST::MulExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label name) {
    IR::Exp* exp;
    TY::Type* t;
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    assert(lExpTy.ty->kind == TY::tyType::Ty_int && rExpTy.ty->kind == TY::tyType::Ty_int);
    IR::binop bop;
    switch (op) {
    case mul_t::MULT: bop = IR::binop::T_mul; break;
    case mul_t::DIV: bop = IR::binop::T_div; break;
    case mul_t::REM: bop = IR::binop::T_mod; break;
    default: assert(0);
    }
    exp = new IR::Binop(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx());
    t = binopResType(lExpTy.ty, rExpTy.ty, bop);
    return IR::ExpTy(new IR::Tr_Exp(exp), t);
}

IR::ExpTy AST::AddExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label name) {
    IR::Exp* exp;
    TY::Type* t;
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    assert(lExpTy.ty->kind == TY::tyType::Ty_int && rExpTy.ty->kind == TY::tyType::Ty_int);
    // FIXME except (int op int)
    IR::binop bop;
    switch (this->op) {
    case AST::addop_t::ADD: bop = IR::binop::T_plus; break;
    case AST::addop_t::SUB: bop = IR::binop::T_minus; break;
    default: assert(0);
    }
    exp = new IR::Binop(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx());
    t = binopResType(lExpTy.ty, rExpTy.ty, bop);
    return IR::ExpTy(new IR::Tr_Exp(exp), t);
}
IR::ExpTy AST::RelExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label name) {
    TY::Type* t = TY::intType(0);
    IR::RelOp bop;
    switch (this->op) {
    case AST::rel_t::GE: bop = IR::RelOp::T_eq; break;
    case AST::rel_t::GT: bop = IR::RelOp::T_gt; break;
    case AST::rel_t::LE: bop = IR::RelOp::T_le; break;
    case AST::rel_t::LT: bop = IR::RelOp::T_lt; break;
    default: assert(0);
    }
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cjump* stm = new IR::Cjump(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx(), NULL, NULL);
    IR::PatchList* trues = new IR::PatchList(stm->trueLabel, NULL);
    IR::PatchList* falses = new IR::PatchList(stm->falseLabel, NULL);
    return IR::ExpTy(new IR::Tr_Exp(trues, falses, stm), t);
}
IR::ExpTy AST::EqExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label name) {
    TY::Type* t = TY::intType(0);
    IR::RelOp bop;
    switch (this->op) {
    case AST::equal_t::EQ: bop = IR::RelOp::T_eq; break;
    case AST::equal_t::NE: bop = IR::RelOp::T_ne; break;
    default: assert(0);
    }
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cjump* stm = new IR::Cjump(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx(), NULL, NULL);
    IR::PatchList* trues = new IR::PatchList(stm->trueLabel, NULL);
    IR::PatchList* falses = new IR::PatchList(stm->falseLabel, NULL);
    return IR::ExpTy(new IR::Tr_Exp(trues, falses, stm), t);
}
IR::ExpTy AST::LAndExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                               Temp_Label name) {
    IR::Exp* exp;
    TY::Type* t = TY::intType(0);
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cx lcx = lExpTy.exp->unCx();
    IR::Cx rcx = rExpTy.exp->unCx();
    Temp_Label lab = Temp_newlabel();
    doPatch(lcx.trues, lab);
    return IR::ExpTy(
        new IR::Tr_Exp(rcx.trues, joinPatch(lcx.falses, rcx.falses),
                       new IR::Seq(lcx.stm, new IR::Seq(new IR::Label(lab), rcx.stm))),
        t);
}
IR::ExpTy AST::LOrExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label name) {
    IR::Exp* exp;
    TY::Type* t = TY::intType(0);
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cx lcx = lExpTy.exp->unCx();
    IR::Cx rcx = rExpTy.exp->unCx();
    Temp_Label lab = Temp_newlabel();
    doPatch(lcx.falses, lab);
    return IR::ExpTy(
        new IR::Tr_Exp(joinPatch(lcx.trues, rcx.trues), rcx.falses,
                       new IR::Seq(lcx.stm, new IR::Seq(new IR::Label(lab), rcx.stm))),
        t);
}
IR::ExpTy AST::IdExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label name) {
    std::string idname = *(this->str);
    assert(venv->exist(idname));
    TY::Entry* entry = venv->look(idname);
    IR::Exp* exp;
    TY::Type* t = entry->ty;
    if (entry->kind == TY::tyEntry::Ty_global) {
        exp = new IR::Name(static_cast<TY::GloVar*>(entry)->label);
    } else if (entry->kind == TY::tyEntry::Ty_local) {
        exp = new IR::Temp(static_cast<TY::LocVar*>(entry)->temp);
    }
    // assume that array is not in "IdExp"
    IR::ExpTy(new IR::Tr_Exp(exp), t);
}
IR::Stm* AST::PutintStmt::ast2ir() {}
IR::Stm* AST::PutchStmt::ast2ir() {}
IR::Stm* AST::PutarrayStmt::ast2ir() {}
IR::Stm* AST::PutfloatStmt::ast2ir() {}
IR::Stm* AST::PutfarrayStmt::ast2ir() {}
IR::Stm* AST::PutfStmt::ast2ir() {}
IR::Stm* AST::StarttimeStmt::ast2ir() {}
IR::Stm* AST::StoptimeStmt::ast2ir() {}
IR::ExpTy AST::GetintExp::ast2ir() {}
IR::ExpTy AST::GetchExp::ast2ir() {}
IR::ExpTy AST::GetfloatExp::ast2ir() {}
IR::ExpTy AST::GetarrayExp::ast2ir() {}
IR::ExpTy AST::GetfarrayExp::ast2ir() {}
