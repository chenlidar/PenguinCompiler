#pragma once
#include <iostream>
#include <cstring>
#include <string>
typedef std::string Temp_label;
typedef struct{Temp_labelList labels;}*AS_targets;
typedef struct AS_instr_ *AS_instr;
AS_targets AS_Targets(Temp_labelList labels);
namespace ASM{
struct AS_instr{
    
};
struct AS_Oper:public AS_instr{
    std::string assem;
    Temp_tempList dst,src;

};
struct AS_Label:public AS_instr{

};
struct AS_Move:public AS_instr{

};
}