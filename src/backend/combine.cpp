#include "../structure/treeIR.hpp"
#include "../util/utils.hpp"
#include <unordered_map>
#include <algorithm>
using std::swap;
using std::unordered_map;
void tempCombine(IR::StmList* stml) {
    auto it = stml;
    unordered_map<Temp_Temp, IR::Exp**> use;
    unordered_map<Temp_Temp, IR::StmList*> def;
    unordered_map<Temp_Temp, int> usecond, defcond;
    while (it) {
        auto df = getDef(it->stm);
        if (df) {
            auto tid = static_cast<IR::Temp*>(*df)->tempid;
            defcond[tid]++;
            def[tid] = it;
        }
        auto uses = getUses(it->stm);
        for (auto u : uses) {
            auto tid = static_cast<IR::Temp*>(*u)->tempid;
            usecond[tid]++;
            use[tid] = u;
        }
        it = it->tail;
    }
    for (auto it : usecond) {
        auto tid = it.first;
        if (!defcond.count(tid)) {
            //???
            continue;
        }
        if (defcond[tid] == 1 && it.second == 1) {
            swap(*(use[tid]), static_cast<IR::Move*>(def[tid]->stm)->src);
            delete def[tid]->stm;
            def[tid]->stm = nopStm();
        }
    }
}