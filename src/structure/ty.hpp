#ifndef __TY
#define __TY
namespace TY {
enum class tyType { Ty_void, Ty_array, Ty_int, Ty_float };
class Type {
   public:
    Type* tp;
    tyType kind;
    // int arity;
    Type(Type* t, tyType k) { tp = t, kind = k; }
};

Type intType() {
    return TY::Type(0, TY::tyType::Ty_int);
}
Type floatType() {
    return TY::Type(0, TY::tyType::Ty_float);
}
};  // namespace TY

#endif