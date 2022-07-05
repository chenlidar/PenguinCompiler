#ifndef __TEMPLABEL
#define __TEMPLABEL
#include <string>
#include "singleton.hpp"
using std::string;
using std::to_string;
class labelManager {
   public:
    labelManager() { labelno = 1; }
    string newlabel() {
        int ret = labelno;
        labelno++;
        return "L" + to_string(ret);
    }

   private:
    int labelno;
};
string Temp_newlabel() {
    return Singleton::instance().LabelManager.newlabel();
}
#endif