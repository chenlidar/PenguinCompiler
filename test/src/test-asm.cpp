#include <stdio.h>
#include <string>
// #include "./frontend/parser.hpp"
#include "structure/ast.h"
#include "util/table.hpp"
#include "backend/canon.hpp"
#include "backend/regalloc.hpp"
extern int yyparse();
extern AST::CompUnitList* root;
int main() {
    yyparse();
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    std::cout << ".global main\n";
    //global var handle

    IR::StmList* stmlist = root->ast2ir(venv, fenv);
    for (IR::StmList* l = stmlist; l; l = l->tail) {
        IR::Stm* stm = l->stm;
        IR::StmList* out = CANON::handle(stm);
        std::string funcname = static_cast<IR::Label*>(out->stm)->label;
        if (!fenv->exist(funcname))continue;//global var
        else {//function
            bool isvoid = fenv->look(funcname)->ty->tp->kind == TY::tyType::Ty_void;
            RA::RA_RegAlloc(
                CANON::funcEntryExit2(&IR::ir2asm(out)->body, isvoid, funcname == "main"));
        }
        // for(auto it:*CANON::funcEntryExit2(&IR::ir2asm(out)->body, isvoid, funcname == "main")){
        //     it->print();
        // }
    }
    return 0;
}