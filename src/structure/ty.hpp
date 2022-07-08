#ifndef __TY
#define __TY
#include <vector>
#include <assert.h>
namespace TY {
enum class tyType { Ty_void, Ty_array, Ty_int, Ty_float, Ty_func };
class Type {
public:
    Type* tp;
    std::vector<Type*> param;
    tyType kind;
    int dim;
    int arraysize;
    bool isconst;
    int* value;
    //FIXME
    // int arity;
    Type(){}
    Type(Type* t, tyType k, int* _value,bool _isconst) { arraysize=1;dim=1;tp = t, kind = k; value=_value;isconst=_isconst;}
    Type(Type* t, tyType k, int* _value,int _dim,bool _isconst) { 
        assert(t);
        arraysize = _dim*t->arraysize;
        tp = t, kind = k; value=_value;dim=_dim;isconst=_isconst;}
    Type(Type* t, tyType k, std::vector<Type*> _param,Type* rtn) {
        tp = t, kind = k;
        param = _param;
        tp=rtn;
    }
};

Type* intType(int* val,bool _isconst) { return new TY::Type(0, TY::tyType::Ty_int, val,_isconst); }
    //FIXME
Type* floatType(int* val,bool _isconst) { return new TY::Type(0, TY::tyType::Ty_float, val,_isconst); }
Type* arrayType(Type* ty,int _dim,bool isconst) { return new TY::Type(ty, TY::tyType::Ty_array, NULL,_dim); }
Type* funcType(Type* rtn, std::vector<Type*> _param) {
    return new TY::Type(0, TY::tyType::Ty_func, _param,rtn);
}
enum class tyEntry { Ty_local, Ty_global, Ty_func };
struct Entry {
    tyEntry kind;
    Type* ty;
};
struct LocVar : public Entry {
    Temp_Temp temp;
    LocVar(Type* _ty, Temp_Temp _temp) {
        assert(_ty->kind != tyType::Ty_func && _ty->kind != tyType::Ty_void);
        kind = tyEntry::Ty_local;
        ty = _ty;
        temp = _temp;
    }
};
struct GloVar : public Entry {
    Temp_Label label;
    GloVar(Type* _ty, Temp_Label _label) {
        assert(_ty->kind != tyType::Ty_func && _ty->kind != tyType::Ty_void);
        kind = tyEntry::Ty_global;
        ty = _ty;
        label = _label;
    }
};
struct EnFunc : public Entry {
    Temp_Label label;
    EnFunc(Type* _ty, Temp_Label _label) {
        assert(_ty->kind==tyType::Ty_func);
        kind = tyEntry::Ty_func;
        ty = _ty;
        label = _label;
    }
};
};  // namespace TY
#endif