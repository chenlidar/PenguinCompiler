#ifndef __CFG
#define __CFG
#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/canon.hpp"
#include <unordered_set>
namespace CFG {
class CFGraph : public GRAPH::Graph {  // Node->info:IR::Stmlist*--a block begin with label,end
                                       // with jump/cjump
public:
    CFGraph(CANON::Block blocks);
    std::vector<std::unordered_set<Temp_Temp> > orig;

private:
    std::unordered_map<Temp_Label, GRAPH::Node*> LNTable;
};
}  // namespace CFG
#endif