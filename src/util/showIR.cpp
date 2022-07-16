#include "showIR.hpp"
#include "../structure/treeIR.hpp"
void showir(IR::StmList* sl) {
    while (sl) {
        sl->stm->printIR();
        sl = sl->tail;
    }
}