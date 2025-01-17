#ifndef __BUILD_SSA
#define __BUILD_SSA
#include "../backend/graph.hpp"
#include "../structure/treeIR.hpp"
#include "../backend/flowgraph.hpp"
#include "../backend/canon.hpp"
#include "CFG.hpp"
#include "CDG.hpp"
#include "./Dtree.hpp"
#include <map>
#include <functional>
namespace SSA {
class SSAIR;
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
    Uexp() {}
    IR::Exp* toExp() {
        switch (kind) {
        case IR::expType::name: {
            return new IR::Name(name);
        } break;
        case IR::expType::temp: {
            return new IR::Temp(val);
        } break;
        case IR::expType::constx: {
            return new IR::Const(val);
        } break;
        default: assert(0);
        }
        return nullptr;
    }
    bool operator==(const Uexp& e2) const {
        return kind == e2.kind && val == e2.val && name == e2.name;
    }
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
    Biexp() {}
    bool isTemp() { return l.kind == IR::expType::temp && r.kind == IR::expType::constx; }
    bool isConst() {
        return (l.kind == IR::expType::constx || l.kind == IR::expType::name)
               && r.kind == IR::expType::constx;
    }
    bool isExp() { return l.kind == IR::expType::temp && r.kind == IR::expType::temp; }
};
struct hash_name {
    size_t operator()(const Biexp& p) const {
        return std::hash<int>()(static_cast<int>(p.op))
               ^ std::hash<int>()(static_cast<int>(p.l.kind))
               ^ std::hash<int>()(static_cast<int>(p.r.kind)) ^ std::hash<int>()(p.l.val)
               ^ std::hash<int>()(p.r.val);
    }
};
struct hash_name2 {
    size_t operator()(const Uexp& p) const {
        return std::hash<int>()(static_cast<int>(p.kind)) ^ std::hash<std::string>()(p.name)
               ^ std::hash<int>()(p.val);
    }
};
class Optimizer {
public:
    void deadCodeElimilation();
    void constantPropagation();
    void strengthReduction();
    void PRE();
    void combExp();
    void CME();

    SSA::SSAIR* ir;

private:
    std::pair<IR::StmList*, IR::StmList*> handelexp(IR::StmList* begin, IR::StmList* end);
    std::unordered_map<int, std::vector<IR::StmList*>> usemap;
    void buildTable();
    void insertPRE(int node);
    void insertPRE();
    void deletePRE();
    void buildAvail();
    void buildAvail(int node, int fa);
    void buildAntic();
    bool buildAntic(int node);
    void deletenode(int node, int fa);

    typedef std::unordered_map<Biexp, int, hash_name> Vtb;
    std::vector<std::unordered_set<int>> avail_out;
    std::vector<std::unordered_map<int, int>> avail_map;
    Vtb G_map;
    std::unordered_map<int, Uexp> vG_map;
    std::vector<std::list<Biexp>> anticIn;
    std::vector<std::unordered_map<int, Biexp>> anticIn_map;
    std::vector<std::list<Biexp>> exp_gen;
    std::vector<std::unordered_map<int, Biexp>> exp_map;
    std::vector<std::unordered_set<int>> tmp_gen;
    Biexp U2Biexp(IR::Exp* e);
    int findGV(const Biexp& biexp);
    CDG::CDgraph* cdg;
};
class Loop {
public:
    Loop()
        : loops()
        , doneloop()
        , parent() {}
    void findLoop();
    struct LoopInfo {
        bool canUnroll, isConst;
        int step, times;
        Uexp begin, end;
        int inducetemp;
    };
    LoopInfo analyse(int loopHead);
    bool loopUnroll();
    std::pair<bool, std::pair<Uexp, int>> findCircle(int block, int temp);
    SSAIR* ir;
    std::map<int, std::set<int>> loops;
    std::set<int> doneloop;
    std::vector<int> parent;
};
class SSAIR : public CFG::CFGraph {
public:
    SSAIR(CANON::Block blocks);
    CANON::Block ssa2ir();
    void showmark() {  // func that can output ssa for debuging
        for (const auto& it : mynodes) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                stm->printIR();
                stml = stml->tail;
            }
        }
    }
    std::unordered_map<int, std::map<Temp_Temp, IR::StmList*>> Aphi;
    SSA::Optimizer opt;
    Temp_Label endlabel;
    DTREE::Dtree* dtree;
    Loop loops;

private:
    std::map<Temp_Temp, std::vector<int>> defsites;
    std::map<Temp_Temp, std::stack<Temp_Temp>> stk;
    void placePhi();
    void rename();
    void rename(int node);
};

}  // namespace SSA
#endif