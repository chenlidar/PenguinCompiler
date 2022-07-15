#include "quadruple.hpp"
#include "../structure/treeIR.hpp"
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
namespace QUADRUPLE {
IR::Stm* handle(IR::StmList* stml) {
    if (!stml)
        return 0;
    return new IR::Seq(stml->stm->quad(), handle(stml->tail));
}
};  // namespace QUADRUPLE