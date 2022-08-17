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
struct OPR {
    RCT kind;
    int val;
    bool operator<(const OPR& other) const {
        int a = static_cast<int>(kind), b = static_cast<int>(other.kind);
        return (a == b) ? val < other.val : a < b;
    }
    bool operator==(const OPR& other) const { return kind == other.kind && val == other.val; }
};
OPR eopr() { return {RCT::E, -1}; }
struct IVentry {
    IR::binop op;
    OPR iv;
    OPR rc;
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
std::vector<pair<OPR, IR::Exp**>> getOps(IR::Stm* stm) {
    std::vector<pair<OPR, IR::Exp**>> uses;
    auto gexp = [&](IR::Exp*& x) {
        switch (x->kind) {
        case IR::expType::constx: {
            uses.push_back({{RCT::C, static_cast<IR::Const*>(x)->val}, &x});
            break;
        }
        case IR::expType::temp: {
            uses.push_back({{RCT::T, static_cast<IR::Temp*>(x)->tempid}, &x});
            break;
        }
        case IR::expType::name: {
            assert(0);
        }
        default: assert(0);
        }
    };
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movsrc = static_cast<IR::Move*>(stm)->src;
        switch (movsrc->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(movsrc);
            gexp(bop->left);
            gexp(bop->right);
            break;
        }
        case IR::expType::call: {
            auto cal = static_cast<IR::Call*>(movsrc);
            for (auto& it : cal->args) { gexp(it); }
            break;
        }
        default: assert(0);
        }
        break;
    }
    default: assert(0);
    }
    return (uses);
}

