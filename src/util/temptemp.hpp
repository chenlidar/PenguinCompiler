#ifndef __TEMPTEMP
#define __TEMPTEMP

#include <vector>
#include <unordered_map>
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
int Temp_newtemp();
typedef int Temp_Temp;
typedef std::vector<Temp_Temp> Temp_TempList;
std::string Temp_tempname(std::unordered_map<Temp_Temp, Temp_Temp>*, Temp_Temp);
#endif