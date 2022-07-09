#include <stdio.h>
#include <string>
// #include "./frontend/parser.hpp"
#include "structure/ast.h"
#include "util/table.hpp"
extern int yyparse();
extern AST::CompUnitList* root;
int main() {
    yyparse();
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    IR::StmList * stmlist=root->ast2ir(venv,fenv);
    // for(const auto & stm:stmlist){
        
    // }
    return 0;
}