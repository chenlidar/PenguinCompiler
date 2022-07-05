#ifndef __TREE_IR
#define __TREE_IR
#include <memory>
#include <string>
#include <vector>
using std::string;
using std::unique_ptr;
using std::vector;

class T_stm_ {};
class T_exp_ {};

// FIXME use smart
typedef T_stm_* T_stm;
typedef T_exp_* T_exp;

// use T_Binop::T_plus
enum class T_Binop {
    T_plus,
    T_minus,
    T_mul,
    T_div,
    T_and,
    T_or,
    T_lshift,
    T_rshift,
    T_arshift,
    T_xor
};
enum class T_relOp {
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
T_relOp T_commute(T_relOp op);  // a op b    ==    b commute(op) a
T_relOp T_notRel(T_relOp op);   // a op b    ==     not(a notRel(op) b)
class T_seq_ : public T_stm_ {
   public:
    T_stm left, right;
    T_seq_(T_stm lf, T_stm rg) { left = lf, right = rg; }
};
class T_label_ : public T_stm_ {
   public:
    string label;
    T_label_(string lb) { label = lb; }
};
class T_jump_ : public T_stm_ {
   public:
    T_exp exp;
    vector<string> jumps;
    T_jump_(T_exp ep, vector<string> s) { exp = ep, jumps = s; }
};
class T_cjump_ : public T_stm_ {
   public:
    T_relOp op;
    T_exp left, right;
    string trueLabel, falseLabel;
    T_cjump_(T_relOp p, T_exp lf, T_exp rg, string tr, string fs) {
        op = p, left = lf, right = rg, trueLabel = tr, falseLabel = fs;
    }
};
class T_move_ : public T_stm_ {
   public:
    T_exp src, dst;
    T_move_(T_exp ds, T_exp sr) { src = sr, dst = ds; }
};
class T_expStm_ : public T_stm_ {
   public:
    T_exp exp;
    T_expStm_(T_exp e) { exp = e; }
};

class T_constInt_ : public T_exp_ {
   public:
    int val;
    T_constInt_(int x) { val = x; }
};
class T_constFloat_ : public T_exp_ {
   public:
    float val;
    T_constFloat_(float f) { val = f; }
};

class T_binop_ : public T_exp_ {
   public:
    T_exp left, right;
    T_Binop op;
    T_binop_(T_Binop o, T_exp lf, T_exp rg) { op = o, left = lf, right = rg; }
};
class T_temp_ : public T_exp_ {
   public:
    int tempid;
    T_temp_(int id) { tempid = id; }
};
class T_mem_ : public T_exp_ {
   public:
    T_exp mem;
    T_mem_(T_exp e) { mem = e; }
};
class T_eseq_ : public T_exp_ {
   public:
    T_stm stm;
    T_exp exp;
    T_eseq_(T_stm s, T_exp e) { stm = s, exp = e; }
};
// TBD TODO:
class T_name_ : public T_exp_ {
   public:
    string name;
    T_name_(string s) { name = s; }
};
class T_call_ : public T_exp_ {
   public:
    T_exp fun;
    vector<T_exp> args;
    T_call_(T_exp fu, vector<T_exp> ar) { args = ar, fun = fu; }
};

#endif