#ifndef __TREE_IR
#define __TREE_IR
#include <list>
#include <memory>
#include <string>
#include <vector>
#include "../util/templabel.hpp"
#include "assem.h"
#include "ty.hpp"
#include <unordered_map>
#include <unordered_set>
#include "../util/table.hpp"
namespace IR {
using std::string;
using std::unique_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

// use Binop::plus
enum class binop { T_plus, T_minus, T_mul, T_div, T_mod };
enum class RelOp { T_eq, T_ne, T_lt, T_gt, T_le, T_ge };

enum class expType { constx, binop, temp, mem, eseq, name, call };
enum class stmType { seq, label, jump, cjump, move, exp };

RelOp commute(RelOp op);  // a op b    ==    b commute(op) a
RelOp notRel(RelOp op);  // a op b    ==     not(a notRel(op) b)
class Stm {
public:
    stmType kind;
    virtual ~Stm() = default;
    virtual void ir2asm(ASM::InstrList* ls) = 0;
    // new sth
    virtual Stm* quad() { assert(0); }
    virtual void printIR() { assert(0); }
    virtual void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                          unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                          vector<Stm*>& ls) {
        assert(0);
    }
    virtual Stm* dCopy() {
        assert(0);
        return nullptr;
    }
};
class Exp {
public:
    expType kind;
    virtual ~Exp() = default;
    virtual Temp_Temp ir2asm(ASM::InstrList* ls) = 0;
    // new sth
    virtual Exp* quad() { assert(0); }
    virtual void printIR() { assert(0); }
    virtual Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                          unordered_map<string, string>& lbmp, unordered_set<Temp_Label>& venv,
                          vector<Stm*>& ls) {
        assert(0);
    }
    virtual Exp* dCopy() {
        assert(0);
        return nullptr;
    }
};
class Seq : public Stm {
public:
    Stm *left, *right;
    Seq(Stm* lf, Stm* rg) {
        left = lf, right = rg;
        kind = stmType::seq;
    }
    void ir2asm(ASM::InstrList* ls) { assert(0); }
    Stm* quad() { assert(0); }
    void printIR() { assert(0); }
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy() {
        assert(0);
        return nullptr;
    }
};
class Label : public Stm {
public:
    string label;
    Label(string lb) {
        label = lb;
        kind = stmType::label;
    }
    void ir2asm(ASM::InstrList* ls);
    Stm* quad();
    void printIR();
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy() {
        assert(0);
        return nullptr;
    }
};
class Jump : public Stm {
public:
    string target;
    Jump(string _target) {
        target = _target;
        kind = stmType::jump;
    }
    void ir2asm(ASM::InstrList* ls);
    Stm* quad();
    void printIR();
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy() {
        assert(0);
        return nullptr;
    }
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
    void ir2asm(ASM::InstrList* ls);
    Stm* quad();
    void printIR();
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy() {
        assert(0);
        return nullptr;
    }
};
class Move : public Stm {
public:
    Exp *src, *dst;
    Move(Exp* ds, Exp* sr) {
        src = sr, dst = ds;
        kind = stmType::move;
    }
    void ir2asm(ASM::InstrList* ls);
    Stm* quad();
    void printIR();
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy();
};
class ExpStm : public Stm {
public:
    Exp* exp;
    ExpStm(Exp* e) {
        exp = e;
        kind = stmType::exp;
    }
    void ir2asm(ASM::InstrList* ls);
    Stm* quad();
    void printIR();
    void deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Stm* dCopy();
};

class Const : public Exp {
public:
    int val;
    Const(int x) {
        val = x;
        kind = expType::constx;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
};

class Binop : public Exp {
public:
    Exp *left, *right;
    binop op;
    Binop(binop o, Exp* lf, Exp* rg) { op = o, left = lf, kind = expType::binop, right = rg; }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
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
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
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
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
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
    void printIR() { assert(0); }
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy() {
        assert(0);
        return nullptr;
    }
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
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
};
class Call : public Exp {
public:
    string fun;
    vector<Exp*> args;
    Call(string fu, vector<Exp*> ar) {
        args = ar, fun = fu;
        kind = expType::call;
    }
    Temp_Temp ir2asm(ASM::InstrList* ls);
    Exp* quad();
    void printIR();
    Exp* deepCopy(unordered_map<Temp_Temp, Temp_Temp>& tpmp, unordered_map<string, string>& lbmp,
                  unordered_set<Temp_Label>& venv, vector<Stm*>& ls);
    Exp* dCopy();
};
struct PatchList {
    Temp_Label* head;
    PatchList* tail;
    PatchList(Temp_Label* _head, PatchList* _tail)
        : head(_head)
        , tail(_tail) {}
};
struct Cx {
    PatchList* trues;
    PatchList* falses;
    Stm* stm;
    Cx(PatchList* _trues, PatchList* _falses, Stm* _stm)
        : trues(_trues)
        , falses(_falses)
        , stm(_stm) {}
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
    ExpTy(Tr_Exp* _exp, TY::Type* _ty)
        : exp(_exp)
        , ty(_ty) {}
    ExpTy() {}
};
class StmList {
public:
    Stm* stm;
    StmList* tail;
    StmList(Stm* _stm, StmList* _tail)
        : stm(_stm)
        , tail(_tail) {}
    StmList* deepCopy(unordered_set<Temp_Label>& venv, int offset,
                      unordered_map<Temp_Temp, Temp_Temp>& tpmp,
                      unordered_map<string, string>& lbmp);
};
typedef std::vector<Exp*> ExpList;
ASM::Proc* ir2asm(StmList* stmlist);
}  // namespace IR
#endif