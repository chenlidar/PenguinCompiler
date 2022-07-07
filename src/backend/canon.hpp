#pragma once
#include <string>
#include <vector>
#include <list>
#include "treeIR.hpp"
#include "../util/temptemp.hpp"
#include "../util/templabel.hpp"
#include "../util/table.hpp"
using namespace IR;
namespace CANON {
struct StmExp {
    Stm* stm;
    Exp* exp;
    StmExp(Stm* _stm, Exp* _exp)
        : stm(_stm)
        , exp(_exp) {}
};
struct StmListList {
    StmList* head;
    StmListList* tail;
    StmListList(StmList* _head, StmListList* _tail)
        : head(_head)
        , tail(_tail) {}
};
struct Block {
    StmListList* llist;
    Temp_Label label;
};
StmList* linearize(Stm* stm);
Block basicBlocks(StmList* stmList);
StmList* traceSchedule(Block b);
}  // namespace CANON