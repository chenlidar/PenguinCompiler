#pragma once
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include "util/templabel.hpp"
#include "util/temptemp.hpp"

namespace ASM{
typedef Temp_labelList Targets;
struct Instr{
    virtual void print()=0;
};
typedef std::vector<Instr> InstrList;
struct Oper:public Instr{
    std::string assem;
    Temp_tempList dst,src;
    Targets jumps;
    Oper(std::string _assem,Temp_tempList &_dst,Temp_tempList &_src,Targets _jumps):
        assem(_assem),dst(_dst),src(_src),jumps(_jumps){}
    void print(){
        int i=0;
        for(auto d:dst){
            std::string s=std::to_string(i++);
            assem = assem.replace(assem.find("`d"+s), 1,std::to_string(d));
        }
        i=0;
        for(auto sr:src){
            std::string s=std::to_string(i++);
            assem = assem.replace(assem.find("`s"+s), 1,std::to_string(sr));
        }
        std::cout<<assem<<std::endl;
    }
};
struct Label:public Instr{
    std::string assem;
    Temp_label label;
    Label(std::string _assem,Temp_label _label):assem(_assem),label(_label){}
    void print(){
        std::cout<<label<<":"<<std::endl;
    }
};
struct Move:public Instr{
    std::string assem;
    Temp_temp dst,src;
    Move(std::string _assem,Temp_temp _dst,Temp_temp _src):
        assem(_assem),dst(_dst),src(_src){}
    void print(){
        assem = assem.replace(assem.find("`d0"), 1,std::to_string(dst));
        assem = assem.replace(assem.find("`s0"), 1,std::to_string(src));
        std::cout<<assem<<std::endl;
    }
};
struct Proc{
    InstrList body;
    void print(){
        for(auto instr:body){
            instr.print();
        }
    }
};
}