class OSR {
public:
    OSR(SSA::SSAIR* t) {
        ir = t;
        cleanNoDefTemp();
        buildSSAGraph();
        // ir->showmark();
        auto ttmp = DFSCFG::Loop_Nesting_Tree(ir);
        lnt = &ttmp;
        nextNum = 0;
        nodesz = tempDefMap.size();
        vector<Temp_Temp> ns;
        for (auto i : tempDefMap) { ns.push_back(i.first); }
        for (auto it : ns)
            if (!vis.count(it)) { DFS(it); }
        lnt = 0;
    }

private:
    void cleanNoDefTemp() {
        unordered_set<Temp_Temp> tmp;
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                auto df = getDef(stm);
                if (df) {
                    auto tid = static_cast<IR::Temp*>(*df)->tempid;
                    tmp.insert(tid);
                }
                stml = stml->tail;
            }
        }
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                auto uses = getUses(stm);
                for (auto it : uses) {
                    if (!tmp.count(static_cast<IR::Temp*>(*it)->tempid)) {
                        delete (*it);
                        *it = new IR::Const(0);
                    }
                }
                stml = stml->tail;
            }
        }
    }
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
                stml = stml->tail;
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
            if (Num[tid] < Num[x] && (ons.count(tid) && ons[tid])) {
                Low[x] = min(Low[x], Num[tid]);
            }
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
    OPR isiv(IR::Exp* exp) {
        if (exp->kind == IR::expType::temp) {
            auto tid = static_cast<IR::Temp*>(exp)->tempid;
            if (!header.count(tid)) return eopr();
            if (header[tid] != -1) { return {RCT::T, tid}; }
        }
        return eopr();
    }
    OPR isrc(Temp_Temp x, int hder) {
        auto db = tempDefBlockMap[x];
        if (lnt->is_StrictlyDominate(db, hder)) { return {RCT::T, x}; }
        return eopr();
    }
    OPR isrc(IR::Exp* x, int hder) {
        if (x->kind == IR::expType::constx) return {RCT::C, static_cast<IR::Const*>(x)->val};
        if (x->kind == IR::expType::temp) return isrc(static_cast<IR::Temp*>(x)->tempid, hder);
        return eopr();
    }
    IR::Exp* rc2exp(OPR x) {
        if (x.kind == RCT::C) return new IR::Const(x.val);
        return new IR::Temp(x.val);
    }
    IVentry IsCandidateOperation(IR::Exp* exp) {
        OPR c1, c2;
        switch (exp->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(exp);
            switch (bop->op) {
            case IR::binop::T_mul:  // fall through
            case IR::binop::T_plus: {
                auto v1 = isiv(bop->left), v2 = isiv(bop->right);
                if (v1.kind != RCT::E) {
                    c2 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->left)->tempid]);
                    if (c2.kind != RCT::E) { return {bop->op, v1, c2}; }
                    return {bop->op, eopr(), eopr()};
                } else if (v2.kind != RCT::E) {
                    c1 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->right)->tempid]);
                    if (c1.kind != RCT::E) { return {bop->op, v2, c1}; }
                    return {bop->op, eopr(), eopr()};
                }
                return {bop->op, eopr(), eopr()};
            }
            case IR::binop::T_minus: {
                auto v1 = isiv(bop->left);

                if (v1.kind != RCT::E) {
                    c2 = isrc(bop->right, header[static_cast<IR::Temp*>(bop->left)->tempid]);
                    if (c2.kind != RCT::E) { return {bop->op, v1, c2}; }
                    return {bop->op, eopr(), eopr()};
                }
                return {bop->op, eopr(), eopr()};
            }
            default: return {bop->op, eopr(), eopr()};
            }
        }
        case IR::expType::temp: {
            auto v1 = isiv(exp);
            if (v1.kind != RCT::E) return {IR::binop::T_plus, v1, {RCT::C, 0}};
            return {IR::binop::T_plus, eopr(), eopr()};
        }
        default: return {IR::binop::T_plus, eopr(), eopr()};
        }
    }
    OPR getRC(IR::Exp* exp) {
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
        case IR::expType::binop: {
            auto bi = static_cast<IR::Binop*>(src);
            if (bi->op == IR::binop::T_plus || bi->op == IR::binop::T_minus) return true;
            return false;
        }
        case IR::expType::call: {
            return isphifunc(stm);
        }
        case IR::expType::temp: {
            return true;
        }
        default: {
            return false;
        }
        }
    }
    void Process(unordered_set<int>& N) {
        if (N.size() == 1) {
            auto nb = *N.begin();
            if (nb == 7839) std::cerr << "nb" << nb << '\n';

            auto src = static_cast<IR::Move*>(tempDefMap[nb]->stm)->src;
            auto tive = IsCandidateOperation(src);
            if (tive.iv.kind != RCT::E && tive.rc.kind != RCT::E) {
                Replace(nb, tive);
            } else
                header[nb] = -1;
        } else
            ClassifyIV(N);
    }
    Temp_Temp Reduce(IVentry tmp) {

        // std::cerr << "REDUCE:" << (int)(tmp.op) << ' ' << tmp.iv << ' ' << (int)(tmp.rc.kind)
        //           << ' ' << tmp.rc.val << '\n';

        // auto df = (static_cast<IR::Move*>(tempDefMap[x]->stm))->src;
        // auto tmp = getIVentry(df);
        if (ivmp.count(tmp)) { return ivmp[tmp]; }

        assert(tmp.iv.kind == RCT::T);

        auto nt = Temp_newtemp();

        ivmp[tmp] = nt;
        IR::Exp* src = (static_cast<IR::Move*>(tempDefMap[tmp.iv.val]->stm)->src);
        auto ivtail = tempDefMap[tmp.iv.val]->tail;
        auto dfnt = new StmList(new IR::Move(new IR::Temp(nt), src->dCopy()), ivtail);
        tempDefMap[nt] = dfnt;
        tempDefBlockMap[nt] = tempDefBlockMap[tmp.iv.val];
        if (isphifunc(dfnt->stm)) { ir->Aphi[tempDefBlockMap[nt]][nt] = dfnt; }
        tempDefMap[tmp.iv.val]->tail = dfnt;
        header[nt] = header[tmp.iv.val];

        auto uses = getOps(dfnt->stm);

        for (auto it : uses) {
            if (it.first.kind == RCT::C) {
                if (tmp.op == IR::binop::T_mul || isphifunc(dfnt->stm)) {
                    auto nx = Apply(tmp.op, it.first, tmp.rc);
                    *(it.second) = new IR::Temp(nx);
                }
            } else if (it.first.kind == RCT::T) {
                auto tid = it.first.val;
                ssaEdge[nt].push_back({tid, *(it.second)});
                if (header.count(tid) && header[tid] == header[tmp.iv.val]) {
                    auto nx = Reduce({tmp.op, {RCT::T, tid}, tmp.rc});
                    static_cast<IR::Temp*>(*(it.second))->tempid = nx;
                } else if (tmp.op == IR::binop::T_mul || isphifunc(dfnt->stm)) {
                    auto nx = Apply(tmp.op, it.first, tmp.rc);
                    static_cast<IR::Temp*>(*(it.second))->tempid = nx;
                }
            } else {
                assert(0);
            }
        }
        return nt;
    }
    Temp_Temp Apply(IR::binop op, OPR op1, OPR op2) {

        if (ivmp.count({op, op1, op2})) { return ivmp[{op, op1, op2}]; }
        int res;
        if (op1.kind == RCT::T && header[op1.val] != -1
            && (op2.kind == RCT::C
                || (op2.kind == RCT::T && isrc(op2.val, header[op1.val]).kind != RCT::E))) {
            res = Reduce({op, op1, op2});
        } else if (op2.kind == RCT::T && header[op2.val] != -1
                   && (op1.kind == RCT::C
                       || (op1.kind == RCT::T && isrc(op1.val, header[op2.val]).kind != RCT::E))) {
            res = Reduce({op, op2, op1});
        } else {
            res = Temp_newtemp();

            ivmp[{op, op1, op2}] = res;
            if (op1.kind == RCT::C && op2.kind == RCT::C) {

                auto stml = (IR::StmList*)(ir->mynodes[0]->info);
                auto ns = new IR::StmList(
                    new IR::Move(new IR::Temp(res), new IR::Binop(op, new IR::Const(op1.val),
                                                                  new IR::Const(op2.val))),
                    stml->tail);
                stml->tail = ns;
                tempDefMap[res] = ns;
                tempDefBlockMap[res] = 0;
            } else {
                int lb = -1;
                if (op1.kind == RCT::T && op2.kind == RCT::T) {
                    int bl1 = tempDefBlockMap[op1.val];
                    int bl2 = tempDefBlockMap[op2.val];
                    if (lnt->Block_Dfsnum(bl1) < lnt->Block_Dfsnum(bl2)) {
                        lb = bl2;
                    } else if (lnt->Block_Dfsnum(bl1) > lnt->Block_Dfsnum(bl2)) {
                        lb = bl1;
                    } else {
                        lb = bl1;
                    }
                } else if (op1.kind == RCT::T) {
                    lb = tempDefBlockMap[op1.val];
                } else {
                    lb = tempDefBlockMap[op2.val];
                }

                // std::cerr << "search in " << lb << '\n';
                // if (op1.kind == RCT::T) std::cerr << "op1" << op1.val << '\n';
                // if (op2.kind == RCT::T) std::cerr << "op2" << op2.val << '\n';
                auto stml = (IR::StmList*)(ir->nodes()->at(lb)->info);
                if (op1.val == 3311) {
                    auto tx = (IR::StmList*)(ir->nodes()->at(lb)->info);
                    while (tx) {
                        tx->stm->printIR();
                        tx = tx->tail;
                    }
                }

                IR::StmList* tg = 0;
                while (stml) {
                    auto df = getDef(stml->stm);
                    if (df) {
                        auto tid = static_cast<IR::Temp*>(*df)->tempid;
                        if (op1.kind == RCT::T && tid == op1.val) tg = stml;
                        if (op2.kind == RCT::T && tid == op2.val) tg = stml;
                    }
                    stml = stml->tail;
                }
                assert(tg);
                auto ns = new IR::StmList(
                    new IR::Move(new IR::Temp(res), new IR::Binop(op, rc2exp(op1), rc2exp(op2))),
                    tg->tail);
                tg->tail = ns;
                tempDefMap[res] = ns;
                tempDefBlockMap[res] = lb;
            }
            header[res] = -1;
        }
        // if (res == 3311) {
        //     auto tx = (IR::StmList*)(ir->nodes()->at(21)->info);
        //     while (tx) {
        //         tx->stm->printIR();
        //         tx = tx->tail;
        //     }
        // }
        return res;
    }
    void Replace(Temp_Temp x, IVentry ive) {

        assert(ive.iv.kind == RCT::T);
        auto nx = Reduce(ive);

        // tempDefMap[x]->stm->printIR();
        // tempDefMap[nx]->stm->printIR();
        for (auto it : uselog[x]) { static_cast<IR::Temp*>(it)->tempid = nx; }
        if (isphifunc(tempDefMap[x]->stm)) { ir->Aphi[tempDefBlockMap[x]].erase(x); }
        tempDefMap[x]->stm = nopStm();
        tempDefBlockMap.erase(x);
        tempDefMap.erase(x);
        header[nx] = header[ive.iv.val];
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
                if (tive.iv.kind == RCT::T) {
                    Replace(it, tive);
                } else
                    header[it] = -1;
            }
        }
    }

