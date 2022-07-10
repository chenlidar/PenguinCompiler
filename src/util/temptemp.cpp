#include"singleton.hpp"
#include"temptemp.hpp"
int Temp_newtemp() {
    return Singleton::instance().TempManager.newtemp();
}
std::string Temp_tempname(Temp_Temp temp){
    return "r"+std::to_string(temp);
}