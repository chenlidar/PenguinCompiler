#ifndef __TY
#define __TY
#include <vector>
namespace TY {
enum class tyType { Ty_void, Ty_array, Ty_int, Ty_float,Ty_func };
class Type {
   public:
    Type* tp;
    std::vector<Type*> param;
    tyType kind;
    // int arity;
    Type(Type* t, tyType k) { tp = t, kind = k; }
    Type(Type* t, tyType k,std::vector<Type*> _param) { tp = t, kind = k;param=_param; }
};

Type intType() {
    return TY::Type(0, TY::tyType::Ty_int);
}
Type floatType() {
    return TY::Type(0, TY::tyType::Ty_float);
}
Type arrayType(Type* ty) {
    return TY::Type(ty,TY::tyType::Ty_array);
}
Type funcType(Type* rtn,std::vector<Type*> _param){
    return TY::Type(0,TY::tyType::Ty_func,_param);
}
struct Entry{
    
};
};  // namespace TY
#endif