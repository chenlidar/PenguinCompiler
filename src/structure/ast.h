#ifndef __AST_H
#define __AST_H

#include <string>
#include <vector>
#include "treeIR.hpp"
#include "../util/table.hpp"

namespace AST {

enum class rel_t { LE, LT, GE, GT };

enum class equal_t { EQ, NE };

enum class mul_t { MULT, DIV, REM };

enum class unaryop_t { ADD, SUB, NOT };

enum class btype_t { VOID, INT, FLOAT };

enum class addop_t { ADD, SUB };

enum class constop_t { ADD, SUB, MUL, DIV, REM, NOT };

struct CompUnitList;
struct CompUnit;
struct Decl;
struct ConstDecl;
struct ConstDefList;
struct ConstDef;
struct VarDecl;
struct VarDefList;
struct VarDef;
struct ArrayIndex;
struct InitVal;
struct ArrayInit;
struct InitValList;
struct FuncDef;
struct Parameters;
struct Parameter;
struct Block;
struct BlockItemList;
struct BlockItem;
struct Stmt;
struct AssignStmt;
struct ExpStmt;
struct IfStmt;
struct WhileStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct Exp;
struct IdExp;
struct Lval;
struct PrimaryExp;
struct Number;
struct IntNumber;
struct FloatNumber;
struct UnaryExp;
struct CallExp;
struct ExpList;
struct MulExp;
struct AddExp;
struct RelExp;
struct EqExp;
struct LAndExp;
struct LOrExp;

struct ConstVal {
    bool is_const;
    bool is_int;

    /*
     * const int b[2] = {0, 1};
     * int a[b[1]] = {};
     * b[1] cannot be directly calculated from AST
     */
    bool can_cal_from_ast;
    union {
        int i;
        float f;
    } val;

    ConstVal()
        : is_const(false) {}
    ConstVal(int i)
        : is_const(true)
        , is_int(true)
        , can_cal_from_ast(true) {
        val.i = i;
    }
    ConstVal(float f)
        : is_const(true)
        , is_int(false)
        , can_cal_from_ast(true) {
        val.f = f;
    }

    /* Pass nullptr to set can_cal_from_ast to false */
    ConstVal(void* x)
        : is_const(true)
        , can_cal_from_ast(false) {}
};

static ConstVal calconst(const ConstVal& lhs, constop_t op, const ConstVal& rhs) {
    if (!lhs.is_const || !rhs.is_const) { return ConstVal(); }

    if (!lhs.can_cal_from_ast || !rhs.can_cal_from_ast) { return ConstVal(nullptr); }

    switch (op) {
    case constop_t::ADD:
        if (lhs.is_int && rhs.is_int) return ConstVal(lhs.val.i + rhs.val.i);
        if (lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.i + rhs.val.f);
        if (!lhs.is_int && rhs.is_int) return ConstVal(lhs.val.f + rhs.val.i);
        if (!lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.f + rhs.val.f);
        break;
    case constop_t::SUB:
        if (lhs.is_int && rhs.is_int) return ConstVal(lhs.val.i - rhs.val.i);
        if (lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.i - rhs.val.f);
        if (!lhs.is_int && rhs.is_int) return ConstVal(lhs.val.f - rhs.val.i);
        if (!lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.f - rhs.val.f);
        break;
    case constop_t::MUL:
        if (lhs.is_int && rhs.is_int) return ConstVal(lhs.val.i * rhs.val.i);
        if (lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.i * rhs.val.f);
        if (!lhs.is_int && rhs.is_int) return ConstVal(lhs.val.f * rhs.val.i);
        if (!lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.f * rhs.val.f);
        break;
    case constop_t::DIV:
        if (lhs.is_int && rhs.is_int) return ConstVal(lhs.val.i / rhs.val.i);
        if (lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.i / rhs.val.f);
        if (!lhs.is_int && rhs.is_int) return ConstVal(lhs.val.f / rhs.val.i);
        if (!lhs.is_int && !rhs.is_int) return ConstVal(lhs.val.f / rhs.val.f);
        break;
    case constop_t::REM:
        if (lhs.is_int && rhs.is_int) return ConstVal(lhs.val.i % rhs.val.i);
        break;
    case constop_t::NOT:
        if (rhs.is_int) return ConstVal(int(!lhs.val.i));
        break;
    default:;
    }
    return ConstVal();
}

/* Nullable List Template */
template <typename T> struct NullableList {
    std::vector<T> list;
    int lineno;

    NullableList() {}
    NullableList(int _lineno)
        : lineno(_lineno) {}
    NullableList(T x, int _lineno)
        : lineno(_lineno) {
        list.push_back(x);
    }

    virtual ~NullableList() = default;
};

/* CompUnit -> [CompUnit] (Decl | FuncDef) */
/*
 * Root -> CompUnit*
 */
struct CompUnitList {
    std::vector<CompUnit*> list;
    int lineno;

