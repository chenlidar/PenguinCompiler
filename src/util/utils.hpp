#ifndef __UTILS
#define __UTILS
#include "../structure/treeIR.hpp"
#include "../structure/ty.hpp"
inline IR::Stm* nopStm() {
    return (new IR::ExpStm(new IR::ConstInt(0)));
}
bool isNop(IR::Stm* x) {
    return typeid(*x) == typeid(IR::ExpStm) &&
           (typeid(*static_cast<IR::ExpStm*>(x)->exp) == typeid(IR::ConstInt) ||
            typeid(*static_cast<IR::ExpStm*>(x)->exp) ==
                typeid(IR::ConstFloat));
}

IR::Stm* seq(IR::Stm* x, IR::Stm* y) {
    if (isNop(x))
        return y;
    if (isNop(y))
        return x;
    return new IR::Seq(x, y);
}
void doPatch(IR::PatchList* tList, Temp_Label label) {
    for (; tList; tList = tList->tail)
        tList->head = label;
}
IR::PatchList* joinPatch(IR::PatchList* first,IR::PatchList* second) {
    if(!first)return second;
	IR::PatchList* tmp=first;
    for (;tmp->tail;tmp=tmp->tail);
    tmp->tail=second;
    return first;
}
TY::Type* binopResType(const TY::Type* a, const TY::Type* b,IR::binop op) {
    assert(a->kind==TY::tyType::Ty_float||a->kind==TY::tyType::Ty_int);
    assert(b->kind==TY::tyType::Ty_float||b->kind==TY::tyType::Ty_int);
    switch(op){
        case IR::binop::T_plus:{
            if(a->kind==TY::tyType::Ty_int&&b->kind==TY::tyType::Ty_int){
                if(a->value&&b->value)return TY::intType(new int(*a->value+*b->value));
            }
            else assert(0);//FIXME
        }break;
        case IR::binop::T_minus:{
            if(a->kind==TY::tyType::Ty_int&&b->kind==TY::tyType::Ty_int){
                if(a->value&&b->value)return TY::intType(new int(*a->value-*b->value));
            }
            else assert(0);//FIXME
        }break;
        case IR::binop::T_mul:{
            if(a->kind==TY::tyType::Ty_int&&b->kind==TY::tyType::Ty_int){
                if(a->value&&b->value)return TY::intType(new int(*a->value* *b->value));
            }
            else assert(0);//FIXME
        }break;
        case IR::binop::T_div:{
            if(a->kind==TY::tyType::Ty_int&&b->kind==TY::tyType::Ty_int){
                if(a->value&&b->value)return TY::intType(new int(*a->value/ *b->value));
            }
            else assert(0);//FIXME
        }break;
        case IR::binop::T_mod:{
            assert(a->kind==TY::tyType::Ty_int&&b->kind==TY::tyType::Ty_int);
            if(a->value&&b->value)return TY::intType(new int(*a->value% *b->value));
        }break;
        default:assert(0);
    }
}
#endif