#ifndef __CFG
#define __CFG
#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
namespace CFG {
class CFGraph : public GRAPH::Graph {
public:
    CFGraph(IR::StmList* stmlist);
    private:
    std::unordered_map<Temp_Label, GRAPH::Node*> LNTable;
};
}  // namespace CFG
#endif