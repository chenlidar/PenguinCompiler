#include <stdio.h>
#include <string>
// #include "./frontend/parser.hpp"
#include "structure/ast.h"
#include "util/table.hpp"
#include "backend/canon.hpp"
extern int yyparse();
extern AST::CompUnitList* root;
int main() {
    yyparse();
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    IR::StmList* stmlist = root->ast2ir(venv, fenv);
    for (IR::StmList* l = stmlist; l; l = l->tail) {
        IR::Stm* stm = l->stm;
        IR::StmList* out = CANON::handle(stm);
        IR::ir2asm(out)->print();
    }
    return 0;
}