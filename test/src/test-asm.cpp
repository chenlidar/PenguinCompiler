#include <stdio.h>
#include <string>
// #include "./frontend/parser.hpp"
#include "structure/ast.h"
#include "util/table.hpp"
#include "backend/canon.hpp"
#include "backend/regalloc.hpp"
#include "util/utils.hpp"
extern int yyparse();
extern AST::CompUnitList* root;
int main() {
    yyparse();
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    std::cout << ".arch armv7ve\n.arm\n.global main\n.text\n";
    IR::StmList* stmlist = root->ast2ir(venv, fenv);
    for (IR::StmList* l = stmlist; l; l = l->tail) {
        IR::Stm* stm = l->stm;
        if (isNop(stm)) continue;  // global var
        //function
        IR::StmList* out = CANON::handle(stm);
        std::string funcname = static_cast<IR::Label*>(out->stm)->label;
        assert(fenv->exist(funcname));
        bool isvoid = fenv->look(funcname)->ty->tp->kind == TY::tyType::Ty_void;
        RA::RA_RegAlloc(CANON::funcEntryExit2(&IR::ir2asm(out)->body, isvoid, funcname == "main"));
        // for(auto it:*CANON::funcEntryExit2(&IR::ir2asm(out)->body, isvoid, funcname == "main")){
        //     it->print();
        // }
    }
    // global var handle
    std::cout<<".data\n";
    for(auto it=venv->begin();it!=venv->end();++it){
        TY::Entry * entry=venv->look(it->first);
        assert(entry&&entry->kind==TY::tyEntry::Ty_global);
        if(entry->ty->isconst&&entry->ty->kind!=TY::tyType::Ty_array)continue;//const
        Temp_Label name=static_cast<TY::GloVar*>(entry)->label;
        switch(entry->ty->kind){
            case TY::tyType::Ty_int:{
                assert(entry->ty->value);
                std::cout<<name+":\n"+".word "+std::to_string(*entry->ty->value)<<std::endl;
            }break;
            case TY::tyType::Ty_float:{
                assert(0);
            }break;
            case TY::tyType::Ty_array:{//int
                std::cout<<name+":\n";
                for(int i=0;i<entry->ty->arraysize;i++){
                    std::cout<<".word "+std::to_string(*(entry->ty->value+i))<<std::endl;
                }
            }break;
            default:assert(0);
        }
    }
    return 0;
}