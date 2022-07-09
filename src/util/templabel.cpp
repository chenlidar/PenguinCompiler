#include"singleton.hpp"
#include"templabel.hpp"
#include<string>
using std::string;
string Temp_newlabel() {
    return Singleton::instance().LabelManager.newlabel();
}