    CompUnitList(int _lineno)
        : lineno(_lineno) {}
    IR::StmList* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv);
};

/* CompUnit -> FuncDef | Decl */
struct CompUnit {
    virtual IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
    virtual ~CompUnit() = default;
};

struct BlockItem {
    virtual ~BlockItem() = default;
    virtual IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct Decl : CompUnit, BlockItem {
    virtual ~Decl() = default;
    virtual IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct ConstDecl : Decl {
    btype_t btype;
    ConstDefList* defs;
    int lineno;
    ConstDecl(btype_t _btype, ConstDefList* _defs, int _lineno)
        : btype(_btype)
        , defs(_defs)
        , lineno(_lineno) {}
    IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct ConstDefList : public NullableList<ConstDef*> {
    ConstDefList()
        : NullableList() {}
    ConstDefList(int _lineno)
        : NullableList(_lineno) {}
    ConstDefList(ConstDef* x, int _lineno)
        : NullableList(x, _lineno) {}
};
struct ConstDef {
    IdExp* id;
    ArrayIndex* index_list;
    InitVal* val;
    int lineno;

    ConstDef(IdExp* _id, ArrayIndex* _index_list, InitVal* _val, int _lineno)
        : id(_id)
        , index_list(_index_list)
        , val(_val)
        , lineno(_lineno) {
        id->constval = ConstVal(nullptr);

        /* if (auto x = dynamic_cast<Exp *>(val)) {
                if (x->ast_isconst()) {
                        id->constval = x->constval;
                }
        } */
    }
    void binder(btype_t btype,Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv, Temp_Label name);
};
struct VarDecl : public Decl {
    btype_t btype;
    VarDefList* defs;
    int lineno;

    VarDecl(btype_t _btype, VarDefList* _defs, int _lineno)
        : btype(_btype)
        , defs(_defs)
        , lineno(_lineno) {}
    IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct VarDefList : public NullableList<VarDef*> {
    VarDefList()
        : NullableList() {}
    VarDefList(int _lineno)
        : NullableList(_lineno) {}
    VarDefList(VarDef* x, int _lineno)
        : NullableList(x, _lineno) {}
    IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct VarDef {
    IdExp* id;
    ArrayIndex* index;
    InitVal* initval;
    int lineno;

    VarDef(IdExp* _id, ArrayIndex* _index, InitVal* _initval, int _lineno)
        : id(_id)
        , index(_index)
        , initval(_initval)
        , lineno(_lineno) {}
    IR::Stm* ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label brelabel, Temp_Label conlabel,Temp_Label name);
};

struct ArrayIndex : NullableList<Exp*> {
    ArrayIndex()
        : NullableList() {}
    ArrayIndex(int _lineno)
        : NullableList(_lineno) {}
    ArrayIndex(Exp* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct InitVal {
    virtual ~InitVal() = default;
    virtual IR::ExpTy ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label name);
};

struct ArrayInit : public InitVal {
    virtual ~ArrayInit() = default;
};

struct InitValList : public ArrayInit, NullableList<InitVal*> {
    InitValList()
        : NullableList() {}
    InitValList(int _lineno)
        : NullableList(_lineno) {}
    InitValList(InitVal* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct FuncDef : public CompUnit {
    btype_t btype;
    IdExp* id;
    Parameters* parameters;
    Block* block;
    int lineno;

    FuncDef(btype_t _btype, IdExp* _id, Parameters* _parameters, Block* _block, int _lineno)
        : btype(_btype)
        , id(_id)
        , parameters(_parameters)
        , block(_block)
        , lineno(_lineno) {}
    IR::Stm* ast2ir();
};

struct Parameters : public NullableList<Parameter*> {
    Parameters()
        : NullableList() {}
    Parameters(int _lineno)
        : NullableList(_lineno) {}
    Parameters(Parameter* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct Parameter {
    btype_t btype;
    IdExp* id;
    ArrayIndex* arrayindex;
    int lineno;

    Parameter(btype_t _btype, IdExp* _id, ArrayIndex* _arrayindex, int _lineno)
        : btype(_btype)
        , id(_id)
        , arrayindex(_arrayindex)
        , lineno(_lineno) {}
};

struct Stmt : public BlockItem {
    /* Empty */
    virtual ~Stmt() = default;
};

struct Block : public Stmt {
    BlockItemList* items;
    int lineno;

    Block(BlockItemList* _items, int _lineno)
        : items(_items)
        , lineno(_lineno) {}
    IR::Stm* ast2ir();
};

struct BlockItemList : public NullableList<BlockItem*> {
    BlockItemList()
        : NullableList() {}
    BlockItemList(int _lineno)
        : NullableList(_lineno) {}
    BlockItemList(BlockItem* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct AssignStmt : public Stmt {
    Lval* lval;
    Exp* exp;
    int lineno;

    AssignStmt(Lval* _lval, Exp* _exp, int _lineno)
        : lval(_lval)
        , exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::AssignStmt::ast2ir();
};

struct ExpStmt : public Stmt {
    Exp* exp;
    int lineno;

    ExpStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::ExpStmt::ast2ir();
};

struct IfStmt : public Stmt {
    Exp* exp;
    Stmt* if_part;
    Stmt* else_part;
    int lineno;

    IfStmt(Exp* _exp, Stmt* _if_part, Stmt* _else_part, int _lineno)
        : exp(_exp)
        , if_part(_if_part)
        , else_part(_else_part)
        , lineno(_lineno) {}
    IR::Stm* AST::IfStmt::ast2ir();
};

struct WhileStmt : public Stmt {
    Exp* exp;
    Stmt* loop;
    int lineno;

    WhileStmt(Exp* _exp, Stmt* _loop, int _lineno)
        : exp(_exp)
        , loop(_loop)
        , lineno(_lineno) {}
    IR::Stm* AST::WhileStmt::ast2ir();
};

struct BreakStmt : public Stmt {
    int lineno;

    BreakStmt(int _lineno)
        : lineno(_lineno) {}
    IR::Stm* AST::BreakStmt::ast2ir();
};

struct ContinueStmt : public Stmt {
    int lineno;

    ContinueStmt(int _lineno)
        : lineno(_lineno) {}
    IR::Stm* AST::ContinueStmt::ast2ir();
};

struct ReturnStmt : public Stmt {
    Exp* exp;
    int lineno;

    ReturnStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::ReturnStmt::ast2ir();
};

struct Exp : InitVal {
    ConstVal constval;

    bool ast_isconst() { return constval.is_const; }
    virtual ~Exp() = default;
    virtual IR::ExpTy ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label name);
};

struct IdExp : public Exp {
    std::string* str;
    int lineno;

    IdExp(std::string* _str, int _lineno)
        : str(_str)
        , lineno(_lineno) {}
    IR::ExpTy AST::IdExp::ast2ir(Table::Stable<TY::Entry*>* venv, Table::Stable<TY::EnFunc*>* fenv,
                            Temp_Label name);
};

struct PrimaryExp : public Exp {
    virtual ~PrimaryExp() = default;
    // IR::ExpTy AST::PrimaryExp::ast2ir();
};

struct Lval : public PrimaryExp {
    IdExp* id;
    ArrayIndex* arrayindex;
    int lineno;

    Lval(IdExp* _id, ArrayIndex* _arrayindex, int _lineno)
        : id(_id)
        , arrayindex(_arrayindex)
        , lineno(_lineno) {
        /* Lval exists in AssignStmt and should not be const val */
        constval = ConstVal(nullptr);
    }
    IR::ExpTy AST::Lval::ast2ir();
};

struct Number : public PrimaryExp {
    virtual ~Number() = default;
};

struct IntNumber : Number {
    int value;
    int lineno;

    IntNumber(int _value, int _lineno)
        : value(_value)
        , lineno(_lineno) {
        constval = ConstVal(value);
    }
    IR::ExpTy AST::IntNumber::ast2ir();
};

struct FloatNumber : Number {
    std::string* float_in_string;
    int lineno;

    FloatNumber(std::string* _float_in_string, int _lineno)
        : float_in_string(_float_in_string)
        , lineno(_lineno) {
        constval = ConstVal(std::stof(*float_in_string));
    }
    IR::ExpTy AST::FloatNumber::ast2ir();
};

struct UnaryExp : public Exp {
    unaryop_t op;
    Exp* exp;
    int lineno;

    UnaryExp(unaryop_t _op, Exp* _exp, int _lineno)
        : op(_op)
        , exp(_exp)
        , lineno(_lineno) {
        if (exp->ast_isconst()) {
            switch (op) {
            case unaryop_t::ADD:
                constval = calconst(ConstVal(0), constop_t::ADD, exp->constval);
                break;
            case unaryop_t::SUB:
                constval = calconst(ConstVal(0), constop_t::SUB, exp->constval);
                break;
            case unaryop_t::NOT:
                constval = calconst(ConstVal(0), constop_t::NOT, exp->constval);
                break;
            default:;
            }
        }
    }
    IR::ExpTy AST::UnaryExp::ast2ir();
};

struct CallExp : public Exp {
    IdExp* id;
    ExpList* params;
    int lineno;

    CallExp(IdExp* _id, ExpList* _params, int _lineno)
        : id(_id)
        , params(_params)
        , lineno(_lineno) {}
    IR::ExpTy AST::CallExp::ast2ir();
};

struct ExpList : public NullableList<Exp*> {
    ExpList()
        : NullableList() {}
    ExpList(int _lineno)
        : NullableList(_lineno) {}
    ExpList(Exp* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct MulExp : public Exp {
    Exp* lhs;
    mul_t op;
    Exp* rhs;
    int lineno;

    MulExp(Exp* _lhs, mul_t _op, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , op(_op)
        , rhs(_rhs)
        , lineno(_lineno) {
        if (lhs->ast_isconst() && rhs->ast_isconst()) {
            switch (op) {
            case mul_t::MULT: constval = calconst(lhs, constop_t::MUL, rhs); break;
            case mul_t::DIV: constval = calconst(lhs, constop_t::DIV, rhs); break;
            case mul_t::REM: constval = calconst(lhs, constop_t::REM, rhs); break;

            default:;
            }
        }
    }
    IR::ExpTy AST::MulExp::ast2ir();
};

struct AddExp : public Exp {
    Exp* lhs;
    addop_t op;
    Exp* rhs;
    int lineno;

    AddExp(Exp* _lhs, addop_t _op, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , op(_op)
        , rhs(_rhs)
        , lineno(_lineno) {
        if (lhs->ast_isconst() && rhs->ast_isconst()) {
            switch (op) {
            case addop_t::ADD: constval = calconst(lhs, constop_t::ADD, rhs); break;
            case addop_t::SUB: constval = calconst(lhs, constop_t::SUB, rhs); break;
            default:;
            }
        }
    }
    IR::ExpTy AST::AddExp::ast2ir();
};

struct RelExp : public Exp {
    Exp* lhs;
    rel_t op;
    Exp* rhs;
    int lineno;

    RelExp(Exp* _lhs, rel_t _op, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , op(_op)
        , rhs(_rhs)
        , lineno(_lineno) {}
    IR::ExpTy AST::RelExp::ast2ir();
};

struct EqExp : public Exp {
    Exp* lhs;
    equal_t op;
    Exp* rhs;
    int lineno;

    EqExp(Exp* _lhs, equal_t _op, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , op(_op)
        , rhs(_rhs)
        , lineno(_lineno) {}
    IR::ExpTy AST::EqExp::ast2ir();
};

struct LAndExp : public Exp {
    Exp* lhs;
    Exp* rhs;
    int lineno;

    LAndExp(Exp* _lhs, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , rhs(_rhs)
        , lineno(_lineno) {}
    IR::ExpTy AST::LAndExp::ast2ir();
};

struct LOrExp : public Exp {
    Exp* lhs;
    Exp* rhs;
    int lineno;

    LOrExp(Exp* _lhs, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , rhs(_rhs)
        , lineno(_lineno) {}
    IR::ExpTy AST::LOrExp::ast2ir();
};

struct PutintStmt : public Stmt {
    Exp* exp;
    int lineno;

    PutintStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::PutintStmt::ast2ir();
};

struct PutchStmt : public Stmt {
    Exp* exp;
    int lineno;

    PutchStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::PutchStmt::ast2ir();
};

struct PutarrayStmt : public Stmt {
    Exp* len;
    Exp* arr;
    int lineno;

    PutarrayStmt(Exp* _len, Exp* _arr, int _lineno)
        : len(_len)
        , arr(_arr)
        , lineno(_lineno) {}
    IR::Stm* AST::PutarrayStmt::ast2ir();
};

struct PutfloatStmt : public Stmt {
    Exp* exp;
    int lineno;

    PutfloatStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
    IR::Stm* AST::PutfloatStmt::ast2ir();
};

struct PutfarrayStmt : public Stmt {
    Exp* len;
    Exp* arr;
    int lineno;

    PutfarrayStmt(Exp* _len, Exp* _arr, int _lineno)
        : len(_len)
        , arr(_arr)
        , lineno(_lineno) {}
    IR::Stm* AST::PutfarrayStmt::ast2ir();
};

struct PutfStmt : public Stmt {
    ExpList* args;
    int lineno;

    PutfStmt(ExpList* _args, int _lineno)
        : args(_args)
        , lineno(_lineno) {}
    IR::Stm* AST::PutfStmt::ast2ir();
};

struct StarttimeStmt : public Stmt {
    int lineno;

    StarttimeStmt(int _lineno)
        : lineno(_lineno) {}
    IR::Stm* AST::StarttimeStmt::ast2ir();
};

struct StoptimeStmt : public Stmt {
    int lineno;

    StoptimeStmt(int _lineno)
        : lineno(_lineno) {}
    IR::Stm* AST::StoptimeStmt::ast2ir();
};

struct GetintExp : public Exp {
    int lineno;

    GetintExp(int _lineno)
        : lineno(_lineno) {}
    IR::ExpTy AST::GetintExp::ast2ir();
};

struct GetchExp : public Exp {
    int lineno;

    GetchExp(int _lineno)
        : lineno(_lineno) {}
    IR::ExpTy AST::GetchExp::ast2ir();
};

struct GetfloatExp : public Exp {
    int lineno;

    GetfloatExp(int _lineno)
        : lineno(_lineno) {}
    IR::ExpTy AST::GetfloatExp::ast2ir();
};

struct GetarrayExp : public Exp {
    Exp* arr;
    int lineno;

    GetarrayExp(Exp* _arr, int _lineno)
        : arr(_arr)
        , lineno(_lineno) {}
    IR::ExpTy AST::GetarrayExp::ast2ir();
};

struct GetfarrayExp : public Exp {
    Exp* arr;
    int lineno;

    GetfarrayExp(Exp* _arr, int _lineno)
        : arr(_arr)
        , lineno(_lineno) {}
    IR::ExpTy AST::GetfarrayExp::ast2ir();
};

};  // namespace AST

#endif