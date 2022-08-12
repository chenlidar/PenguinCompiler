#include "BuildSSA.hpp"
#include "../util/utils.hpp"
namespace SSA {
void Optimizer::CME() {  // behind PRE
    for (auto node : ir->mynodes) {
        if (node->inDegree() == 0 && node->mykey != 0) continue;
        std::unordered_map<Uexp, Uexp, hash_name2> avail_mem;
        IR::StmList* stmlist = (IR::StmList*)node->nodeInfo();
        for (; stmlist; stmlist = stmlist->tail) {
            IR::Stm* stm = stmlist->stm;
            if (isLdr(stm)) {  // gen
                Uexp dst = Uexp(static_cast<IR::Move*>(stm)->dst);
                Uexp addr = Uexp(static_cast<IR::Mem*>(static_cast<IR::Move*>(stm)->src)->mem);
                if (avail_mem.count(addr)) {
                    IR::Exp* exp = avail_mem.at(addr).toExp();
                    IR::Move* mv = static_cast<IR::Move*>(stm);
                    mv->src = exp;
                } else {
                    avail_mem.insert({addr, dst});
                }
            } else if (isStr(stm)) {  // kill all,gen one
                avail_mem.clear();
                Uexp src = Uexp(static_cast<IR::Move*>(stm)->src);
                Uexp addr = Uexp(static_cast<IR::Mem*>(static_cast<IR::Move*>(stm)->dst)->mem);
                avail_mem.insert({addr, src});
            } else if (isCall(stm)) {  // kill all
                avail_mem.clear();
            }
        }
    }
}
}  // namespace SSA