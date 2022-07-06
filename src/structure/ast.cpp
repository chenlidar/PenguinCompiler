#include "ast.h"
#include "../util/utils.hpp"
#include "treeIR.hpp"

IR::StmList* AST::CompUnitList::ast2ir() {
    auto ret = new IR::StmList();
    for (const auto& it : list) {
        ret->push_back(it->ast2ir());
    }
    return ret;
}
IR::Stm* AST::FuncDef::ast2ir() {
    // TODO
}
IR::Stm* AST::Block::ast2ir() {
    auto ls = items->list;
    auto nowseq = nopStm();
    for (const auto& p : ls)
        nowseq = seq(nowseq, p->ast2ir());
    return nowseq;
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
    return IR::ExpTy(new IR::ConstInt(value), TY::Type(0, TY::tyType::Ty_int));
}
IR::ExpTy AST::FloatNumber::ast2ir() {
    // return IR::ExpTy(new IR::ConstFloat(), TY::Type(0,
    // TY::tyType::Ty_float));
}
IR::ExpTy AST::UnaryExp::ast2ir() {}
IR::ExpTy AST::CallExp::ast2ir() {}
IR::ExpTy AST::MulExp::ast2ir() {}
IR::ExpTy AST::AddExp::ast2ir() {}
IR::ExpTy AST::RelExp::ast2ir() {}
IR::ExpTy AST::EqExp::ast2ir() {}
IR::ExpTy AST::LAndExp::ast2ir() {}
IR::ExpTy AST::LOrExp::ast2ir() {}
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
