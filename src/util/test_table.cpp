/* #include <iostream>
#include "table.hpp"

using namespace Table;

class NSH{
        public:
        int A,B,C;
        NSH(){}
        NSH(int a,int b,int c):A(a),B(b),C(c) {}

        bool operator==(NSH &y)
        {
            return this->A==y.A && this->B==y.B && this->C==y.C;
        }
};




int main()
{
    std::cout<<NSH(1,2,3).C<<std::endl;
    Table::Stable<NSH> S;
    S.enter(std::string("george"),NSH(1,2,3));
    S.enter(std::string("plover"),NSH(-1,-2,-3));
    std::cout<<(S.look(std::string("george")).C)<<std::endl;//3

    S.beginScope(NSH());
    S.enter(std::string("george"),NSH(10,20,30));
    S.enter(std::string("George"),NSH(-10,-20,-30));
    std::cout<<(S.look(std::string("George")).C)<<std::endl;//-30
    std::cout<<(S.look(std::string("george")).C)<<std::endl;//30

    S.endScope();
    assert(!S.exist(std::string("George")));
    std::cout<<(S.look(std::string("plover")).C)<<std::endl;//-3

    return 0;
} */