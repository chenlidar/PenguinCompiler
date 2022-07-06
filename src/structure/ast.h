#ifndef __AST_H
#define __AST_H

#include <string>
#include <vector>

namespace AST {

enum class rel_t { LE, LT, GE, GT };

enum class equal_t { EQ, NE };

enum class mul_t { MULT, DIV, REM };

enum class unaryop_t { ADD, SUB, NOT };

enum class btype_t { VOID, INT, FLOAT };

enum class addop_t { ADD, SUB };

struct CompUnitList;
struct CompUnit;
struct Decl;
struct ConstDecl;
struct ConstDefList;
struct ConstDef;
struct ConstInitVal;
struct ConstExpList;
struct ConstInitValList;
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
};

/* CompUnit -> FuncDef | Decl */
struct CompUnit {};

struct BlockItem {
    /* Empty */
};

struct Decl : CompUnit, BlockItem {};

struct ConstDecl : Decl {
    btype_t btype;
    ConstDefList* defs;
    int lineno;
    ConstDecl(btype_t _btype, ConstDefList* _defs, int _lineno)
        : btype(_btype)
        , defs(_defs)
        , lineno(_lineno) {}
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
    std::string* id;
    ConstExpList* index_list;
    ConstInitVal* val;
    int lineno;

    ConstDef(std::string* _id, ConstExpList* _index_list, ConstInitVal* _val, int _lineno)
        : id(_id)
        , index_list(_index_list)
        , val(_val)
        , lineno(_lineno) {}
};

struct ConstInitVal {};

struct ConstExpList : public NullableList<Exp*> {
    ConstExpList()
        : NullableList() {}
    ConstExpList(int _lineno)
        : NullableList(_lineno) {}
    ConstExpList(Exp* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct ConstInitValList : public NullableList<ConstInitVal*>, public ConstInitVal {
    ConstInitValList()
        : NullableList() {}
    ConstInitValList(int _lineno)
        : NullableList(_lineno) {}
    ConstInitValList(ConstInitVal* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct VarDecl : public Decl {
    btype_t btype;
    VarDefList* defs;
    int lineno;

    VarDecl(btype_t _btype, VarDefList* _defs, int _lineno)
        : btype(_btype)
        , defs(_defs)
        , lineno(_lineno) {}
};

struct VarDefList : public NullableList<VarDef*> {
    VarDefList()
        : NullableList() {}
    VarDefList(int _lineno)
        : NullableList(_lineno) {}
    VarDefList(VarDef* x, int _lineno)
        : NullableList(x, _lineno) {}
};

struct VarDef {
    std::string* id;
    ArrayIndex* index;
    InitVal* initval;
    int lineno;

    VarDef(std::string* _id, ArrayIndex* _index, InitVal* _initval, int _lineno)
        : id(_id)
        , index(_index)
        , initval(_initval)
        , lineno(_lineno) {}
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
    /* Empty */
};

struct ArrayInit : public InitVal {
    /* Empty */
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
    std::string* id;
    Parameters* parameters;
    Block* block;
    int lineno;

    FuncDef(btype_t _btype, std::string* _id, Parameters* _parameters, Block* _block, int _lineno)
        : btype(_btype)
        , id(_id)
        , parameters(_parameters)
        , block(_block)
        , lineno(_lineno) {}
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
    std::string* id;
    ArrayIndex* arrayindex;
    int lineno;

    Parameter(btype_t _btype, std::string* _id, ArrayIndex* _arrayindex, int _lineno)
        : btype(_btype)
        , id(_id)
        , arrayindex(_arrayindex)
        , lineno(_lineno) {}
};

struct Stmt : public BlockItem {
    /* Empty */
};

struct Block : public Stmt {
    BlockItemList* items;
    int lineno;

    Block(BlockItemList* _items, int _lineno)
        : items(_items)
        , lineno(_lineno) {}
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
};

struct ExpStmt : public Stmt {
    Exp* exp;
    int lineno;

    ExpStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
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
};

struct WhileStmt : public Stmt {
    Exp* exp;
    Stmt* loop;
    int lineno;

    WhileStmt(Exp* _exp, Stmt* _loop, int _lineno)
        : exp(_exp)
        , loop(_loop)
        , lineno(_lineno) {}
};

struct BreakStmt : public Stmt {
    int lineno;

    BreakStmt(int _lineno)
        : lineno(_lineno) {}
};

struct ContinueStmt : public Stmt {
    int lineno;

    ContinueStmt(int _lineno)
        : lineno(_lineno) {}
};

struct ReturnStmt : public Stmt {
    Exp* exp;
    int lineno;

    ReturnStmt(Exp* _exp, int _lineno)
        : exp(_exp)
        , lineno(_lineno) {}
};

struct Exp : ConstInitVal, InitVal {
    /* Empty */
};

struct PrimaryExp : public Exp {
    /* Empty */
};

struct Lval : public PrimaryExp {
    std::string* id;
    ArrayIndex* arrayindex;
    int lineno;

    Lval(std::string* _id, ArrayIndex* _arrayindex, int _lineno)
        : id(_id)
        , arrayindex(_arrayindex)
        , lineno(_lineno) {}
};

struct Number : public PrimaryExp {
    /* Empty */
};

struct IntNumber : Number {
    int value;
    int lineno;

    IntNumber(int _value, int _lineno)
        : value(_value)
        , lineno(_lineno) {}
};

struct FloatNumber : Number {
    std::string* float_in_string;
    int lineno;

    FloatNumber(std::string* _float_in_string, int _lineno)
        : float_in_string(_float_in_string)
        , lineno(_lineno) {}
};

struct UnaryExp : public Exp {
    unaryop_t op;
    Exp* exp;
    int lineno;

    UnaryExp(unaryop_t _op, Exp* _exp, int _lineno)
        : op(_op)
        , exp(_exp)
        , lineno(_lineno) {}
};

struct CallExp : public Exp {
    std::string* id;
    ExpList* params;
    int lineno;

    CallExp(std::string* _id, ExpList* _params, int _lineno)
        : id(_id)
        , params(_params)
        , lineno(_lineno) {}
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
        , lineno(_lineno) {}
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
        , lineno(_lineno) {}
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
};

struct LAndExp : public Exp {
    Exp* lhs;
    Exp* rhs;
    int lineno;

    LAndExp(Exp* _lhs, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , rhs(_rhs)
        , lineno(_lineno) {}
};

struct LOrExp : public Exp {
    Exp* lhs;
    Exp* rhs;
    int lineno;

    LOrExp(Exp* _lhs, Exp* _rhs, int _lineno)
        : lhs(_lhs)
        , rhs(_rhs)
        , lineno(_lineno) {}
};

};  // namespace AST

#endif