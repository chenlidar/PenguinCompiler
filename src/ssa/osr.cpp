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
#include "DFSCFG.hpp"
using std::function;
using std::map;
using std::min;
using std::move;
using std::pair;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

enum class RCT { C, T, E };
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
        auto ttmp = DFSCFG::Loop_Nesting_Tree(ir);
        lnt = &ttmp;

        nextNum = 0;
        nodesz = tempDefMap.size();
        for (auto& i : tempDefMap) {
            if (!vis.count(i.first)) { DFS(i.first); }
        }

        lnt = 0;
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
                        ssaEdge[tid].push_back({static_cast<IR::Temp*>(*jt)->tempid, *jt});
                    }
                }
            }
        }
    }
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
            auto tid = it.first;
            if (!vis[tid]) {
                DFS(tid);
                Low[x] = min(Low[x], Low[tid]);
            }
            if (Num[tid] < Num[x] && ons[tid]) { Low[x] = min(Low[x], Num[tid]); }
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
    int isiv(IR::Exp* exp) {
        if (exp->kind == IR::expType::temp) {
            auto tid = static_cast<IR::Temp*>(exp)->tempid;
            if (!header.count(tid)) return -1;
            if (header[tid] != -1) { return tid; }
        }
        return -1;
    }
    RegionConst isrc(Temp_Temp x, int hder) {
        auto db = tempDefBlockMap[x];
        if (lnt->is_StrictlyDominate(db, hder)) { return {RCT::T, x}; }
        return {RCT::E, 0};
    }
    RegionConst isrc(IR::Exp* x, int hder) {
        if (x->kind == IR::expType::constx) return {RCT::C, static_cast<IR::Const*>(x)->val};
        if (x->kind == IR::expType::temp) return isrc(static_cast<IR::Temp*>(x)->tempid, hder);
        return {RCT::E, 0};
    }
    IR::Exp* rc2exp(RegionConst x) {
        if (x.kind == RCT::C) return new IR::Const(x.val);
        return new IR::Temp(x.val);
    }
    IVentry IsCandidateOperation(IR::Exp* exp) {
        RegionConst c1, c2;
        switch (exp->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(exp);
            switch (bop->op) {
            case IR::binop::T_mul:  // fall through
            case IR::binop::T_plus: {
                int v1 = isiv(bop->left), v2 = isiv(bop->right);

                if (v1 != -1) {
                    c2 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->left)->tempid]);
                    if (c2.kind != RCT::E) { return {bop->op, v1, c2}; }
                    return {bop->op, -1, c2};
                } else if (v2 != -1) {
                    c1 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->right)->tempid]);
                    if (c1.kind != RCT::E) { return {bop->op, v2, c1}; }
                    return {bop->op, -1, c1};
                }
                return {bop->op, -1, c1};
            }
            case IR::binop::T_minus: {
                int v1 = isiv(bop->left);

                if (v1 != -1) {
                    c2 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->left)->tempid]);
                    if (c2.kind != RCT::E) { return {bop->op, v1, c2}; }
                    return {bop->op, -1, c2};
                }
                return {bop->op, -1, c2};
            }
            default: return {bop->op, -1, c2};
            }
        }
        case IR::expType::temp: {
            int v1 = isiv(exp);
            return {IR::binop::T_plus, v1, {RCT::C, 0}};
        }
        default: return {IR::binop::T_plus, -1, {RCT::C, 0}};
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
    bool IsUpdateValid(IR::Stm* stm) {
        auto src = static_cast<IR::Move*>(stm)->src;
        switch (src->kind) {
        case IR::expType::binop:
            auto bi = static_cast<IR::Binop*>(src);
            if (bi->op == IR::binop::T_plus || bi->op == IR::binop::T_minus) return true;
            return false;
        case IR::expType::call: return isphifunc(stm);
        case IR::expType::temp: return true;
        default: return false;
        }
    }
    void Process(unordered_set<int>& N) {
        if (N.size() == 1) {
            auto nb = *N.begin();
            auto src = static_cast<IR::Move*>(tempDefMap[nb]->stm)->src;
            auto tive = IsCandidateOperation(src);
            if (tive.iv != -1) {
                Replace(nb, tive);
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
        IR::Exp* src = (static_cast<IR::Move*>(tempDefMap[tmp.iv]->stm)->src);
        tempDefMap[nt] = new StmList(new IR::Move(new IR::Temp(nt), src->dCopy()), 0);
        ssaEdge[nt] = ssaEdge[tmp.iv];
        header[nt] = header[tmp.iv];
        for (auto it : ssaEdge[nt]) {
            auto tid = it.first;
            if (header.count(tid) && header[tid] == header[tmp.iv]) {
                auto nx = Reduce({tmp.op, tid, tmp.rc});
                static_cast<IR::Temp*>(it.second)->tempid = nx;
            } else if (tmp.op == IR::binop::T_mul || isphifunc(tempDefMap[nt]->stm)) {
                auto nx = Apply(tmp.op, {RCT::T, tid}, tmp.rc);
                static_cast<IR::Temp*>(it.second)->tempid = nx;
            }
        }
        return nt;
    }
    Temp_Temp Apply(IR::binop op, RegionConst op1, RegionConst op2) {
        IVentry x = {op, op1.val, op2};
        if (ivmp.count(x)) return ivmp[x];
        int res;
        if (header[op1.val] != -1
            && (op2.kind == RCT::C
                || (op2.kind == RCT::T && isrc(op2.val, header[op1.val]).kind != RCT::E))) {
            res = Reduce(x);
        } else if (x.rc.kind == RCT::T && header[x.rc.val] != -1
                   && isrc(x.iv, header[x.rc.val]).kind != RCT::E) {
            res = Reduce({x.op, x.rc.val, {RCT::T, x.iv}});
        } else {
            res = Temp_newtemp();
            ivmp[x] = res;
            if (x.rc.kind == RCT::T) {

                int bl1 = tempDefBlockMap[x.iv];
                int bl2 = tempDefBlockMap[x.rc.val];
                int lb;
                int flag = 0;
                if (lnt->Block_Dfsnum(bl1) < lnt->Block_Dfsnum(bl2)) {
                    lb = bl2;
                    flag = 1;
                } else if (lnt->Block_Dfsnum(bl1) > lnt->Block_Dfsnum(bl2)) {
                    lb = bl1;
                    flag = 1;
                } else {
                    lb = bl1;
                    flag = 0;
                }
                auto stml = (IR::StmList*)(ir->nodes()->at(lb)->info);
                while (stml) {
                    auto df = getDef(stml->stm);
                    if (df) {
                        auto tid = static_cast<IR::Temp*>(*df)->tempid;
                        if (tid == x.rc.val || tid == x.iv) {
                            if (!flag) {
                                flag = 1;
                                continue;
                            }
                            stml->tail = new IR::StmList(
                                new IR::Move(new IR::Temp(res),
                                             new IR::Binop(x.op, new IR::Temp(x.iv),
                                                           new IR::Temp(x.rc.val))),
                                stml->tail);
                            break;
                        }
                    }
                }
            } else {
                auto tt = tempDefMap[x.iv];
                auto nt = new IR::StmList(
                    new IR::Move(new IR::Temp(res),
                                 new IR::Binop(x.op, new IR::Temp(x.iv), rc2exp(x.rc))),
                    tt->tail);
                tt->tail = nt;
            }
            header[res] = -1;
        }

        return res;
    }
    void Replace(Temp_Temp x, IVentry ive) {
        assert(ive.iv != -1);
        auto nx = Reduce(ive);
        for (auto it : uselog[x]) { static_cast<IR::Temp*>(it)->tempid = nx; }
        tempDefMap[x]->stm = tempDefMap[nx]->stm;
        if (isphifunc(tempDefMap[x]->stm)) {
            ir->Aphi[tempDefBlockMap[x]].erase(x);
            ir->Aphi[tempDefBlockMap[x]].insert({nx, tempDefMap[x]});
        }
        header[x] = header[ive.iv];
    }
    void ClassifyIV(unordered_set<int>& N) {
        bool IsIV = true;
        int hder = *N.begin();
        int lw = lnt->Block_Dfsnum(tempDefBlockMap[hder]);
        for (auto it : N) {
            auto nx = lnt->Block_Dfsnum(tempDefBlockMap[it]);
            if (nx < lw) {
                lw = nx;
                hder = it;
            }
        }
        for (auto it : N) {
            if (!IsUpdateValid(tempDefMap[it]->stm)) {
                IsIV = false;
            } else {
                for (auto nx : ssaEdge[it]) {
                    if (!N.count(nx.first) && isrc(nx.first, hder).kind != RCT::E) {
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
                auto tive = IsCandidateOperation(static_cast<IR::Move*>(tempDefMap[it]->stm)->src);
                if (tive.iv != -1) {
                    Replace(it, tive);
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
    unordered_map<Temp_Temp, vector<pair<Temp_Temp, IR::Exp*>>> ssaEdge;
    map<IVentry, Temp_Temp> ivmp;
    map<Temp_Temp, IVentry> ivrmp;
    DFSCFG::Loop_Nesting_Tree* lnt;
};
void SSA::Optimizer::strengthReduction() {
    auto tt = OSR(ir);
    return;
}
