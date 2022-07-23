#include "optimizer.hpp"
#include "BuildSSA.hpp"
#include "../structure/treeIR.hpp"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include "../util/utils.hpp"
using std::move;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace SSAOPT {
struct TempInfo {
    IR::Stm* def;
    // vector<IR::Stm*> use;
};
bool isNecessaryStm(IR::Stm* stm) {
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind == IR::expType::mem) return true;
        // fixme: all funcs have side-effect?
        if (movestm->src->kind == IR::expType::call) return true;
        if (movestm->dst->kind == IR::expType::temp
            && isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid))
            return true;
        return false;
    }
    case IR::stmType::exp: {
        auto expstm = static_cast<IR::ExpStm*>(stm);
        // fixme: all funcs have side-effect?
        if (expstm->exp->kind == IR::expType::call) return true;
        return false;
    }
    case IR::stmType::cjump: return false;
    case IR::stmType::label: return false;
    case IR::stmType::seq: assert(0);
    case IR::stmType::jump: return false;
    default: assert(0);
    }
    return false;
}
SSA::SSAIR* SSAOPT::Optimizer::deadCodeElimilation(SSA::SSAIR* ir) {
    unordered_map<Temp_Temp, TempInfo> tempTable;
    unordered_map<IR::Stm*, int> stmBlockmap;
    unordered_set<IR::Stm*> ActivatedStm;
    queue<IR::Stm*> Curstm;
    unordered_map<int, IR::Stm*> blockJumpStm;
    auto setup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                // set up stm-block mapping
                stmBlockmap[stm] = it->mykey;
                // set up def-stm mapping
                auto df = getDef(stm);
                if (df) tempTable[*df].def = stm;
                // set up seed stm
                bool necessary = isNecessaryStm(stm);
                if (necessary) {
                    ActivatedStm.insert(stm);
                    Curstm.push(stm);
                }
                // auto us = getUses(stm);
                // for(auto it:us){
                //     //maybe same stm pushed into info
                //     tempTable[*it].use.push_back(stm);
                // }

                // set up jump
                if (!stml->tail) {
                    if (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump) {
                        blockJumpStm[it->mykey] = stm;
                    }
                }
                stml = stml->tail;
            }
        }
    };
    auto bfsMark = [&]() {
        while (!Curstm.empty()) {
            auto stm = Curstm.front();
            Curstm.pop();
            // upload to the def of uses
            auto uses = getUses(stm);
            for (auto it : uses) {
                auto def = tempTable[*it].def;
                if (!ActivatedStm.count(def)) {
                    ActivatedStm.insert(def);
                    Curstm.push(def);
                }
            }
            // upload to branch stm
            auto block = ir->nodes()->at(stmBlockmap[stm]);
            auto labelstm = ((IR::StmList*)(block->info))->stm;
            if (!ActivatedStm.count(labelstm)) {
                ActivatedStm.insert(labelstm);
                for (auto it : *(block->pred())) {
                    auto jmp = blockJumpStm[it->mykey];
                    if (!ActivatedStm.count(jmp)) {
                        ActivatedStm.insert(jmp);
                        Curstm.push(jmp);
                    }
                }
            }
        }
    };
    setup();
    bfsMark();
}
SSA::SSAIR* SSAOPT::Optimizer::constantPropagation(SSA::SSAIR* ir) {}
}  // namespace SSAOPT