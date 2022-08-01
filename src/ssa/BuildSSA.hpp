#ifndef __BUILD_SSA
#define __BUILD_SSA
#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/flowgraph.hpp"
#include "../backend/canon.hpp"
#include "CFG.hpp"
#include "./Dtree.hpp"
#include <functional>
namespace SSA {
class SSAIR;
class Optimizer {
public:
    void deadCodeElimilation();
    void constantPropagation();
    void PRE();
    SSA::SSAIR* ir;

private:
    bool isNecessaryStm(IR::Stm* stm);
    void buildTable();
    void insertPRE();
    void deletePRE();
    void buildAvail(int node, int fa);
    void buildAntic();
    void deletenode(int node, int fa);

    struct Uexp {
        IR::expType kind;
        int val;
        std::string name;
        Uexp(IR::Exp* u) {
            kind = u->kind;
            val = 0;
            name = "";
            switch (u->kind) {
            case IR::expType::name: {
                name = static_cast<IR::Name*>(u)->name;
            } break;
            case IR::expType::temp: {
                val = static_cast<IR::Temp*>(u)->tempid;
            } break;
            case IR::expType::constx: {
                val = static_cast<IR::Const*>(u)->val;
            } break;
            default: assert(0);
            }
        }
        Uexp(){}
    };
    struct Biexp {
        IR::binop op;
        Uexp l, r;
        bool operator==(const Biexp& e2) const {
            auto eq = [&](Uexp l, Uexp r) {
                if (l.kind != r.kind) return false;
                switch (l.kind) {
                case IR::expType::name: return l.name == r.name;
                case IR::expType::temp:
                case IR::expType::constx: return l.val == r.val;
                default: assert(0);
                }
            };
            return op == e2.op
                   && (eq(l, e2.l) && eq(r, e2.r)
                       || ((op == IR::binop::T_mul || op == IR::binop::T_plus) && eq(l, e2.r)
                           && eq(r, e2.l)));
        }
        Biexp(IR::binop _op, IR::Exp* _l, IR::Exp* _r) {
            op = _op;
            l = Uexp(_l);
            r = Uexp(_r);
        }
        Biexp(){}
    };
    struct hash_name {
        size_t operator()(const Biexp& p) const {
            return std::hash<int>()(static_cast<int>(p.op))
                   ^ std::hash<int>()(static_cast<int>(p.l.kind))
                   ^ std::hash<int>()(static_cast<int>(p.r.kind)) ^ std::hash<int>()(p.l.val)
                   ^ std::hash<int>()(p.r.val);
        }
    };
    typedef std::unordered_map<Biexp, int, hash_name> Vtb;
    std::vector<Vtb> avail;
    std::vector<std::unordered_map<int, Biexp>> phicomp;
    std::vector<std::list<Biexp>> anticIn, anticOut;
    std::vector<std::vector<Biexp>> exp_gen;
    std::vector<std::unordered_set<int>> tmp_gen;
};
class SSAIR : public CFG::CFGraph {
public:
    SSAIR(CANON::Block blocks);
    CANON::Block ssa2ir();
    std::unordered_map<int, std::unordered_map<Temp_Temp, IR::StmList*>> Aphi;
    SSA::Optimizer opt;
    Temp_Label endlabel;
    DTREE::Dtree* dtree;

private:
    std::unordered_map<Temp_Temp, std::vector<int>> defsites;
    std::unordered_map<Temp_Temp, std::stack<Temp_Temp>> stk;
    void placePhi();
    void rename();
    void rename(int node);
};

}  // namespace SSA
#endif