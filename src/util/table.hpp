#ifndef __TABLE
#define __TABLE

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <stack>

namespace Table{

    template <typename T_key,typename T_value>
    class table
    {
    private:
        std::unordered_map<T_key,T_value> map;
        std::stack<std::pair<T_key,std::pair<T_value,bool> > > stk;
    public:
        table(/* args */);
        ~table();

        void enter(T_key,T_value);
        T_value look(T_key);
        T_value pop();
    };

    template <typename T_value>
    class Stable
    {
    private:
        table<std::string,T_value> table;
    public:
        Stable(/* args */);
        ~Stable();

        void enter(std::string,T_value);
        T_value look(std::string);
        void beginScope();
        void endScope();
        T_value pop();
    };
}

#endif
