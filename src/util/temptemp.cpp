#include "singleton.hpp"
#include "temptemp.hpp"
int Temp_newtemp() { return Singleton::instance().TempManager.newtemp(); }
std::string Temp_tempname(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap, Temp_Temp temp) {
    if (tempMap == nullptr)
        return "r" + std::to_string(temp);
    else
        return "r" + std::to_string(tempMap->at(temp));
}