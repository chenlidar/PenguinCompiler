#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/flowgraph.hpp"
#include "CFG.hpp"
#include "Dtree.hpp"
namespace SSA {
class SSAIR : public CFG::CFGraph {
public:
    SSAIR(IR::StmList* stmlist);

private:
    DTREE::Dtree* dtree;
};
}  // namespace SSA