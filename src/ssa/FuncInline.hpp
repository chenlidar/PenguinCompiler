#include "../structure/treeIR.hpp"
#include "../util/table.hpp"
#include <set>
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
        for (auto it = venv->begin(); it != venv->end(); ++it) {
            TY::Entry* entry = venv->look(it->first);
            assert(entry && entry->kind == TY::tyEntry::Ty_global);
            if (entry->ty->isconst) {
                assert(entry->ty->kind != TY::tyType::Ty_array);
                continue;  // const
            }
            Temp_Label name = static_cast<TY::GloVar*>(entry)->label;
            glabel.insert(name);
        }
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
        std::vector<std::pair<std::string, IR::StmList*>> callpos;
        std::set<std::string> Gvar;
        Info()
            : Gvar() {
            ir = nullptr;
            calledNum = callNum = length = stksize = 0;
            isrec = isvoid = false;
        }
    };
    std::unordered_map<std::string, Info> func_info;

private:
    const int maxstk = 1e8;
    std::unordered_set<std::string> glabel;
    std::unordered_map<std::string, IR::StmList*> func_map;
    std::vector<std::string> func_name;
    Table::Stable<TY::Entry*>* venv;
    Table::Stable<TY::EnFunc*>* fenv;
    void analyse(std::string name);
    std::vector<std::pair<std::string, IR::StmList*>> getInlinePos(std::string funcname);
    bool G2Lvar();
};
}  // namespace INTERP