public:
    void checkphi() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            int cnt = 0;
            while (stml) {
                auto stm = stml->stm;
                if (isphifunc(stm)) {
                    cnt++;
                    auto df = getDef(stm);
                    auto tid = static_cast<IR::Temp*>(*df)->tempid;
                    assert(ir->Aphi[it->mykey].count(tid));
                }
                stml = stml->tail;
            }
            assert(cnt >= ir->Aphi[it->mykey].size());
        }
    }

private:
    SSA::SSAIR* ir;
    int nextNum, nodesz;
    unordered_set<Temp_Temp> rpovis;
    int rpoNum;
    unordered_map<Temp_Temp, int> rpomp;
    unordered_map<Temp_Temp, int> vis, Num, Low, header, ons;
    vector<Temp_Temp> stk;
    unordered_map<Temp_Temp, IR::StmList*> tempDefMap;
    unordered_map<Temp_Temp, int> tempDefBlockMap;
    unordered_map<Temp_Temp, vector<IR::Exp*>> uselog;
    unordered_map<Temp_Temp, vector<pair<Temp_Temp, IR::Exp*>>> ssaEdge;
    map<IVentry, Temp_Temp> ivmp;
    map<Temp_Temp, IVentry> ivrmp;
    DFSCFG::Loop_Nesting_Tree* lnt;
};
void SSA::Optimizer::strengthReduction() {
    auto tt = OSR(ir);
    // tt.checkphi();
    return;
}
