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
    case AST::btype_t::INT: {
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
IR::ExpTy AST::UnaryExp::ast2ir() {
    auto body = exp->ast2ir();
    if (body.ty->kind != TY::tyType::Ty_float && body.ty->kind != TY::tyType::Ty_int) {
        // err
    }
    IR::Exp* exp;
    switch (op) {
    case unaryop_t::ADD: return body;
    case unaryop_t::SUB:
        exp = new IR::Binop(IR::binop::T_mul, new IR::ConstInt(-1), body.exp->ex);
        return IR::ExpTy(new IR::Tr_Exp(exp), body.ty);
    case unaryop_t::NOT:
        // FIXME
        // return IR::ExpTy(IR::Tr_Ex(exp), body.ty);
    default:
        // err
    }
}
IR::ExpTy AST::CallExp::ast2ir() {}
IR::ExpTy AST::MulExp::ast2ir() {
    auto lf = lhs->ast2ir(), rf = rhs->ast2ir();
    auto retType = binopResType(lf.ty, rf.ty);
    IR::Exp* exp;
    switch (op) {
    case mul_t::MULT:
        exp = new IR::Binop(IR::binop::T_mul, lf.exp->ex, rf.exp->ex);
        return IR::ExpTy(new IR::Tr_Exp(exp), retType);
    case mul_t::DIV:
        exp = new IR::Binop(IR::binop::T_div, lf.exp->ex, rf.exp->ex);
        return IR::ExpTy(new IR::Tr_Exp(exp), retType);
    case mul_t::REM:
        if (retType->kind == TY::tyType::Ty_float) {
            // err
            assert(0);
        }
        exp = new IR::Binop(IR::binop::T_mod, lf.exp->ex, rf.exp->ex);
        return IR::ExpTy(new IR::Tr_Exp(exp), retType);
    }
}

IR::ExpTy AST::AddExp::ast2ir() {}
IR::ExpTy AST::RelExp::ast2ir() {}
IR::ExpTy AST::EqExp::ast2ir() {}
IR::ExpTy AST::LAndExp::ast2ir() {}
IR::ExpTy AST::LOrExp::ast2ir() {}
IR::ExpTy AST::IdExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label name) {
    std::string idname = *(this->str);
    assert(venv->exist(idname));
    TY::Entry* entry = venv->look(idname);
    IR::Exp *exp;
    TY::Type* t;
    if (entry->kind == TY::tyEntry::Ty_global)
    {
        exp = new IR::Name(static_cast<TY::GloVar*>(entry)->label);
    }
    else if(entry->kind == TY::tyEntry::Ty_local)
    {
        exp = new IR::Temp(static_cast<TY::LocVar*>(entry)->temp);
    }
    // assume that 
    
    if (name == "") {  // in global

    } else {  // in function

    }
    IR::ExpTy(new IR::Tr_Exp(exp),new TY::Type())
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
