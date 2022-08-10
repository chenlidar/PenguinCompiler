#include "BuildSSA.hpp"
#include "../structure/treeIR.hpp"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <functional>
#include "../util/utils.hpp"
#include "CDG.hpp"
using std::function;
using std::min;
using std::move;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

class OSR {
public:
    OSR(SSA::SSAIR* t) {
        ir = t;
        buildSSAGraph();
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
                if (df) {
                    auto tid = static_cast<IR::Temp*>(*df)->tempid;
                    tempDefMap[tid] = stml;
                    auto uses = getUses(stm);
                    for (auto jt : uses) {
                        ssaEdge[tid].push_back(static_cast<IR::Temp*>(*jt)->tempid);
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
            if (!vis[it]) {
                DFS(it);
                Low[x] = min(Low[x], Low[it]);
            }
            if (Num[it] < Num[x] && ons[it]) { Low[x] = min(Low[x], Num[it]); }
        }
        if (Low[x] == Num[x]) {
            vector<int> SCC;
            int t;
            do {
                t = pop();
                SCC.push_back(t);
            } while (t != x);
            Process(SCC);
        }
    }
    bool isiv(IR::Exp* exp) {
        if (exp->kind == IR::expType::temp) {
            return iv.count(static_cast<IR::Temp*>(exp)->tempid);
        }
        return false;
    }
    bool isrc(IR::Exp* exp) {
        if (exp->kind == IR::expType::temp) {
            return rc.count(static_cast<IR::Temp*>(exp)->tempid);
        }
        if (exp->kind == IR::expType::constx) { return true; }
        return false;
    }
    bool IsCandidateOperation(IR::Exp* exp) {
        switch (exp->kind) {
        case IR::expType::binop: {
            auto bop = static_cast<IR::Binop*>(exp);
            switch (bop->op) {
            case IR::binop::T_mul:  // fall through
            case IR::binop::T_plus: {
                return (isiv(bop->left) && isrc(bop->right))
                       || (isiv(bop->right) && isrc(bop->left));
            }
            case IR::binop::T_minus: {
                return isiv(bop->left) && isrc(bop->right);
            }
            default: return false;
            }
        }
        case IR::expType::temp: {
            return;
        }
        default: return false;
        }
    }
    void Process(vector<int>& N) {
        if (N.size() == 1) {

        } else
            ClassifyIV(N);
    }
    void ClassifyIV(vector<int>& N) {
        bool IsIV = true;
        for (auto it : N) {}
    }

private:
    SSA::SSAIR* ir;
    int nextNum, nodesz;
    unordered_map<Temp_Temp, int> vis, Num, Low;
    vector<Temp_Temp> stk, ons;
    vector<Temp_Temp, IR::StmList*> tempDefMap;
    unordered_map<Temp_Temp, vector<Temp_Temp>> ssaEdge;
    unordered_set<Temp_Temp> iv, rc;
};
void SSA::Optimizer::strengthReduction() {
    auto tt = OSR(ir);
    return;
}
