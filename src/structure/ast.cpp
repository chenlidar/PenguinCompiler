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
