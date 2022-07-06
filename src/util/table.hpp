#ifndef __TABLE
#define __TABLE

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <stack>

/*

    Use: table <typea , type b>
    Or:  Stable <string, type b>

*/

namespace Table{

    template <typename T_key,typename T_value>
    class table
    {
    private:
        std::unordered_map<T_key,T_value> map;
        std::stack<std::pair<T_key,std::pair<T_value,bool> > > stk;
    public:
        table(/* args */){}
        ~table(){}

        void enter(T_key,T_value);
        bool exist(T_key);
        T_value look(T_key);
        std::pair<T_key,T_value> pop();
    };

    template <typename T_value>
    class Stable
    {
    private:
        table<std::string,T_value> table;
    public:
        Stable(/* args */){}
        ~Stable(){}

        void enter(std::string,T_value);
        T_value look(std::string);
        bool exist(std::string);
        void beginScope(T_value useless);//anything  
        void endScope();
        std::pair<std::string,T_value> pop();
    };

    // implementation

    template <typename T_key,typename T_value>
    void table<T_key,T_value>::enter(T_key key,T_value value)
    {
        if(this->map.count(key))
            this->stk.push(
                std::make_pair(key,std::make_pair(map[key],true))
            );
        else
            this->stk.push(
                std::make_pair(key,std::make_pair(map[key],false))
            );
        map[key] = value;
    }

    template <typename T_key,typename T_value>
    bool table<T_key,T_value>::exist(T_key key)
    {
        return map.find(key)!=map.end();
    }

    template <typename T_key,typename T_value>
    T_value table<T_key,T_value>::look(T_key key)
    {
        return map.find(key)->second;
    }

    template <typename T_key,typename T_value>
    std::pair<T_key,T_value> table<T_key,T_value>::pop()
    {
        assert(stk.size());
        std::pair<T_key,T_value> ret = *map.find(stk.top().first);
        if(stk.top().second.second)
            map[stk.top().first] = stk.top().second.first;
        else
            map.erase(stk.top().first);
        stk.pop();
        
        return ret;
    }


    //Stable

    template <typename T_value>
    void Stable<T_value>::enter(std::string str,T_value value)
    {
        this->table.enter(str,value);
    }

    template <typename T_value>
    bool Stable<T_value>::exist(std::string str)
    {
        return this->table.exist(str);
    }

    template <typename T_value>
    T_value Stable<T_value>::look(std::string str)
    {
        return this->table.look(str);
    }

    template <typename T_value>
    void Stable<T_value>::beginScope(T_value useless)
    {
        this->enter(std::string("[_A__SCOPE_]"),useless);
    }

    template <typename T_value>
    void Stable<T_value>::endScope()
    {
        while((this->pop().first) != std::string("[_A__SCOPE_]"));
    }

    template <typename T_value>
    std::pair<std::string,T_value> Stable<T_value>::pop()
    {
        return this->table.pop();
    }
    
}



#endif
