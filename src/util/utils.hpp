#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
inline IR::Stm* nopStm() {
    return (new IR::ExpStm(new IR::ConstInt(0)));
}
static bool isNop(IR::Stm* x) {
    return typeid(*x) == typeid(IR::ExpStm) &&
           (typeid(*static_cast<IR::ExpStm*>(x)->exp) == typeid(IR::ConstInt) ||
            typeid(*static_cast<IR::ExpStm*>(x)->exp) ==
                typeid(IR::ConstFloat));
}

static IR::Stm* seq(IR::Stm* x, IR::Stm* y) {
    if (isNop(x))
        return y;
    if (isNop(y))
        return x;
    return new IR::Seq(x, y);
}
#endif