#ifndef __SSAOPTIMIZER
#define __SSAOPTIMIZER
#include "BuildSSA.hpp"
namespace SSAOPT {
class Optimizer {
public:
    SSA::SSAIR* deadCodeElimilation(SSA::SSAIR* ir);
    SSA::SSAIR* constantPropagation(SSA::SSAIR* ir);
};
}  // namespace SSAOPT

#endif