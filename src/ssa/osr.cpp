#include "BuildSSA.hpp"
#include "../structure/treeIR.hpp"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <map>
#include <algorithm>
#include <functional>
#include "../util/utils.hpp"
#include "CDG.hpp"
using std::function;
using std::map;
using std::min;
using std::move;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

enum class RCT { C, T };
struct RegionConst {
    RCT kind;
    int val;
    bool operator<(const RegionConst& other) const {
        int a = static_cast<int>(kind), b = static_cast<int>(other.kind);
        return (a == b) ? val < other.val : a < b;
    }
};
struct IVentry {
    IR::binop op;
    Temp_Temp iv;
    RegionConst rc;
    bool operator<(const IVentry& other) const {
        int a = static_cast<int>(op), b = static_cast<int>(other.op);
        if (a < b)
            return true;
        else if (a == b && iv < other.iv)
            return true;
        else if (a == b && iv == other.iv && rc < other.rc)
            return true;
        return false;
    }
};

class OSR {
public:
    OSR(SSA::SSAIR* t) {
        ir = t;
        buildSSAGraph();
        callRPO();
        auto tmp = CDG::CDgraph(ir);
        gp = &tmp;
        nextNum = 0;
        nodesz = tempDefMap.size();
        for (auto& i : tempDefMap) {
            if (!vis.count(i.first)) { DFS(i.first); }
        }
    }

private:
    void buildSSAGraph() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                auto df = getDef(stm);
                auto uses = getUses(stm);
                for (auto jt : uses) {
                    uselog[static_cast<IR::Temp*>(*jt)->tempid].push_back(*jt);
                }
                if (df) {
                    auto tid = static_cast<IR::Temp*>(*df)->tempid;
                    tempDefMap[tid] = stml;
                    tempDefBlockMap[tid] = it->mykey;
                    for (auto jt : uses) {
                        ssaEdge[tid].push_back(static_cast<IR::Temp*>(*jt)->tempid);
                    }
                }
            }
        }
    }
    void rpodfs(Temp_Temp x) {
        if (rpovis.count(x)) return;
        rpovis.insert(x);
        for (auto nx : ssaEdge[x]) { rpodfs(nx); }
        rpomp[x] = ++rpoNum;
    }
    void callRPO() {}
    void push(int x) {
        stk.push_back(x);
        ons[x] = 1;
    }
    int pop() {
        int x = stk.back();
        stk.pop_back();
        ons[x] = 0;
        return x;
    }
    void DFS(int x) {
        Num[x] = nextNum++;
        vis[x] = 1;
        Low[x] = Num[x];
        push(x);
        for (auto it : ssaEdge[x]) {
            if (!vis[it]) {
                DFS(it);
                Low[x] = min(Low[x], Low[it]);
            }
            if (Num[it] < Num[x] && ons[it]) { Low[x] = min(Low[x], Num[it]); }
        }
        if (Low[x] == Num[x]) {
            unordered_set<int> SCC;
            int t;
            do {
                t = pop();
                SCC.insert(t);
            } while (t != x);
            Process(SCC);
        }
    }
    bool isiv(IR::Exp* exp) {
        if (exp->kind == IR::expType::temp) {
            return ivrmp.count(static_cast<IR::Temp*>(exp)->tempid);
        }
        return false;
    }
    bool isrc(Temp_Temp x, int hder) {

        auto db = tempDefBlockMap[x];
        for (auto jt : gp->CDnode[hder]) {
            if (jt == db) return true;
        }
        return false;
    }
    bool isrc(IR::Exp* x, int hder) {
        if (x->kind == IR::expType::constx) return true;
        if (x->kind == IR::expType::temp) return isrc(static_cast<IR::Temp*>(x)->tempid, hder);
        return false;
    }
    IR::Exp* rc2exp(RegionConst x) {
        if (x.kind == RCT::C) return new IR::Const(x.val);
        return new IR::Temp(x.val);
    }
    bool IsCandidateOperation(IR::Exp* exp, int hder) {
        switch (exp->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(exp);
            switch (bop->op) {
            case IR::binop::T_mul:  // fall through
            case IR::binop::T_plus: {
                return (isiv(bop->left) && isrc(bop->right, hder))
                       || (isiv(bop->right) && isrc(bop->left, hder));
            }
            case IR::binop::T_minus: {
                return isiv(bop->left) && isrc(bop->right, hder);
            }
            default: return false;
            }
        }
        case IR::expType::temp: {
            return isiv(exp);
        }
        default: return false;
        }
    }
    RegionConst getRC(IR::Exp* exp) {
        switch (exp->kind) {
        case IR::expType::constx: {
            return {RCT::C, static_cast<IR::Const*>(exp)->val};
        }
        case IR::expType::temp: {
            return {RCT::T, static_cast<IR::Temp*>(exp)->tempid};
        }
        default: assert(0);
        }
    }
    IVentry getIVentry(IR::Exp* exp, int hder) {
        switch (exp->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(exp);
            switch (bop->op) {
            case IR::binop::T_mul:  // fall through
            case IR::binop::T_plus: {
                if (isiv(bop->left) && isrc(bop->right, hder)) {
                    return {bop->op, static_cast<IR::Temp*>(bop->left)->tempid, getRC(bop->right)};
                }
                if (isiv(bop->right) && isrc(bop->left, hder)) {
                    return {bop->op, static_cast<IR::Temp*>(bop->right)->tempid, getRC(bop->left)};
                }
                assert(0);
            }
            case IR::binop::T_minus: {
                if (isiv(bop->left) && isrc(bop->right, hder)) {
                    return {bop->op, static_cast<IR::Temp*>(bop->left)->tempid, getRC(bop->right)};
                }
            }
            default: assert(0);
            }
        }
        case IR::expType::temp: {
            return {IR::binop::T_plus, static_cast<IR::Temp*>(exp)->tempid, {RCT::C, 0}};
        }
        default: assert(0);
        }
    }
    bool IsUpdateValid(IR::Stm* stm) {
        auto src = static_cast<IR::Move*>(stm)->src;
        switch (src->kind) {
        case IR::expType::binop:
            auto bi = static_cast<IR::Binop*>(src);
            if (bi->op == IR::binop::T_plus || bi->op == IR::binop::T_minus) return true;
            return false;
        case IR::expType::call: auto cl = static_cast<IR::Call*>(src); return cl->fun[0] == '$';
        case IR::expType::temp: return true;
        default: return false;
        }
    }
    void Process(unordered_set<int>& N) {
        if (N.size() == 1) {
            auto nb = *N.begin();
            auto src = static_cast<IR::Move*>(tempDefMap[nb]->stm)->src;
            if (IsCandidateOperation(src, tempDefBlockMap[nb])) {
                Replace(nb, tempDefBlockMap[nb]);
            } else
                header[nb] = -1;
        } else
            ClassifyIV(N);
    }
    Temp_Temp Reduce(IVentry tmp) {
        // auto df = (static_cast<IR::Move*>(tempDefMap[x]->stm))->src;
        // auto tmp = getIVentry(df);
        if (ivmp.count(tmp)) { return ivmp[tmp]; }
        auto nt = Temp_newtemp();
        ivmp[tmp] = nt;
        tempDefMap[nt]
            = new StmList(new IR::Move(new IR::Temp(nt), tempDefMap[tmp.iv]->stm->quad()), 0);
        ssaEdge[nt] = ssaEdge[tmp.iv];
        header[nt] = header[tmp.iv];
        for (auto it : ssaEdge[nt]) {
            if (header[it] == header[tmp.iv]) {
                auto nx = Reduce({tmp.op, it, tmp.rc});
                for (auto jt : uselog[it]) static_cast<IR::Temp*>(jt)->tempid = nx;
            } else if (tmp.op == IR::binop::T_mul || isphifunc(tempDefMap[nt]->stm)) {
                auto nx = Apply({tmp.op, it, tmp.rc});
                for (auto jt : uselog[it]) static_cast<IR::Temp*>(jt)->tempid = nx;
            }
        }
        return nt;
    }
    Temp_Temp Apply(IVentry x) {
        if (ivmp.count(x)) return ivmp[x];
        int res;
        if (header[x.iv] != -1 && isrc(x.rc.val, header[x.iv])) {
            res = Reduce(x);
        } else if (x.rc.kind == RCT::T && header[x.rc.val] != -1 && isrc(x.iv, header[x.rc.val])) {
            res = Reduce({x.op, x.rc.val, {RCT::T, x.iv}});
        } else {
            res = Temp_newtemp();
            ivmp[x] = res;
            if (x.rc.kind == RCT::T) {
                int bl1 = tempDefBlockMap[x.iv];
                int bl2 = tempDefBlockMap[x.rc.val];
                int lb = bl2;
                for (auto it : gp->CDnode[bl1]) {
                    if (it == bl2) {
                        lb = bl1;
                        break;
                    }
                }
                auto stml = (IR::StmList*)(ir->nodes()->at(lb)->info);
                IR::StmList* tg = 0;
                while (stml) {
                    auto df = getDef(stml->stm);
                    if (df) {
                        auto tid = static_cast<IR::Temp*>(*df)->tempid;
                        if (tid == x.iv) {
                            tg = stml;
                        } else if (tid == x.rc.val) {
                            tg = stml;
                        }
                    }
                }
                assert(tg);
                auto nt = new IR::StmList(
                    new IR::Move(new IR::Temp(res),
                                 new IR::Binop(x.op, new IR::Temp(x.iv), rc2exp(x.rc))),
                    tg->tail);
                tg->tail = nt;

            } else {
                auto tt = tempDefMap[x.iv];
                auto nt = new IR::StmList(
                    new IR::Move(new IR::Temp(res),
                                 new IR::Binop(x.op, new IR::Temp(x.iv), rc2exp(x.rc))),
                    tt->tail);
                tt->tail = nt;
            }
            //????
        }

        return res;
    }
    void Replace(Temp_Temp x, int hder) {
        auto df = static_cast<IR::Move*>(tempDefMap[x]->stm)->src;
        auto iventry = getIVentry(df, hder);
        auto nx = Reduce(iventry);
        for (auto it : uselog[x]) { static_cast<IR::Temp*>(it)->tempid = nx; }
        header[x] = header[iventry.iv];
    }
    void ClassifyIV(unordered_set<int>& N) {
        bool IsIV = true;
        int hder = *N.begin();
        int lw = rpomp[hder];
        for (auto it : N) {
            if (rpomp[it] < lw) {
                lw = rpomp[it];
                hder = it;
            }
        }
        for (auto it : N) {
            if (!IsUpdateValid(tempDefMap[it]->stm)) {
                IsIV = false;
            } else {
                for (auto nx : ssaEdge[it]) {
                    if (!N.count(nx) && !isrc(nx, hder)) {
                        IsIV = false;
                        break;
                    }
                }
            }
        }
        if (IsIV) {
            for (auto it : N) { header[it] = hder; }
        } else {
            for (auto it : N) {
                if (IsCandidateOperation(static_cast<IR::Move*>(tempDefMap[it]->stm)->src, hder)) {
                    Replace(it, hder);
                } else
                    header[it] = -1;
            }
        }
    }

private:
    SSA::SSAIR* ir;
    int nextNum, nodesz;
    unordered_set<Temp_Temp> rpovis;
    int rpoNum;
    unordered_map<Temp_Temp, int> rpomp;
    unordered_map<Temp_Temp, int> vis, Num, Low, header;
    vector<Temp_Temp> stk, ons;
    vector<Temp_Temp, IR::StmList*> tempDefMap;
    vector<Temp_Temp, int> tempDefBlockMap;
    unordered_map<Temp_Temp, vector<IR::Exp*>> uselog;
    unordered_map<Temp_Temp, vector<Temp_Temp>> ssaEdge;
    map<IVentry, Temp_Temp> ivmp;
    map<Temp_Temp, IVentry> ivrmp;
    CDG::CDgraph* gp;
};
void SSA::Optimizer::strengthReduction() {
    auto tt = OSR(ir);
    return;
}
