#include"singleton.hpp"
#include"temptemp.hpp"
int Temp_newtemp() {
    return Singleton::instance().TempManager.newtemp();
}