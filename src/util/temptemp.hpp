#ifndef __TEMPTEMP
#define __TEMPTEMP

#include "singleton.hpp"
#include <vector>
class tempManager {
   public:
    tempManager() { tempno = 1000; }
    int newtemp() {
        int ret = tempno;
        tempno++;
        return ret;
    }

   private:
    int tempno;
};
int Temp_newtemp() {
    Singleton::instance().TempManager.newtemp();
}
typedef int Temp_Temp;
typedef std::vector<Temp_Temp> Temp_TempList;
#endif