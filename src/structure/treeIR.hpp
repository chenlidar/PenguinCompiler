#ifndef __TREE_IR
#define __TREE_IR
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "../util/templabel.hpp"
#include "assem.h"
#include "ty.hpp"
namespace IR {
using std::string;
using std::unique_ptr;
using std::vector;

// use Binop::plus
enum class binop { T_plus, T_minus, T_mul, T_div, T_mod, T_and, T_or };
enum class RelOp {
    T_eq,
    T_ne,
    T_lt,
    T_gt,
    T_le,
    T_ge,
    T_ult,
    T_ule,
    T_ugt,
    T_uge
};

enum class expType {
    constint,
    constfloat,
    binop,
    fbinop,
    temp,
    mem,
    eseq,
    name,
    call
};
enum class stmType { seq, label, jump, cjump, move, exp };

RelOp commute(RelOp op);  // a op b    ==    b commute(op) a
RelOp notRel(RelOp op);   // a op b    ==     not(a notRel(op) b)
class Stm {
   public:
    stmType kind;
    virtual ~Stm() = default;
    virtual void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) = 0;
    // new sth
    virtual Stm* quad() { assert(0); };
};
class Exp {
   public:
    expType kind;
    virtual ~Exp() = default;
    virtual Temp_Temp ir2asm(ASM::InstrList* ls) = 0;
    // new sth
    virtual Exp* quad() { assert(0); };
};
class Seq : public Stm {
   public:
    Stm *left, *right;
    Seq(Stm* lf, Stm* rg) {
        left = lf, right = rg;
        kind = stmType::seq;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel) { assert(0); }
    Stm* quad() { assert(0); }
};
class Label : public Stm {
   public:
    string label;
    Label(string lb) {
        label = lb;
        kind = stmType::label;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel);
    Stm* quad();
};
class Jump : public Stm {
   public:
    Exp* exp;
    vector<string> jumps;
    Jump(Exp* ep, vector<string> s) {
        exp = ep, jumps = s;
        kind = stmType::jump;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel);
    Stm* quad();
};
class Cjump : public Stm {
   public:
    RelOp op;
    Exp *left, *right;
    Temp_Label trueLabel, falseLabel;
    Cjump(RelOp p, Exp* lf, Exp* rg, string tr, string fs) {
        op = p, left = lf, right = rg, trueLabel = tr, falseLabel = fs;
        kind = stmType::cjump;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel);
    Stm* quad();
};
class Move : public Stm {
   public:
    Exp *src, *dst;
    Move(Exp* ds, Exp* sr) {
        src = sr, dst = ds;
        kind = stmType::move;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel);
    Stm* quad();
};
class ExpStm : public Stm {
   public:
    Exp* exp;
    ExpStm(Exp* e) {
        exp = e;
        kind = stmType::exp;
    }
    void ir2asm(ASM::InstrList* ls, Temp_Label exitlabel);
    Stm* quad();
};

class ConstInt : public Exp {
   public:
    int val;
    ConstInt(int x) {
        val = x;
        kind = expType::constint;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
class ConstFloat : public Exp {
   public:
    float val;
    ConstFloat(float f) {
        val = f;
        kind = expType::constfloat;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};

class Binop : public Exp {
   public:
    Exp *left, *right;
    binop op;
    Binop(binop o, Exp* lf, Exp* rg) {
        op = o, left = lf, kind = expType::binop, right = rg;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
class Temp : public Exp {
   public:
    Temp_Temp tempid;
    Temp(int id) {
        tempid = id;
        kind = expType::temp;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
class Mem : public Exp {
   public:
    Exp* mem;
    Mem(Exp* e) {
        mem = e;
        kind = expType::mem;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
class Eseq : public Exp {
   public:
    Stm* stm;
    Exp* exp;
    Eseq(Stm* s, Exp* e) {
        stm = s, exp = e;
        kind = expType::eseq;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad() { assert(0); }
};
// TBD TODO:
class Name : public Exp {
   public:
    string name;
    Name(string s) {
        name = s;
        kind = expType::name;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
class Call : public Exp {
   public:
    Exp* fun;
    vector<Exp*> args;
    Call(Exp* fu, vector<Exp*> ar) {
        args = ar, fun = fu;
        kind = expType::call;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
};
struct PatchList {
    Temp_Label* head;
    PatchList* tail;
    PatchList(Temp_Label* _head, PatchList* _tail) : head(_head), tail(_tail) {}
};
struct Cx {
    PatchList* trues;
    PatchList* falses;
    Stm* stm;
    Cx(PatchList* _trues, PatchList* _falses, Stm* _stm)
        : trues(_trues), falses(_falses), stm(_stm) {}
    Cx() {}
};
enum class Tr_ty { Tr_ex, Tr_nx, Tr_cx };
struct Tr_Exp {
    Tr_ty kind;
    Exp* ex;
    Stm* nx;
    Cx cx;
    Tr_Exp(Stm* stm);
    Tr_Exp(PatchList* trues, PatchList* falses, Stm* stm);
    Tr_Exp(Exp* exp);
    Exp* unEx();
    Cx unCx();
    Stm* unNx();
};
class ExpTy {
   public:
    Tr_Exp* exp;
    TY::Type* ty;
    ExpTy(Tr_Exp* _exp, TY::Type* _ty) : exp(_exp), ty(_ty) {}
    ExpTy() {}
};
class StmList {
   public:
    Stm* stm;
    StmList* tail;
    StmList(Stm* _stm, StmList* _tail) : stm(_stm), tail(_tail) {}
};
typedef std::vector<Exp*> ExpList;
ASM::Proc* ir2asm(StmList* stmlist);
}  // namespace IR
#endif