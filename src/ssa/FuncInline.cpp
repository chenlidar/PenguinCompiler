#include"FuncInline.hpp"
#include"../util/utils.hpp"
namespace INTERP{
    std::vector<std::string> FuncInline::functionInline(){
        auto result=std::vector<std::string>();
        for (auto funcname : func_name) { analyse(funcname); }
        // inline
        
        for(auto funcname:func_name){
            if(func_info[funcname].calledNum==0)continue;
            result.push_back(funcname);
        }
        return result;
    }
    void FuncInline::analyse(std::string funcname){
        IR::StmList* stmlist=func_map[funcname];
        func_info[funcname].isvoid=fenv->look(funcname)->ty->kind==TY::tyType::Ty_void;
        func_info[funcname].stksize=fenv->look(funcname)->stksize;
        func_info[funcname].ir=stmlist;
        for(;stmlist;stmlist=stmlist->tail){
            func_info[funcname].length+=1;
            IR::Stm* stm=stmlist->stm;
            if(isCall(stm)){
                func_info[funcname].callNum+=1;
                std::string calledfunc;
                if(stm->kind==IR::stmType::exp)calledfunc=getCallName(static_cast<IR::ExpStm*>(stm)->exp);
                else calledfunc=getCallName(static_cast<IR::Move*>(stm)->src);
                if(funcname==calledfunc)func_info[funcname].isrec=true;
                else if(func_info.count(calledfunc)){
                    func_info[calledfunc].calledNum+=1;
                }
            }
        }
    }
}