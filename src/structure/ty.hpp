#ifndef __TY
#define __TY
namespace TY {
enum class tyType { Ty_void, Ty_array, Ty_int, Ty_float };
class Type {
   public:
    Type* tp;
    tyType kind;
    // int arity;
};
};  // namespace TY

#endif