
#include "tile.hpp"
#include "../structure/assem.h"
#include "../structure/treeIR.hpp"

ASM::InstrList* GenAsm(IR::StmList* irls) {
    auto it = irls;
    auto ret = new ASM::InstrList();
    while (it) {
        it->stm->ir2asm(ret);
        it = it->tail;
    }
    return ret;
}
