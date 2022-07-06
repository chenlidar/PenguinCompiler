#ifndef __TEMPLABEL
#define __TEMPLABEL
#include <string>
#include <vector>
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
typedef std::string Temp_label;
typedef std::vector<std::string> Temp_labelList; 
#endif