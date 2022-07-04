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
typedef unique_ptr<T_stm_> T_stm;
typedef unique_ptr<T_exp_> T_exp;

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
T_relOp T_notRel(T_relOp);   // a op b    ==     not(a notRel(op) b)
T_relOp T_commute(T_relOp);  // a op b    ==    b commute(op) a

class T_seq_ : public T_stm_ {
    T_stm left, right;
};
class T_label_ : public T_stm_ {
    string label;
};
// TBD TODO:
class T_jump_ : public T_stm_ {
    T_exp exp;
    vector<string> jumps;
};
class T_cjump_ : public T_stm_ {
    T_relOp op;
    T_exp left, right;
    string trueLabel, falseLabel;
};
class T_move_ : public T_stm_ {
    T_exp src, dst;
};
class T_expStm_ : public T_stm_ {
    T_exp exp;
};

class T_constInt_ : public T_exp_ {
    int val;
};
class T_constFloat_ : public T_exp_ {
    float val;
};

class T_binop_ : public T_exp_ {
    T_exp left, right;
    T_Binop op;
};
class T_temp_ : public T_exp_ {
    int tempid;
};
class T_mem_ : public T_exp_ {
    T_exp mem;
};
class T_eseq_ : public T_exp_ {
    T_stm stm;
    T_exp exp;
};
// TBD TODO:
class T_name_ : public T_exp_ {
    string name;
};
class T_call_ : public T_exp_ {
    T_exp fun;
    vector<T_exp> args;
};

#endif