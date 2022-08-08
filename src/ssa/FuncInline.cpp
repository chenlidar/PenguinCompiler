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
        for (auto func : funcPos) {
            IR::StmList* in_ir = func_info[func.first].ir;
            for (; in_ir; in_ir = in_ir->tail) {
                if (in_ir->stm->kind == IR::stmType::label
                    && static_cast<IR::Label*>(in_ir->stm)->label == "BEGIN_" + func.first)
                    break;
            }
            std::unordered_map<int, int> temp_map;
            std::unordered_map<std::string, std::string> label_map;
            assert(in_ir);
            in_ir = in_ir->deepCopy(glabel, -func_info[funcname].stksize, temp_map,
                                    label_map);  // copy without param
            IR::StmList *param_in = new IR::StmList(nullptr, nullptr), *tail;
            tail = param_in;
            int cnt = 0;
            IR::ExpList paramv;
            IR::Stm* oldstm = func.second->stm;
            if (oldstm->kind == IR::stmType::exp) {
                paramv = static_cast<IR::Call*>(static_cast<IR::ExpStm*>(oldstm)->exp)->args;
            } else {
                paramv
                    = static_cast<IR::Call*>(static_cast<IR::Move*>(func.second->stm)->src)->args;
            }
            for (auto list = func_info[func.first].ir; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                if (stm->kind == IR::stmType::label
                    && static_cast<IR::Label*>(stm)->label == "BEGIN_" + func.first)
                    break;
                if (stm->kind == IR::stmType::move
                    && ((static_cast<IR::Move*>(stm)->src->kind == IR::expType::temp
                         && static_cast<IR::Temp*>(static_cast<IR::Move*>(stm)->src)->tempid < 4)
                        || static_cast<IR::Move*>(stm)->src->kind == IR::expType::mem)) {
                    int dst = static_cast<IR::Temp*>(static_cast<IR::Move*>(stm)->dst)->tempid;
                    if (temp_map.count(dst))
                        tail = tail->tail = new IR::StmList(
                            new IR::Move(new IR::Temp(temp_map[dst]), paramv[cnt]), nullptr);
                    cnt++;
                }
            }
            assert(cnt == paramv.size());
            tail->tail = in_ir;
            tail = getEnd(in_ir);
            std::string exitlabel = label_map.count(func.first + "_RETURN3124")
                                        ? label_map.at(func.first + "_RETURN3124")
                                        : Temp_newlabel();
            tail = tail->tail = new IR::StmList(new IR::Label(exitlabel), nullptr);
            if (oldstm->kind == IR::stmType::move) {
                int rettmep = temp_map.count(0) ? temp_map.at(0) : Temp_newtemp();
                tail = tail->tail = new IR::StmList(
                    new IR::Move(static_cast<IR::Move*>(oldstm)->dst, new IR::Temp(rettmep)),
                    nullptr);
            }
            tail->tail = func.second->tail;
            func.second->tail = param_in->tail;
            func.second->stm = nopStm();
            func_info[funcname].stksize += func_info[func.first].stksize;
        }
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
    if (funcname == "main") func_info[funcname].calledNum = 1;
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
    return result;
}
}  // namespace INTERP