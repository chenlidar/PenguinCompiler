#include "../structure/treeIR.hpp"
#include "../util/table.hpp"
namespace INTERP {
class FuncInline {
public:
    FuncInline(Table::Stable<TY::Entry*>* _venv, Table::Stable<TY::EnFunc*>* _fenv,
               std::vector<std::string> _func_name,
               std::unordered_map<std::string, IR::StmList*> _func_map) {
        venv = _venv;
        fenv = _fenv;
        func_name = _func_name;
        func_map = _func_map;
        for (auto funcname : func_name) { func_info[funcname] = Info(); }
    }
    std::vector<std::string> functionInline();
    struct Info {
        IR::StmList* ir;
        int calledNum, callNum;
        int length;
        int stksize;
        bool isrec;
        bool isvoid;
        std::vector<std::pair<std::string,IR::StmList*>> callpos;
        Info() {}
    };
    std::unordered_map<std::string, Info> func_info;

private:
    const int maxstk=1e8;
    std::unordered_map<std::string, IR::StmList*> func_map;
    std::vector<std::string> func_name;
    Table::Stable<TY::Entry*>* venv;
    Table::Stable<TY::EnFunc*>* fenv;
    void analyse(std::string name);
    std::vector<std::pair<std::string,IR::StmList*>> getInlinePos(std::string funcname);
};
}  // namespace INTERP