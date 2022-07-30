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
    void deletenode(int node,int fa);

    struct Biexp {
        IR::binop op;
        IR::Exp *l, *r;
        bool operator==(const Biexp& e2) const {
            auto eq = [&](IR::Exp* l, IR::Exp* r) {
                if (l->kind != r->kind) return false;
                switch (l->kind) {
                case IR::expType::name:
                    return static_cast<IR::Name*>(l)->name == static_cast<IR::Name*>(r)->name;
                case IR::expType::temp:
                    return static_cast<IR::Temp*>(l)->tempid == static_cast<IR::Temp*>(r)->tempid;
                case IR::expType::constx:
                    return static_cast<IR::Const*>(l)->val == static_cast<IR::Const*>(r)->val;
                default: {
                    std::cerr<<"ERR: "<<static_cast<int>(l->kind)<<"\n";
                    assert(0);}
                }
            };
            return op == e2.op
                   && (eq(l, e2.l) && eq(r, e2.r)
                       || ((op == IR::binop::T_mul || op == IR::binop::T_plus) && eq(l, e2.r)
                           && eq(r, e2.l)));
        }
        Biexp(IR::binop _op, IR::Exp* _l, IR::Exp* _r) {
            op = _op;
            l = _l;
            r = _r;
        }
        Biexp() {}
    };
    struct hash_name {
        size_t operator()(const Biexp& p) const {
            return std::hash<int>()(static_cast<int>(p.op)) ^ std::hash<uint64_t>()((uint64_t)p.l)
                   ^ std::hash<uint64_t>()((uint64_t)p.r);
        }
    };
    typedef std::unordered_map<Biexp, int, hash_name> Vtb;
    std::vector<Vtb> avail;
    std::vector<std::unordered_map<int,Biexp>> phicomp;
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