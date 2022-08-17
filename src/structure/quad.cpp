#include "treeIR.hpp"
#include <assert.h>
#include <algorithm>
#include <string>
#include <iostream>
#include <cmath>
#include "../backend/canon.hpp"
#include "../util/utils.hpp"
#include "../util/muldiv.hpp"
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
#include "assem.h"
using std::endl;
using std::string;
using namespace IR;

// quad
Stm* Label::quad() { return new Label(label); }
Stm* Jump::quad() { return new Jump(target); }
Stm* Cjump::quad() { return new Cjump(op, left->quad(), right->quad(), trueLabel, falseLabel); }
Stm* Move::quad() {
    if (src->kind == expType::mem && dst->kind == expType::mem) {
        Temp_Temp ntp = Temp_newtemp();
        return new Seq(new Move(new Temp(ntp), new Mem(static_cast<Mem*>(src)->mem->quad())),
                       new Move(new Mem(static_cast<Mem*>(dst)->mem->quad()), new Temp(ntp)));
    }
    if (dst->kind == expType::mem) {
        return new Move(new Mem(static_cast<Mem*>(dst)->mem->quad()), src->quad());
    }
    if (src->kind == expType::mem) {
        return new Move(dst->quad(), new Mem(static_cast<Mem*>(src)->mem->quad()));
    }
    return new Move(dst->quad(), src->quad());
}
Stm* ExpStm::quad() { return new ExpStm(exp->quad()); }
Exp* Const::quad() { return new Const(val); }
Exp* Binop::quad() {
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Binop(op, left->quad(), right->quad())),
                    new Temp(ntp));
}
Exp* Temp::quad() { return new Temp(tempid); }
Exp* Mem::quad() {
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Mem(mem->quad())), new Temp(ntp));
}
Exp* Name::quad() { return new Name(name); }
Exp* Call::quad() {
    vector<Exp*> tm;
    for (auto it : args) tm.push_back(it->quad());
    Temp_Temp ntp = Temp_newtemp();
    return new Eseq(new Move(new Temp(ntp), new Call(fun, tm)), new Temp(ntp));
}