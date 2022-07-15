#include "quadruple.hpp"
#include "../structure/treeIR.hpp"
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
#include"../util/utils.hpp"
namespace QUADRUPLE {
IR::Stm* handle(IR::StmList* stml) {
    IR::Stm *ret=nopStm();
    for(;stml;stml=stml->tail){
        auto p1=stml->stm->quad();
        ret=new IR::Seq(ret,p1);
    }
    return ret;
}
};  // namespace QUADRUPLE