#ifndef __TREE_IR
#define __TREE_IR
#include <memory>
#include <string>
#include <vector>
namespace IR {
using std::string;
using std::unique_ptr;
using std::vector;

// use Binop::plus
enum class binop {
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
RelOp commute(RelOp op);  // a op b    ==    b commute(op) a
RelOp notRel(RelOp op);   // a op b    ==     not(a notRel(op) b)
class Stm {};
class Exp {};
class Seq : public Stm {
   public:
    Stm *left, *right;
    Seq(Stm* lf, Stm* rg) { left = lf, right = rg; }
};
class Label : public Stm {
   public:
    string label;
    Label(string lb) { label = lb; }
};
class Jump : public Stm {
   public:
    Exp* exp;
    vector<string> jumps;
    Jump(Exp* ep, vector<string> s) { exp = ep, jumps = s; }
};
class Cjump : public Stm {
   public:
    RelOp op;
    Exp *left, *right;
    string trueLabel, falseLabel;
    Cjump(RelOp p, Exp* lf, Exp* rg, string tr, string fs) {
        op = p, left = lf, right = rg, trueLabel = tr, falseLabel = fs;
    }
};
class Move : public Stm {
   public:
    Exp *src, *dst;
    Move(Exp* ds, Exp* sr) { src = sr, dst = ds; }
};
class ExpStm : public Stm {
   public:
    Exp* exp;
    ExpStm(Exp* e) { exp = e; }
};

class ConstInt : public Exp {
   public:
    int val;
    ConstInt(int x) { val = x; }
};
class ConstFloat : public Exp {
   public:
    float val;
    ConstFloat(float f) { val = f; }
};

class Binop : public Exp {
   public:
    Exp *left, *right;
    binop op;
    Binop(binop o, Exp* lf, Exp* rg) { op = o, left = lf, right = rg; }
};
class Temp : public Exp {
   public:
    int tempid;
    Temp(int id) { tempid = id; }
};
class Mem : public Exp {
   public:
    Exp* mem;
    Mem(Exp* e) { mem = e; }
};
class Eseq : public Exp {
   public:
    Stm* stm;
    Exp* exp;
    Eseq(Stm* s, Exp* e) { stm = s, exp = e; }
};
// TBD TODO:
class Name : public Exp {
   public:
    string name;
    Name(string s) { name = s; }
};
class Call : public Exp {
   public:
    Exp* fun;
    vector<Exp*> args;
    Call(Exp* fu, vector<Exp*> ar) { args = ar, fun = fu; }
};

}  // namespace IR
#endif