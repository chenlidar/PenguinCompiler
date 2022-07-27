//#include "optimizer.hpp"
#include "BuildSSA.hpp"
#include "../structure/treeIR.hpp"
#include <vector>
#include <assert.h>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <functional>
#include "../util/utils.hpp"
#include "CDG.hpp"
using std::function;
using std::move;
using std::queue;
using std::unordered_map;
using std::unordered_set;
using std::vector;

namespace SSA {
bool isNecessaryStm(IR::Stm* stm) {
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind == IR::expType::mem) return true;
        // fixme: all funcs have side-effect?
        if (movestm->src->kind == IR::expType::call) return true;
        if (movestm->dst->kind == IR::expType::temp
            && isRealregister(static_cast<IR::Temp*>(movestm->dst)->tempid))
            return true;
        return false;
    }
    case IR::stmType::exp: {
        auto expstm = static_cast<IR::ExpStm*>(stm);
        // fixme: all funcs have side-effect?
        if (expstm->exp->kind == IR::expType::call) return true;
        return false;
    }
    case IR::stmType::cjump: return false;
    case IR::stmType::label: return false;
    case IR::stmType::seq: assert(0);
    case IR::stmType::jump: return false;
    default: assert(0);
    }
    return false;
}

void SSA::Optimizer::deadCodeElimilation() {
    unordered_map<Temp_Temp, IR::Stm*> tempDef;
    unordered_map<IR::Stm*, int> stmBlockmap;
    unordered_set<IR::Stm*> ActivatedStm;
    unordered_set<int> ActivatedBlock;
    unordered_map<int, IR::StmList*> blockJumpStm;
    queue<IR::Stm*> Curstm;
    auto nodesz = ir->nodes()->size();
    vector<GRAPH::NodeList> newpred(nodesz), newsucc(nodesz);

    CDG::CDgraph gp(ir);

    auto setup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                // set up stm-block mapping
                stmBlockmap[stm] = it->mykey;

                // set up def-stm mapping
                auto df = getDef(stm);
                if (df) tempDef[static_cast<IR::Temp*>(*df)->tempid] = stm;
                // set up seed stm
                bool necessary = isNecessaryStm(stm);
                if (necessary) {
                    ActivatedStm.insert(stm);
                    Curstm.push(stm);
                    // set up activated block
                    ActivatedBlock.insert(it->mykey);
                }
                // auto us = getUses(stm);
                // for(auto it:us){
                //     //maybe same stm pushed into info
                //     tempDef[*it].use.push_back(stm);
                // }

                // set up jump
                if (!stml->tail) {
                    if (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump) {
                        blockJumpStm[it->mykey] = stml;
                    }
                }
                stml = stml->tail;
            }
        }
    };
    auto bfsMark = [&]() {
        while (!Curstm.empty()) {
            auto stm = Curstm.front();
            Curstm.pop();
            // upload to the def of uses
            auto uses = getUses(stm);
            for (auto it : uses) {
                auto def = tempDef[static_cast<IR::Temp*>(*it)->tempid];
                if (!ActivatedStm.count(def)) {
                    // set up activated block
                    ActivatedBlock.insert(stmBlockmap[def]);
                    ActivatedStm.insert(def);
                    Curstm.push(def);
                }
            }
            // upload to branch stm
            auto block = ir->nodes()->at(stmBlockmap[stm]);
            auto labelstm = ((IR::StmList*)(block->info))->stm;
            if (!ActivatedStm.count(labelstm)) {
                ActivatedStm.insert(labelstm);
                for (auto it : gp.CDnode[block->mykey]) {
                    auto jmp = blockJumpStm[it]->stm;
                    if (jmp->kind == IR::stmType::cjump) {
                        if (!ActivatedStm.count(jmp)) {
                            ActivatedBlock.insert(it);
                            ActivatedStm.insert(jmp);
                            Curstm.push(jmp);
                        }
                    }
                }
            }
        }
    };
    auto FindNextAcitveNode = [&](GRAPH::Node* node) {
        if (ActivatedBlock.count(node->mykey)) return node;
        unordered_set<GRAPH::Node*> vis;
        vis.insert(node);
        GRAPH::Node* ans = 0;
        function<void(GRAPH::Node*)> dfs = [&](GRAPH::Node* cur) {
            if (ans) return;
            if (ActivatedBlock.count(cur->mykey)) {
                ans = cur;
                return;
            }
            GRAPH::NodeList* succs = cur->succ();
            for (GRAPH::NodeList::const_iterator it = succs->begin(); it != succs->end(); ++it) {
                if (!vis.count(*it)) {
                    vis.insert(*it);
                    dfs(*it);
                }
            }
        };
        dfs(node);
        return ans;
    };
    auto elimilation = [&]() {
        queue<GRAPH::Node*> CurNode;
        auto nodes = ir->nodes();
        CurNode.push((nodes->at(0)));  // push the first node
        while (!CurNode.empty()) {
            auto cur = CurNode.front();
            CurNode.pop();
            auto stml = (IR::StmList*)(cur->info);
            auto head = stml, tail = stml->tail;
            while (tail) {
                if (!tail->tail) {  // the last stm
                    if (tail->stm->kind == IR::stmType::cjump) {
                        auto cjumpstm = static_cast<IR::Cjump*>(tail->stm);
                        if (ActivatedStm.count(tail->stm)) {
                            GRAPH::Node *trueNode = 0, *falseNode = 0;
                            for (auto& it : (*cur->succ())) {
                                auto nodelabel = getNodeLabel(it);
                                if (nodelabel == cjumpstm->trueLabel) {
                                    trueNode = FindNextAcitveNode(it);
                                    cjumpstm->trueLabel = getNodeLabel(trueNode);
                                } else if (nodelabel == cjumpstm->falseLabel) {
                                    falseNode = FindNextAcitveNode(it);
                                    cjumpstm->falseLabel = getNodeLabel(falseNode);
                                }
                            }
                            newsucc[cur->mykey].insert(trueNode);
                            newsucc[cur->mykey].insert(falseNode);
                            newpred[trueNode->mykey].insert(cur);
                            newpred[falseNode->mykey].insert(cur);
                            CurNode.push(trueNode);
                            CurNode.push(falseNode);
                        } else {
                            GRAPH::Node* nextNode = 0;
                            for (auto& it : (*cur->succ())) {
                                nextNode = FindNextAcitveNode(it);
                                if (nextNode) break;
                            }
                            newsucc[cur->mykey].insert(nextNode);
                            newpred[nextNode->mykey].insert(cur);
                            CurNode.push(nextNode);
                            // fixme :delete ,memory leak
                            blockJumpStm[cur->mykey]->stm = new IR::Jump(getNodeLabel(nextNode));
                        }
                    } else if (tail->stm->kind == IR::stmType::jump) {
                        GRAPH::Node* nextNode = 0;
                        for (auto& it : (*cur->succ())) {
                            nextNode = FindNextAcitveNode(it);
                            if (nextNode) break;
                        }
                        newsucc[cur->mykey].insert(nextNode);
                        newpred[nextNode->mykey].insert(cur);
                        CurNode.push(nextNode);
                        // fixme :delete ,memory leak
                        blockJumpStm[cur->mykey]->stm = new IR::Jump(getNodeLabel(nextNode));
                    } else {
                        // return no jump TODO
                    }
                    break;
                }
                if (!ActivatedStm.count(tail->stm)) {
                    // auto todelete=tail;
                    auto newtail = tail->tail;
                    tail->tail = 0;
                    tail = newtail;
                    head->tail = newtail;
                    // delete todelete
                } else {
                    head = head->tail;
                    tail = tail->tail;
                }
            }
        }
        for (int i = 0; i < nodesz; i++) {
            ir->nodes()->at(i)->preds = move(newpred[i]);
            ir->nodes()->at(i)->succs = move(newsucc[i]);
        }
    };
    setup();
    bfsMark();
    elimilation();
}

}  // namespace SSA