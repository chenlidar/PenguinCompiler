#ifndef __TILE
#define __TILE

#include "../structure/assem.h"
#include "../structure/treeIR.hpp"

ASM::InstrList* GenAsm(IR::StmList* irls);
#endif