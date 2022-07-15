#include "quadruple.hpp"
#include "../structure/treeIR.hpp"
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
#include"../util/utils.hpp"
namespace QUADRUPLE {
IR::Stm* handle(IR::StmList* stml) {
    if (!stml)
        return nopStm();
    auto p1=stml->stm->quad();
    return new IR::Seq(p1, handle(stml->tail));
}
};  // namespace QUADRUPLE