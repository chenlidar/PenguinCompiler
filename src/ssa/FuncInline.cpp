#include "FuncInline.hpp"
#include "../util/utils.hpp"
namespace INTERP {
std::vector<std::string> FuncInline::functionInline() {
    auto result = std::vector<std::string>();
    for (auto funcname : func_name) { analyse(funcname); }
    // inline
    for (auto funcname : func_name) {
        auto funcPos = getInlinePos(funcname);
        // do inline
        
    }
    for (auto funcname : func_name) {
        if (func_info[funcname].calledNum == 0) continue;
        result.push_back(funcname);
    }
    return result;
}
void FuncInline::analyse(std::string funcname) {
    IR::StmList* stmlist = func_map[funcname];
    func_info[funcname].isvoid = fenv->look(funcname)->ty->kind == TY::tyType::Ty_void;
    func_info[funcname].stksize = fenv->look(funcname)->stksize;
    func_info[funcname].ir = stmlist;
    for (; stmlist; stmlist = stmlist->tail) {
        func_info[funcname].length += 1;
        IR::Stm* stm = stmlist->stm;
        if (isCall(stm)) {
            std::string calledfunc;
            if (stm->kind == IR::stmType::exp)
                calledfunc = getCallName(static_cast<IR::ExpStm*>(stm)->exp);
            else
                calledfunc = getCallName(static_cast<IR::Move*>(stm)->src);
            if (funcname == calledfunc)
                func_info[funcname].isrec = true;
            else if (func_info.count(calledfunc)) {
                func_info[funcname].callNum += 1;
                func_info[calledfunc].calledNum += 1;
                func_info[funcname].callpos.push_back({calledfunc, stmlist});
            }
        }
    }
}
std::vector<std::pair<std::string, IR::StmList*>> FuncInline::getInlinePos(std::string funcname) {
    // now all func except recur func can be inline
    auto result = std::vector<std::pair<std::string, IR::StmList*>>();
    int stksizesum = func_info[funcname].stksize;
    for (auto it : func_info[funcname].callpos) {
        std::string fn = it.first;
        if (func_info[it.first].isrec) continue;
        if (!func_info[funcname].isrec && func_info[it.first].stksize + stksizesum < maxstk) {
            result.push_back(it);
            stksizesum += func_info[it.first].stksize;
        } else if (func_info[funcname].isrec
                   && func_info[it.first].stksize + stksizesum <= 2 * func_info[funcname].stksize
                   && func_info[it.first].length < func_info[funcname].length) {
            result.push_back(it);
            stksizesum += func_info[it.first].stksize;
        }
    }
}
}  // namespace INTERP