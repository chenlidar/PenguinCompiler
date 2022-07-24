#ifndef __TEMPLABEL
#define __TEMPLABEL
#include <string>
#include <vector>
using std::string;
using std::to_string;
class labelManager {
public:
    labelManager() { labelno = 1; }
    string newlabel() {
        int ret = labelno;
        labelno++;
        return "AaBbcCL" + to_string(ret);
    }

private:
    int labelno;
};
std::string Temp_newlabel();
typedef std::string Temp_Label;
typedef std::vector<Temp_Label> Temp_LabelList;
#endif