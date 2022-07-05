#ifndef __TEMPTEMP
#define __TEMPTEMP

#include "singleton.hpp"
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
#endif