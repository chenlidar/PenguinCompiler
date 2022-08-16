#include "FuncInline.hpp"
#include "../util/utils.hpp"
namespace INTERP {
std::vector<std::string> FuncInline::functionInline() {
    auto result = std::vector<std::string>();
    for (auto funcname : func_name) { analyse(funcname); }
    // inline
    for (auto funcname : func_name) {
        // do return callself
        IR::StmList* in_ir = func_info[funcname].ir;
        IR::StmList* head = new IR::StmList(nullptr, nullptr);
        IR::StmList* tail = head;
        std::vector<IR::StmList*> stmv;
        for (auto list = in_ir; list; list = list->tail) {
            IR::Stm* stm = list->stm;
            if (isNop(stm)) continue;
            stmv.push_back(list);
        }
        bool isrec = false;
        for (int i = 0; i < stmv.size();) {
            IR::Stm* stm = stmv[i]->stm;
            bool isvalrtn = isCall(stm) && getCallName(stm) == funcname && i + 1 < stmv.size()
                            && i + 2 < stmv.size()
                            && isReturn(stm, stmv[i + 1]->stm, stmv[i + 2]->stm);
            bool isvoidrtn = isCall(stm) && getCallName(stm) == funcname && i + 1 < stmv.size()
                             && isReturn(stmv[i + 1]->stm);
            if (isvalrtn || isvoidrtn) {  // do return replace
                int cnt = 0;
                IR::ExpList paramv = getCallParam(stm);
                for (auto list = func_info[funcname].ir; list; list = list->tail) {
                    IR::Stm* stmm = list->stm;
                    if (stmm->kind == IR::stmType::label
                        && static_cast<IR::Label*>(stmm)->label == "BEGIN_" + funcname)
                        break;
                    if (stmm->kind == IR::stmType::move
                        && ((static_cast<IR::Move*>(stmm)->src->kind == IR::expType::temp
                             && static_cast<IR::Temp*>(static_cast<IR::Move*>(stmm)->src)->tempid
                                    < 4)
                            || static_cast<IR::Move*>(stmm)->src->kind == IR::expType::mem)) {
                        int dst
                            = static_cast<IR::Temp*>(static_cast<IR::Move*>(stmm)->dst)->tempid;
                        tail = tail->tail = new IR::StmList(
                            new IR::Move(new IR::Temp(dst), paramv[cnt]), nullptr);
                        cnt++;
                    }
                }
                assert(cnt == paramv.size());
                tail = tail->tail = new IR::StmList(new IR::Jump("BEGIN_" + funcname), nullptr);

            } else {
                if (isCall(stm) && getCallName(stm) == funcname) isrec = true;
                tail = tail->tail = stmv[i];
            }
            i += isvalrtn ? 3 : (isvoidrtn ? 2 : 1);
        }
        func_info[funcname].ir = head->tail;
        func_info[funcname].isrec = isrec;
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
            in_ir = in_ir->deepCopy(glabel, -func_info[funcname].stksize + 64, temp_map,
                                    label_map);  // copy without param
            IR::StmList *param_in = new IR::StmList(nullptr, nullptr), *tail;
            tail = param_in;
            IR::Stm* oldstm = func.second->stm;
            int cnt = 0;
            IR::ExpList paramv = getCallParam(oldstm);
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
            func_info[funcname].stksize += func_info[func.first].stksize - 64;
        }
    }
    // global var to local var
    bool nocall = G2Lvar();
    if (nocall)
        result.push_back("main");
    else
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
            std::string calledfunc = getCallName(stm);
            if (funcname == calledfunc)
                func_info[funcname].isrec = true;
            else if (func_info.count(calledfunc)) {
                func_info[funcname].callNum += 1;
                func_info[calledfunc].calledNum += 1;
                func_info[funcname].callpos.push_back({calledfunc, stmlist});
                for(auto name:func_info[calledfunc].Gvar)func_info[funcname].Gvar.insert(name);
            }
        }
        if(isStr(stm)){
            IR::Exp* exp=static_cast<IR::Mem*>(static_cast<IR::Move*>(stm)->dst)->mem;
            if(exp->kind==IR::expType::name)func_info[funcname].Gvar.insert(static_cast<IR::Name*>(exp)->name);
        }
        else if(isLdr(stm)){
            IR::Exp* exp=static_cast<IR::Mem*>(static_cast<IR::Move*>(stm)->src)->mem;
            if(exp->kind==IR::expType::name)func_info[funcname].Gvar.insert(static_cast<IR::Name*>(exp)->name);
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
        if (!func_info[funcname].isrec && func_info[it.first].stksize - 64 + stksizesum < maxstk) {
            result.push_back(it);
            stksizesum += func_info[it.first].stksize - 64;
        } else if (func_info[funcname].isrec
                   && func_info[it.first].stksize - 64 + stksizesum
                          <= 10 * func_info[funcname].stksize
                   && func_info[it.first].length < func_info[funcname].length) {
            result.push_back(it);
            stksizesum += func_info[it.first].stksize - 64;
        }
    }
    return result;
}
bool FuncInline::G2Lvar() {
    std::unordered_map<std::string, int> nameMap;
    for (auto it = venv->begin(); it != venv->end(); ++it) {
        TY::Entry* entry = venv->look(it->first);
        assert(entry && entry->kind == TY::tyEntry::Ty_global);
        if (entry->ty->isconst) {
            assert(entry->ty->kind != TY::tyType::Ty_array);
            continue;  // const
        }
        if (entry->ty->kind == TY::tyType::Ty_array) continue;
        Temp_Label name = static_cast<TY::GloVar*>(entry)->label;
        nameMap.insert({name, Temp_newtemp()});
    }
    IR::StmList* stmlist = func_info["main"].ir;
    bool nocall = true;
    for (auto list = stmlist; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        if (isCall(stm) && func_info.count(getCallName(stm))) {
            nocall = false;
            std::string callname=getCallName(stm);
            for(auto it:func_info[callname].Gvar)nameMap.erase(it);
        }
    }
    // if (!nocall) return false;
    // replace Gvar
    for (auto list = stmlist; list; list = list->tail) {
        IR::Stm* stm = list->stm;
        if(stm->kind!=IR::stmType::move)continue;
        IR::Move* mv=static_cast<IR::Move*>(stm);
        if (isLdr(stm)) {
            IR::Mem* mm=static_cast<IR::Mem*>(mv->src);
            if(mm->mem->kind==IR::expType::name){
                std::string name=static_cast<IR::Name*>(mm->mem)->name;
                if(nameMap.count(name)){
                    mv->src=new IR::Temp(nameMap[name]);
                }
            }
        }
        else if(isStr(stm)){
            IR::Mem* mm=static_cast<IR::Mem*>(mv->dst);
            if(mm->mem->kind==IR::expType::name){
                std::string name=static_cast<IR::Name*>(mm->mem)->name;
                if(nameMap.count(name)){
                    mv->dst=new IR::Temp(nameMap[name]);
                }
            }
        }
    }
    return nocall;
}
}  // namespace INTERP