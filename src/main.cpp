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
#include "util/showIR.hpp"
#include "ssa/BuildSSA.hpp"
#include "ssa/FuncInline.hpp"
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
        if (entry->ty->isconst) {
            assert(entry->ty->kind != TY::tyType::Ty_array);
            continue;  // const
        }
        Temp_Label name = static_cast<TY::GloVar*>(entry)->label;
        switch (entry->ty->kind) {
        case TY::tyType::Ty_int: {
            assert(entry->ty->value);
            *globalVar << name + ":\n" + ".word " + std::to_string(*entry->ty->value) << std::endl;
        } break;
        case TY::tyType::Ty_float: {
            assert(entry->ty->value);
            *globalVar << name + ":\n" + ".word " + std::to_string(*entry->ty->value) << std::endl;
        } break;
        case TY::tyType::Ty_array: {  // int
            *globalArray << name + ":\n";
            *globalArray << ".space " + std::to_string(entry->ty->arraysize * 4) << std::endl;
            Temp_Label tl = Temp_newlabel(), fl = Temp_newlabel();
            Temp_Temp itemp = Temp_newtemp();
            tail->tail = new IR::StmList(
                new IR::Move(new IR::Temp(itemp), new IR::Const(entry->ty->arraysize - 1)),
                new IR::StmList(
                    new IR::Label(tl),
                    new IR::StmList(
                        new IR::Move(new IR::Mem(new IR::Binop(
                                         IR::binop::T_plus, new IR::Name(name),
                                         new IR::Binop(IR::binop::T_mul, new IR::Const(4),
                                                       new IR::Temp(itemp)))),
                                     new IR::Const(0)),
                        new IR::StmList(
                            new IR::Move(new IR::Temp(itemp),
                                         new IR::Binop(IR::binop::T_minus, new IR::Temp(itemp),
                                                       new IR::Const(1))),
                            new IR::StmList(new IR::Cjump(IR::RelOp::T_ge, new IR::Temp(itemp),
                                                          new IR::Const(0), tl, fl),
                                            new IR::StmList(new IR::Label(fl), nullptr))))));
            tail = getEnd(tail);
            for (int i = 0; i < entry->ty->arraysize; i++) {
                if (*(entry->ty->value + i) == 0) continue;
                tail = tail->tail = new IR::StmList(
                    new IR::Move(new IR::Mem(new IR::Binop(IR::binop::T_plus, new IR::Name(name),
                                                           new IR::Const(4 * i))),
                                 new IR::Const(*(entry->ty->value + i))),
                    nullptr);
            }
        } break;
        default: assert(0);
        }
    }
    return arrayInitstm->tail;
}
void initFenv(Table::Stable<TY::EnFunc*>* fenv) {
    auto inttype = TY::intType(new int(0), false);
    auto floattype = TY::floatType(new int(0), false);
    auto voidtype = TY::voidType();
    fenv->enter("getint",
                new TY::EnFunc(TY::funcType(inttype, std::vector<TY::Type*>()), "getint"));
    fenv->enter("getch", new TY::EnFunc(TY::funcType(inttype, std::vector<TY::Type*>()), "getch"));
    fenv->enter("getfloat",
                new TY::EnFunc(TY::funcType(floattype, std::vector<TY::Type*>()), "getfloat"));
    fenv->enter("getarray",
                new TY::EnFunc(TY::funcType(inttype, std::vector<TY::Type*>(
                                                         1, TY::arrayType(inttype, 1, false))),
                               "getarray"));
    fenv->enter("getfarray",
                new TY::EnFunc(TY::funcType(inttype, std::vector<TY::Type*>(
                                                         1, TY::arrayType(floattype, 1, false))),
                               "getfarray"));
    fenv->enter(
        "putint",
        new TY::EnFunc(TY::funcType(voidtype, std::vector<TY::Type*>(1, inttype)), "putint"));
    fenv->enter("putch", new TY::EnFunc(TY::funcType(voidtype, std::vector<TY::Type*>(1, inttype)),
                                        "putch"));
    fenv->enter(
        "putfloat",
        new TY::EnFunc(TY::funcType(voidtype, std::vector<TY::Type*>(1, floattype)), "putfloat"));
    fenv->enter(
        "putarray",
        new TY::EnFunc(TY::funcType(voidtype, std::vector<TY::Type*>(
                                                  {inttype, TY::arrayType(inttype, 1, false)})),
                       "putarray"));
    fenv->enter(
        "putfarray",
        new TY::EnFunc(TY::funcType(voidtype, std::vector<TY::Type*>(
                                                  {inttype, TY::arrayType(floattype, 1, false)})),
                       "putfarray"));
}
int main(int argc, char** argv) {
    // compiler testcase.sysy -S -o testcase.s [-O1]
    char* sy_input = 0;
    char* sy_output = 0;

    /* Get output */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'o') {
                sy_output = argv[i + 1];
                break;
            }
        }
    }

    /* Get intput */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && argv[i] != sy_output) { sy_input = argv[i]; }
    }
    assert(sy_input);
    assert(sy_output);
    freopen(sy_input, "r", stdin);
    yyparse();
    freopen(sy_output, "w", stdout);
    auto venv = new Table::Stable<TY::Entry*>();
    auto fenv = new Table::Stable<TY::EnFunc*>();
    initFenv(fenv);
    std::cout << ".arch armv7ve\n.arm\n.global main\n";
    std::ostringstream* globalVar = new std::ostringstream();
    std::ostringstream* globalArray = new std::ostringstream();
    std::unordered_map<std::string, IR::StmList*> func_ir;
    std::vector<std::string> func_p;
    // to ir
    for (const auto& ast_stm : root->list) {
        IR::Stm* stm = ast_stm->ast2ir(venv, fenv, "", "", "");
        if (typeid(*ast_stm) != typeid(AST::FuncDef)) continue;  // global var
        // function
        std::string funcname = *static_cast<AST::FuncDef*>(ast_stm)->id->str;
        assert(fenv->exist(funcname));
        bool ismain = funcname == "main";
        IR::StmList* out = CANON::linearize(stm);
        if (ismain) {
            IR::StmList* initarray = handleGlobalVar(globalVar, globalArray, venv);
            if (initarray) {
                getEnd(initarray)->tail = out->tail;
                out->tail = initarray;
            }
        }
        IR::Stm* stmq = QUADRUPLE::handle(out);
        out = CANON::linearize(stmq);
        func_p.push_back(funcname);
        func_ir.insert({funcname, out});
        if (ismain) {
            std::cout << globalVar->str();
            std::cout << globalArray->str();
            break;  // function, global var behind main will never used;
        }
    }
    std::cout << ".text\n";
    // do function inline in this place
    INTERP::FuncInline finline(venv, fenv, func_p, func_ir);
    std::vector<std::string> procs = finline.functionInline();
    for (auto funcname : procs) {
        IR::StmList* ir = finline.func_info[funcname].ir;
        CANON::Block blocks = CANON::basicBlocks(ir, funcname);
        bool isvoid = finline.func_info[funcname].isvoid;
        int stksize = finline.func_info[funcname].stksize;
        bool ismain = funcname == "main";
        // do ssa in this place
        SSA::SSAIR* ssa = new SSA::SSAIR(blocks);

        for (int i = 0; i < 6; i++) {

            ssa->opt.deadCodeElimilation();
            ssa->opt.constantPropagation();
            ssa->opt.combExp();
            ssa->opt.deadCodeElimilation();
            ssa->opt.constantPropagation();
            ssa->opt.PRE();
            ssa->opt.constantPropagation();
            ssa->opt.deadCodeElimilation();
            ssa->opt.CME();
            ssa->opt.constantPropagation();
            ssa->opt.deadCodeElimilation();
            ssa->mergeNode();
            ssa->loops.loopUnroll();
            ssa->opt.constantPropagation();
            ssa->opt.deadCodeElimilation();
            // ssa->showmark();
            // std::cerr << "-----------------\n";
            ssa->opt.strengthReduction();
            // ssa->showmark();
            // std::cerr << "-----------------\n";
            ssa->opt.constantPropagation();
            ssa->opt.deadCodeElimilation();
        }
        blocks = ssa->ssa2ir();
        ir = CANON::traceSchedule(blocks);
        //
        // if (ismain) showir(ir);
        RA::RA_RegAlloc(CANON::funcEntryExit2(&IR::ir2asm(ir)->body, isvoid, ismain), stksize);
    }

    return 0;
}