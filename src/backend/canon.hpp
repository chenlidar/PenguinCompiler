#pragma once
#include <string>
#include <vector>
#include <list>
#include "../structure/treeIR.hpp"
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
    Block(StmListList* _llist, Temp_Label _label) {
        llist = _llist;
        label = _label;
    }
    Block() {}
};
StmList* linearize(Stm* stm);
Block basicBlocks(StmList* stmList);
StmList* traceSchedule(Block b);
StmList* handle(Stm* stm);
StmList* funcEntryExit1(StmList*);  // save caller save reg
ASM::InstrList* funcEntryExit2(ASM::InstrList*, bool,
                               bool);  // sink r0 when return is not void and putint in main
}  // namespace CANON