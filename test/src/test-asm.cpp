#include <stdio.h>
#include <string>
#include <iostream>
// #include "./frontend/parser.hpp"
#include "structure/ast.h"
#include "util/table.hpp"
#include "backend/canon.hpp"
#include "backend/regalloc.hpp"
#include "util/utils.hpp"
#include "ssa/quadruple.hpp"
#include <sstream>
#include <typeinfo>
extern int yyparse();
extern AST::CompUnitList* root;
IR::StmList* handleGlobalVar(std::ostringstream* globalVar, std::ostringstream* globalArray,
                             Table::Stable<TY::Entry*>* venv) {
    // global var handle
    *globalVar << ".data\n";
    *globalArray << ".bss\n";
    IR::StmList *arrayInitstm = new IR::StmList(nullptr, nullptr), *tail;
    tail = arrayInitstm;
    for (auto it = venv->begin(); it != venv->end(); ++it) {
        TY::Entry* entry = venv->look(it->first);
        assert(entry && entry->kind == TY::tyEntry::Ty_global);
        if (entry->ty->isconst && entry->ty->kind != TY::tyType::Ty_array) continue;  // const
        Temp_Label name = static_cast<TY::GloVar*>(entry)->label;
        switch (entry->ty->kind) {
        case TY::tyType::Ty_int: {
            assert(entry->ty->value);
            *globalVar << name + ":\n" + ".word " + std::to_string(*entry->ty->value) << std::endl;
        } break;
        case TY::tyType::Ty_float: {
            assert(0);
        } break;
        case TY::tyType::Ty_array: {  // int
            *globalArray << name + ":\n";
            *globalArray << ".space " + std::to_string(entry->ty->arraysize*4) << std::endl;
            for (int i = 0; i < entry->ty->arraysize; i++) {
                if(*(entry->ty->value + i)==0)continue;
                tail = tail->tail = new IR::StmList(
                    new IR::Move(new IR::Mem(new IR::Binop(IR::binop::T_plus, new IR::Name(name),
                                                           new IR::ConstInt(4 * i))),
                                 new IR::ConstInt(*(entry->ty->value + i))),
                    nullptr);
            }
        } break;
        default: assert(0);
        }
    }
    return arrayInitstm->tail;
}
int main() {
    yyparse();
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    std::cout << ".arch armv7ve\n.arm\n.global main\n.text\n";
    std::ostringstream* globalVar = new std::ostringstream();
    std::ostringstream* globalArray = new std::ostringstream();
    for (const auto& ast_stm : root->list) {
        IR::Stm* stm = ast_stm->ast2ir(venv, fenv, "", "", "");
        if (typeid(*ast_stm) != typeid(AST::FuncDef)) continue;  // global var
        // function
        std::string funcname = *static_cast<AST::FuncDef*>(ast_stm)->id->str;
        assert(fenv->exist(funcname));

        if (funcname == "main") {
            IR::StmList* initarray = handleGlobalVar(globalVar, globalArray, venv);
            IR::StmList* out = CANON::linearize(stm);
            if (initarray) {
                getEnd(initarray)->tail = out->tail;
                out->tail = initarray;
            }
            IR::Stm* stmq = QUADRUPLE::handle(out);
            out = CANON::handle(stmq);
            int stksize = fenv->look(funcname)->stksize;
            RA::RA_RegAlloc(CANON::funcEntryExit2(&IR::ir2asm(out)->body, false, true), stksize);
            std::cout << globalVar->str();
            std::cout << globalArray->str();
            break;  // function, global var behind main will never used;
        } else {
            IR::StmList* out = CANON::linearize(stm);
            IR::Stm* stmq = QUADRUPLE::handle(out);
            out = CANON::handle(stmq);
            bool isvoid = fenv->look(funcname)->ty->tp->kind == TY::tyType::Ty_void;
            int stksize = fenv->look(funcname)->stksize;
            RA::RA_RegAlloc(CANON::funcEntryExit2(&IR::ir2asm(out)->body, isvoid, false), stksize);
        }
    }

    return 0;
}