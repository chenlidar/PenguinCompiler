#include "ast.h"
#include "../util/utils.hpp"
#include "treeIR.hpp"
#include <assert.h>
#include <cstring>

IR::StmList* AST::CompUnitList::ast2ir(Table::Stable<TY::Entry*>* venv,
                                       Table::Stable<TY::EnFunc*>* fenv) {
    IR::StmList *ret, *tail;
    ret = tail = NULL;
    assert(list.size());
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
    IR::Stm* stm = nopStm();
    for (auto it : defs->list) { stm = seq(stm, it->ast2ir(btype, venv, fenv, name)); }
    return stm;
}
IR::Stm* AST::ConstDef::ast2ir(AST::btype_t btype, Table::Stable<TY::Entry*>* venv,
                               Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    switch (btype) {
    case AST::btype_t::INT: {
        if (index_list->list.empty()) {  // int
            IR::ExpTy expty = this->val->ast2ir(venv, fenv, name);
            assert(expty.ty->value);
            venv->enter(*this->id->str,
                        new TY::GloVar(TY::intType(expty.ty->value, true), Temp_newlabel()));
            return nopStm();
        } else {  // int []
            // get array index,make array type
            int offset = 0;
            TY::Type* head = TY::intType(new int(0), false);
            for (int i = (int)(this->index_list->list.size()) - 1; i >= 0; i--) {
                AST::Exp* it = this->index_list->list[i];
                IR::ExpTy expty = it->ast2ir(venv, fenv, name);
                assert(expty.ty->kind == TY::tyType::Ty_int);
                head = TY::arrayType(head, *expty.ty->value, true);
            }
            if (name == "") {
                Temp_Label label = Temp_newlabel();
                IR::ExpTy expty
                    = this->val->calArray(new IR::Name(label), 0, head, venv, fenv, name);
                venv->enter(*this->id->str, new TY::GloVar(expty.ty, label));
                return nopStm();
            } else {  // local const,same as valdef
                Temp_Temp tmp = Temp_newtemp();
                TY::EnFunc* enfunc = fenv->look(name);
                // local array in stk
                int stksize = enfunc->stksize + head->arraysize * 4;
                enfunc->stksize = stksize;
                IR::Stm* cat_stm = new IR::Move(new IR::Temp(tmp),
                                                new IR::Binop(IR::binop::T_plus, new IR::Temp(11),
                                                              new IR::ConstInt(-stksize)));
                IR::ExpList param = IR::ExpList();
                param.push_back(new IR::Temp(tmp));
                param.push_back(new IR::ConstInt(0));
                param.push_back(new IR::ConstInt(head->arraysize * 4));
                cat_stm
                    = seq(cat_stm, new IR::ExpStm(new IR::Call(new IR::Name("memset"), param)));
                assert(this->val);
                IR::ExpTy expty
                    = this->val->calArray(new IR::Temp(tmp), 0, head, venv, fenv, name);
                venv->enter(*this->id->str, new TY::LocVar(head, tmp));
                return seq(cat_stm, new IR::ExpStm(expty.exp->unEx()));
            }
        }

    } break;
    // case AST::btype_t::FLOAT: {
    //     assert(0);
    //     if (index_list->list.empty()) {  // float
    //         IR::ExpTy expty = this->val->ast2ir(venv, fenv, name);
    //         assert(expty.ty->value);
    //         int* val = new int(0);
    //         if (expty.ty->kind != TY::tyType::Ty_float) {  // int to float
    //             *val = f2i_encode((float)*expty.ty->value);
    //         } else
    //             *val = *expty.ty->value;
    //         venv->enter(*this->id->str,
    //                     new TY::GloVar(TY::floatType(expty.ty->value, true), Temp_newlabel()));
    //         return nopStm();
    //     } else {  // float []
    //         // get array index,make array type
    //         int offset = 0;
    //         TY::Type* head = TY::floatType(new int(0), true);
    //         for (int i = (int)(this->index_list->list.size()) - 1; i >= 0; i--) {
    //             AST::Exp* it = this->index_list->list[i];
    //             IR::ExpTy expty = it->ast2ir(venv, fenv, name);
    //             assert(expty.ty->kind == TY::tyType::Ty_int);
    //             head = TY::arrayType(head, *expty.ty->value, true);
    //         }
    //         if (name == "") {
    //             Temp_Label label = Temp_newlabel();
    //             IR::ExpTy expty
    //                 = this->val->calArray(new IR::Name(label), 0, head, venv, fenv, name);
    //             venv->enter(*this->id->str, new TY::GloVar(expty.ty, label));
    //             return nopStm();
    //         } else {  // local const,same as valdef,except must have initval
    //             Temp_Temp tmp = Temp_newtemp();
    //             TY::EnFunc* enfunc = fenv->look(name);
    //             // local array in stk
    //             int stksize = enfunc->stksize + head->arraysize * 4;
    //             enfunc->stksize = stksize;
    //             IR::Stm* cat_stm = new IR::Move(new IR::Temp(tmp),
    //                                             new IR::Binop(IR::binop::T_plus, new
    //                                             IR::Temp(11),
    //                                                           new IR::ConstInt(-stksize)));
    //             IR::ExpList param = IR::ExpList();
    //             param.push_back(new IR::Temp(tmp));
    //             param.push_back(new IR::ConstInt(0));
    //             param.push_back(new IR::ConstInt(head->arraysize * 4));
    //             cat_stm
    //                 = seq(cat_stm, new IR::ExpStm(new IR::Call(new IR::Name("memset"), param)));
    //             assert(this->val);  // must can be calculated
    //             IR::ExpTy expty
    //                 = this->val->calArray(new IR::Temp(tmp), 0, head, venv, fenv, name);
    //             venv->enter(*this->id->str, new TY::LocVar(head, tmp));
    //             return seq(cat_stm, new IR::ExpStm(expty.exp->unEx()));
    //         }
    //     }
    // } break;
    default: assert(0);
    }
}
IR::Stm* AST::VarDecl::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    IR::Stm* cat_stm = nopStm();
    for (auto it : defs->list) { cat_stm = seq(cat_stm, it->ast2ir(btype, venv, fenv, name)); }
    return cat_stm;
}
IR::Stm* AST::VarDef::ast2ir(btype_t btype, Table::Stable<TY::Entry*>* venv,
                             Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    switch (btype) {
    case AST::btype_t::INT: {
        if (name == "") {  // Global
            if (index->list.empty()) {  // int
                if (this->initval) {
                    IR::ExpTy expty = this->initval->ast2ir(venv, fenv, name);
                    assert(expty.ty->value);
                    venv->enter(*this->id->str, new TY::GloVar(TY::intType(expty.ty->value, false),
                                                               Temp_newlabel()));
                } else
                    venv->enter(*this->id->str,
                                new TY::GloVar(TY::intType(new int(0), false), Temp_newlabel()));
                return nopStm();
            } else {  // int []
                // get array index
                int offset = 0;
                TY::Type* head = TY::intType(new int(0), false);
                for (int i = (int)(this->index->list.size()) - 1; i >= 0; i--) {
                    AST::Exp* it = this->index->list[i];
                    IR::ExpTy expty = it->ast2ir(venv, fenv, name);
                    head = TY::arrayType(head, *expty.ty->value, false);
                }
                Temp_Label label = Temp_newlabel();
                if (this->initval) {
                    IR::ExpTy expty
                        = this->initval->calArray(new IR::Name(label), 0, head, venv, fenv, name);

                    venv->enter(*this->id->str, new TY::GloVar(head, label));
                } else {
                    head->value = new int[head->arraysize];
                    memset(head->value, 0, head->arraysize * sizeof(int));
                    venv->enter(*this->id->str, new TY::GloVar(head, label));
                }
                return nopStm();
            }
        } else {
            if (index->list.empty()) {  // int
                if (this->initval) {
                    IR::ExpTy expty = this->initval->ast2ir(venv, fenv, name);
                    Temp_Temp tmp = Temp_newtemp();
                    venv->enter(*this->id->str,
                                new TY::LocVar(TY::intType(expty.ty->value, false), tmp));
                    return new IR::Move(new IR::Temp(tmp), expty.exp->unEx());
                } else {
                    Temp_Temp tmp = Temp_newtemp();
                    venv->enter(*this->id->str,
                                new TY::LocVar(TY::intType(new int(0), false), tmp));
                    return nopStm();  // need not be 0
                }
            } else {  // int []
                // get array index
                int offset = 0;
                TY::Type* head = TY::intType(new int(0), false);
                for (int i = (int)(this->index->list.size()) - 1; i >= 0; i--) {
                    AST::Exp* it = this->index->list[i];
                    assert(it != nullptr);
                    IR::ExpTy expty = it->ast2ir(venv, fenv, name);
                    head = TY::arrayType(head, *expty.ty->value, false);
                }
                Temp_Temp tmp = Temp_newtemp();
                TY::EnFunc* enfunc = fenv->look(name);
                // local array in stk
                int stksize = enfunc->stksize + head->arraysize * 4;
                enfunc->stksize = stksize;
                IR::Stm* cat_stm = new IR::Move(new IR::Temp(tmp),
                                                new IR::Binop(IR::binop::T_plus, new IR::Temp(11),
                                                              new IR::ConstInt(-stksize)));
                // local array init value dont be defined need not be 0
                if (this->initval) {
                    IR::ExpList param = IR::ExpList();
                    param.push_back(new IR::Temp(tmp));
                    param.push_back(new IR::ConstInt(0));
                    param.push_back(new IR::ConstInt(head->arraysize * 4));
                    cat_stm = seq(cat_stm,
                                  new IR::ExpStm(new IR::Call(new IR::Name("memset"), param)));
                    IR::ExpTy expty
                        = this->initval->calArray(new IR::Temp(tmp), 0, head, venv, fenv, name);
                    venv->enter(*this->id->str, new TY::LocVar(head, tmp));
                    return seq(cat_stm, new IR::ExpStm(expty.exp->unEx()));
                } else {
                    head->value = new int[head->arraysize];
                    memset(head->value, 0, head->arraysize * sizeof(int));
                    venv->enter(*this->id->str, new TY::LocVar(head, tmp));
                    return cat_stm;
                }
            }
        }

    } break;
    default: assert(0);
    }
}
IR::Stm* AST::FuncDef::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    TY::Type* ty = TY::funcType(NULL, std::vector<TY::Type*>());
    ty->tp = typeAst2ir(this->btype);
    for (AST::Parameter* it : this->parameters->list) {
        if (it->arrayindex->list.empty())
            ty->param.push_back(typeAst2ir(it->btype));
        else {  // FIXME: only int[]
            TY::Type* head = TY::intType(new int(0), false);
            for (int i = (int)(it->arrayindex->list.size()) - 1; i >= 0; i--) {
                AST::Exp* exp = it->arrayindex->list[i];
                head = TY::arrayType(head, 0, false);
            }
            head->value = new int[head->arraysize];
            memset(head->value, 0, head->arraysize * sizeof(int));
            if (it->arrayindex->list.empty()) ty->param.push_back(head);
        }
    }
    name = *this->id->str;
    Temp_Label fb = name;
    fenv->enter(fb, new TY::EnFunc(ty, fb));
    venv->beginScope(NULL);
    IR::Stm* cat_stm = new IR::Label(fb);
    int stksize = 0, cnt = 0;
    for (AST::Parameter* it : this->parameters->list) {
        AST::VarDef(it->id, it->arrayindex, NULL, it->lineno).ast2ir(it->btype, venv, fenv, name);
        TY::LocVar* entry
            = static_cast<TY::LocVar*>(venv->look(*static_cast<AST::IdExp*>(it->id)->str));
        if (cnt < 4) {
            cat_stm = seq(cat_stm, new IR::Move(new IR::Temp(entry->temp), new IR::Temp(cnt)));
            cnt++;
        } else {
            cat_stm
                = seq(cat_stm,
                      new IR::Move(new IR::Temp(entry->temp),
                                   new IR::Mem(new IR::Binop(IR::binop::T_plus, new IR::Temp(11),
                                                             new IR::ConstInt(stksize)))));
            stksize += 4;
        }  // low ..now stack..fp 5 6 7 8 ... high
    }
    IR::Stm* stm = this->block->ast2ir(venv, fenv, brelabel, conlabel, name);
    venv->endScope();
    return seq(cat_stm, stm);
}
IR::Stm* AST::Block::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    venv->beginScope(NULL);
    IR::Stm* cat_stm = nopStm();
    for (AST::BlockItem*& it : this->items->list) {
        IR::Stm* stm = it->ast2ir(venv, fenv, brelabel, conlabel, name);
        cat_stm = seq(cat_stm, stm);
    }
    venv->endScope();
    return cat_stm;
}
IR::Stm* AST::AssignStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                 Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    IR::ExpTy l = this->lval->ast2ir(venv, fenv, name);
    IR::ExpTy r = this->exp->ast2ir(venv, fenv, name);
    IR::Exp* lexp = l.exp->unEx();
    IR::Exp* rexp = r.exp->unEx();
    return new IR::Move(lexp, rexp);
}
IR::Stm* AST::ExpStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                              Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(this->exp->ast2ir(venv, fenv, name).exp->unEx());
}
IR::Stm* AST::IfStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    IR::Cx ct = this->exp->ast2ir(venv, fenv, name).exp->unCx();
    Temp_Label tlabel = Temp_newlabel(), flabel = Temp_newlabel(), done = Temp_newlabel();
    doPatch(ct.trues, tlabel);
    doPatch(ct.falses, flabel);
    auto ifstm = this->if_part->ast2ir(venv, fenv, brelabel, conlabel, name);
    auto elsestm = nopStm();
    if (this->else_part) elsestm = this->else_part->ast2ir(venv, fenv, brelabel, conlabel, name);
    return seq(
        ct.stm,
        seq(new IR::Label(tlabel),
            seq(ifstm, seq(new IR::Jump(new IR::Name(done), Temp_LabelList(1, done)),
                           seq(new IR::Label(flabel), seq(elsestm, new IR::Label(done)))))));
}
IR::Stm* AST::WhileStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    IR::Cx ct = this->exp->ast2ir(venv, fenv, name).exp->unCx();
    Temp_Label begin = Temp_newlabel(), test = Temp_newlabel(), done = Temp_newlabel();
    doPatch(ct.trues, begin);
    doPatch(ct.falses, done);
    return seq(new IR::Label(test),
               seq(ct.stm, seq(new IR::Label(begin),
                               seq(this->loop->ast2ir(venv, fenv, done, test, name),
                                   seq(new IR::Jump(new IR::Name(test), Temp_LabelList(1, test)),
                                       new IR::Label(done))))));
}
IR::Stm* AST::BreakStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    return new IR::Jump(new IR::Name(brelabel), Temp_LabelList(1, brelabel));
}
IR::Stm* AST::ContinueStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                   Temp_Label conlabel, Temp_Label name) {
    return new IR::Jump(new IR::Name(conlabel), Temp_LabelList(1, conlabel));
}
IR::Stm* AST::ReturnStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                 Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    if (this->exp == nullptr) {
        return new IR::Jump(new IR::Name("RETURN"), Temp_LabelList(1, "RETURN"));
    } else
        return seq(new IR::Move(new IR::Temp(0), this->exp->ast2ir(venv, fenv, name).exp->unEx()),
                   new IR::Jump(new IR::Name("RETURN"), Temp_LabelList(1, "RETURN")));
}
IR::ExpTy AST::Exp::calArray(IR::Exp* addr, int noff, TY::Type* ty,
                             Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label name) {
    IR::ExpTy expty = this->ast2ir(venv, fenv, name);
    IR::Exp* exp = expty.exp->unEx();
    IR::Stm* stm = new IR::Move(
        new IR::Mem(new IR::Binop(IR::binop::T_plus, addr, new IR::ConstInt(4 * noff))), exp);
    return IR::ExpTy(new IR::Tr_Exp(new IR::Eseq(stm, new IR::ConstInt(0))),
                     expty.ty);  // not use exp
}
IR::ExpTy AST::InitValList::calArray(IR::Exp* addr, int noff, TY::Type* ty,
                                     Table::Stable<TY::Entry*>* venv,
                                     Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    ty->value = new int[ty->arraysize];
    memset(ty->value, 0, ty->arraysize * sizeof(int));
    int doff = 0;
    IR::Stm* cat_stm = nopStm();
    for (const auto& it : this->list) {
        IR::ExpTy expty = it->calArray(addr, noff + doff, ty->tp, venv, fenv, name);
        IR::Stm* stm = new IR::ExpStm(expty.exp->unEx());
        cat_stm = seq(cat_stm, stm);
        if (expty.ty->tp == NULL) {
            ty->value[doff] = *expty.ty->value;
        } else {
            std::memcpy(&ty->value[doff], expty.ty->value, expty.ty->arraysize * 4);
        }
        doff += expty.ty->arraysize;
    }
    return IR::ExpTy(new IR::Tr_Exp(new IR::Eseq(cat_stm, addr)), ty);
}
IR::ExpTy AST::Lval::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label name) {
    std::string idname = *this->id->str;
    assert(venv->exist(idname));
    TY::Entry* exp = venv->look(idname);
    IR::Exp* idIR;
    if (exp->kind == TY::tyEntry::Ty_global) {
        idIR = new IR::Name(static_cast<TY::GloVar*>(exp)->label);
    } else if (exp->kind == TY::tyEntry::Ty_local) {
        idIR = new IR::Temp(static_cast<TY::LocVar*>(exp)->temp);
    }

    TY::Type* ty = exp->ty;
    if (!this->arrayindex->list.empty()) {  // array
        IR::Exp* e = new IR::ConstInt(0);
        int offset = 0;
        if (ty->isconst && 0) {  // NOTE: FIXME: offset may not const
            int* an = ty->value;
            for (AST::Exp* it : this->arrayindex->list) {
                IR::ExpTy expty = it->ast2ir(venv, fenv, name);
                int dim = ty->dim;
                offset = offset * dim + *expty.ty->value;  //
                ty = ty->tp;
            }
            TY::Type* LvalType = TY::intType(new int(an[offset]), 0);
            return IR::ExpTy(new IR::Tr_Exp(new IR::ConstInt(an[offset])), LvalType);
        } else {
            for (AST::Exp* it : this->arrayindex->list) {
                IR::ExpTy expty = it->ast2ir(venv, fenv, name);
                int dim = ty->dim;
                e = new IR::Binop(IR::binop::T_plus,
                                  new IR::Binop(IR::binop::T_mul, new IR::ConstInt(dim), e),
                                  expty.exp->unEx());

                offset = offset * dim + *expty.ty->value;
                ty = ty->tp;
            }
            if (ty->kind == TY::tyType::Ty_array) {
                TY::Type* LvalType = ty;
                return IR::ExpTy(
                    new IR::Tr_Exp(new IR::Binop(
                        IR::binop::T_plus, idIR,
                        new IR::Binop(IR::binop::T_mul, new IR::ConstInt(4 * ty->arraysize), e))),
                    LvalType);
            } else {
                TY::Type* LvalType = TY::intType(
                    new int(exp->ty->value[(offset % exp->ty->arraysize + exp->ty->arraysize)
                                           % exp->ty->arraysize]),
                    0);
                return IR::ExpTy(new IR::Tr_Exp(new IR::Mem(new IR::Binop(
                                     IR::binop::T_plus, idIR,
                                     new IR::Binop(IR::binop::T_mul, new IR::ConstInt(4), e)))),
                                 LvalType);
            }
        }

    } else {  // not array
        if (ty->isconst)
            return IR::ExpTy(new IR::Tr_Exp(new IR::ConstInt(*ty->value)), ty);
        else {
            if (exp->kind == TY::tyEntry::Ty_global) {
                if (ty->kind == TY::tyType::Ty_array) {
                    return IR::ExpTy(new IR::Tr_Exp(idIR), ty);
                } else {
                    return IR::ExpTy(new IR::Tr_Exp(new IR::Mem(idIR)), ty);
                }
            } else
                return IR::ExpTy(new IR::Tr_Exp(idIR), ty);
        }
    }
}
IR::ExpTy AST::IntNumber::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                 Temp_Label name) {
    auto exp = new IR::Tr_Exp(new IR::ConstInt(value));
    return IR::ExpTy(exp, TY::intType(new int(this->value), 0));
}
IR::ExpTy AST::FloatNumber::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
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
        exp = new IR::Binop(IR::binop::T_minus, new IR::ConstInt(0), body.exp->unEx());
        body.ty->value[0] = -body.ty->value[0];
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
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(new IR::Name(func->label), list)), func->ty->tp);
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
    TY::Type* t = TY::intType(new int(0), false);
    IR::RelOp bop;
    switch (this->op) {
    case AST::rel_t::GE: bop = IR::RelOp::T_ge; break;
    case AST::rel_t::GT: bop = IR::RelOp::T_gt; break;
    case AST::rel_t::LE: bop = IR::RelOp::T_le; break;
    case AST::rel_t::LT: bop = IR::RelOp::T_lt; break;
    default: assert(0);
    }
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cjump* stm = new IR::Cjump(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx(), "", "");
    IR::PatchList* trues = new IR::PatchList(&stm->trueLabel, NULL);
    IR::PatchList* falses = new IR::PatchList(&stm->falseLabel, NULL);
    return IR::ExpTy(new IR::Tr_Exp(trues, falses, stm), t);
}
IR::ExpTy AST::EqExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                             Temp_Label name) {
    TY::Type* t = TY::intType(new int(0), false);
    IR::RelOp bop;
    switch (this->op) {
    case AST::equal_t::EQ: bop = IR::RelOp::T_eq; break;
    case AST::equal_t::NE: bop = IR::RelOp::T_ne; break;
    default: assert(0);
    }
    IR::ExpTy lExpTy = this->lhs->ast2ir(venv, fenv, name);
    IR::ExpTy rExpTy = this->rhs->ast2ir(venv, fenv, name);
    IR::Cjump* stm = new IR::Cjump(bop, lExpTy.exp->unEx(), rExpTy.exp->unEx(), "", "");
    IR::PatchList* trues = new IR::PatchList(&stm->trueLabel, NULL);
    IR::PatchList* falses = new IR::PatchList(&stm->falseLabel, NULL);
    return IR::ExpTy(new IR::Tr_Exp(trues, falses, stm), t);
}
IR::ExpTy AST::LAndExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                               Temp_Label name) {
    IR::Exp* exp;
    TY::Type* t = TY::intType(new int(0), false);
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
    TY::Type* t = TY::intType(new int(0), false);
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
    if (t->isconst)
        return IR::ExpTy(new IR::Tr_Exp(new IR::ConstInt(*t->value)),
                         TY::intType(new int(*t->value), false));
    if (entry->kind == TY::tyEntry::Ty_global) {
        exp = new IR::Name(static_cast<TY::GloVar*>(entry)->label);
    } else if (entry->kind == TY::tyEntry::Ty_local) {
        exp = new IR::Temp(static_cast<TY::LocVar*>(entry)->temp);
    }
    // assume that array is not in "IdExp"
    return IR::ExpTy(new IR::Tr_Exp(exp), t);
}
IR::Stm* AST::PutintStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                 Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(new IR::Call(
        new IR::Name("putint"), IR::ExpList(1, this->exp->ast2ir(venv, fenv, name).exp->unEx())));
}
IR::Stm* AST::PutchStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(new IR::Call(
        new IR::Name("putch"), IR::ExpList(1, this->exp->ast2ir(venv, fenv, name).exp->unEx())));
}
IR::Stm* AST::PutarrayStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                   Temp_Label conlabel, Temp_Label name) {
    IR::ExpList v = IR::ExpList(1, this->len->ast2ir(venv, fenv, name).exp->unEx());
    v.push_back(this->arr->ast2ir(venv, fenv, name).exp->unEx());
    return new IR::ExpStm(new IR::Call(new IR::Name("putarray"), v));
}
IR::Stm* AST::PutfloatStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                   Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(
        new IR::Call(new IR::Name("putfloat"),
                     IR::ExpList(1, this->exp->ast2ir(venv, fenv, name).exp->unEx())));
}
IR::Stm* AST::PutfarrayStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                    Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                    Temp_Label conlabel, Temp_Label name) {
    IR::ExpList v = IR::ExpList(1, this->len->ast2ir(venv, fenv, name).exp->unEx());
    v.push_back(this->arr->ast2ir(venv, fenv, name).exp->unEx());
    return new IR::ExpStm(new IR::Call(new IR::Name("putfarray"), v));
}
IR::Stm* AST::PutfStmt::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                               Temp_Label brelabel, Temp_Label conlabel, Temp_Label name) {
    assert(0);
    return NULL;
}
IR::Stm* AST::StarttimeStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                    Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                    Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(new IR::Call(new IR::Name("_sysy_starttime"),
                                       IR::ExpList(1, new IR::ConstInt(this->lineno))));
}
IR::Stm* AST::StoptimeStmt::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label brelabel,
                                   Temp_Label conlabel, Temp_Label name) {
    return new IR::ExpStm(new IR::Call(new IR::Name("_sysy_stoptime"),
                                       IR::ExpList(1, new IR::ConstInt(this->lineno))));
}
IR::ExpTy AST::GetintExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                 Temp_Label name) {
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(new IR::Name("getint"), IR::ExpList())),
                     TY::intType(new int(0), false));
}
IR::ExpTy AST::GetchExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                                Temp_Label name) {
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(new IR::Name("getch"), IR::ExpList())),
                     TY::intType(new int(0), false));
}
IR::ExpTy AST::GetfloatExp::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(new IR::Name("getfloat"), IR::ExpList())),
                     TY::floatType(NULL, false));
}
IR::ExpTy AST::GetarrayExp::ast2ir(Table::Stable<TY::Entry*>* venv,
                                   Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(
                         new IR::Name("getarray"),
                         IR::ExpList(1, this->arr->ast2ir(venv, fenv, name).exp->unEx()))),
                     TY::intType(new int(0), false));
}
IR::ExpTy AST::GetfarrayExp::ast2ir(Table::Stable<TY::Entry*>* venv,
                                    Table::Stable<TY::EnFunc*>* fenv, Temp_Label name) {
    return IR::ExpTy(new IR::Tr_Exp(new IR::Call(
                         new IR::Name("getfarray"),
                         IR::ExpList(1, this->arr->ast2ir(venv, fenv, name).exp->unEx()))),
                     TY::intType(new int(0), false));
}
