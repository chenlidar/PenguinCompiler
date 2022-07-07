#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
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
static TY::Type binopResType(const TY::Type& a, const TY::Type& b) {
    if (a.kind != TY::tyType::Ty_float && a.kind != TY::tyType::Ty_int) {
        // err
    }
    if (b.kind != TY::tyType::Ty_float && b.kind != TY::tyType::Ty_int) {
        // err
    }
    return (b.kind == TY::tyType::Ty_float || a.kind != TY::tyType::Ty_float)
               ? TY::floatType()
               : TY::intType();
}
#endif