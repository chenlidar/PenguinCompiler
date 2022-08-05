#ifndef __SINGLETON
#define __SINGLETON
#include "templabel.hpp"
#include "temptemp.hpp"
#include "muldiv.hpp"
class Singleton {
public:
    // get any instance item by Singleton::instance().xxx
    // xxx is a public variable or method
    static Singleton& instance() {
        static Singleton* ins = new Singleton();
        return *ins;
    }

private:
    Singleton() {}

public:
    tempManager TempManager;
    labelManager LabelManager;
    MULOPT mulopt;
};
#endif
