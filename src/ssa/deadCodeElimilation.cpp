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
bool SSA::Optimizer::isNecessaryStm(IR::Stm* stm) {
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind == IR::expType::mem) return true;
        // FIXME: all funcs have side-effect?
        if (movestm->src->kind == IR::expType::call) {
            auto callexp = static_cast<IR::Call*>(movestm->src);
            if (callexp->fun[0] == '$') return false;
            return true;
        }
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
    case IR::stmType::label: return static_cast<IR::Label*>(stm)->label == this->ir->endlabel;
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
    int oldParentKey;
    CDG::CDgraph gp(ir);

    auto setup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                //  set up stm-block mapping
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
                // set up jump
                if (!stml->tail
                    && (stm->kind == IR::stmType::cjump || stm->kind == IR::stmType::jump)) {
                    blockJumpStm[it->mykey] = stml;
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
                if (!def) continue;
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
            auto df = getDef(stm);
            if (df && ir->Aphi[block->mykey].count(static_cast<IR::Temp*>(*df)->tempid)) {
                for (auto it : *(block->pred())) {
                    auto predlabel = getNodeLabelStm(ir->mynodes[it]);
                    if (!ActivatedStm.count(predlabel)) {
                        ActivatedStm.insert(predlabel);
                        for (auto jt : gp.CDnode[it]) {
                            auto jmp = blockJumpStm[jt]->stm;
                            if (jmp->kind == IR::stmType::cjump) {
                                if (!ActivatedStm.count(jmp)) {
                                    ActivatedBlock.insert(jt);
                                    ActivatedStm.insert(jmp);
                                    Curstm.push(jmp);
                                }
                            }
                        }
                    }
                }
            }
        }
    };
    // FIXME can we remember the block?
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
            if (!ans) oldParentKey = cur->mykey;
            for (GRAPH::NodeList::const_iterator it = succs->begin(); it != succs->end(); ++it) {
                if (!vis.count(ir->mynodes[*it])) {
                    vis.insert(ir->mynodes[*it]);
                    dfs(ir->mynodes[*it]);
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
        auto vis = unordered_set<int>();
        while (!CurNode.empty()) {
            auto cur = CurNode.front();
            CurNode.pop();
            if (vis.count(cur->mykey)) continue;
            vis.insert(cur->mykey);
            auto stml = (IR::StmList*)(cur->info);
            auto head = stml, tail = stml->tail;
            while (tail && tail->tail) {
                if (!ActivatedStm.count(tail->stm)) {
                    if (tail->stm->kind == IR::stmType::move) {
                        auto movestm = static_cast<IR::Move*>(tail->stm);
                        if (movestm->src->kind == IR::expType::call) {
                            auto callexp = static_cast<IR::Call*>(movestm->src);
                            if (callexp->fun[0] == '$') {
                                ir->Aphi[cur->mykey].erase(
                                    static_cast<IR::Temp*>(movestm->dst)->tempid);
                            }
                        }
                    }
                    auto newtail = tail->tail;
                    tail->tail = 0;
                    delete tail;
                    tail = newtail;
                    head->tail = newtail;
                } else {
                    head = head->tail;
                    tail = tail->tail;
                }
            }
            // the last stm
            if (!tail) continue;  // the return block with no jump
            if (ActivatedStm.count(tail->stm)) {  // means that a valid cjump
                auto cjumpstm = static_cast<IR::Cjump*>(tail->stm);
                GRAPH::Node *trueNode = 0, *falseNode = 0;
                int trueoldkey = 0, falseoldkey = 0;
                for (auto& it : (*cur->succ())) {
                    auto nodelabel = getNodeLabel(ir->mynodes[it]);
                    if (nodelabel == cjumpstm->trueLabel) {
                        oldParentKey = cur->mykey;
                        trueNode = FindNextAcitveNode(ir->mynodes[it]);
                        trueoldkey = oldParentKey;
                        // phimap[trueNode->mykey][oldParentKey].push_back(cur->mykey);
                        if (trueoldkey != cur->mykey) {
                            cjumpstm->trueLabel = getNodeLabel(ir->nodes()->at(trueoldkey));
                            ActivatedStm.insert(getNodeLabelStm(ir->nodes()->at(trueoldkey)));
                            newpred[trueoldkey].insert(cur->mykey);
                            newsucc[cur->mykey].insert(trueoldkey);
                        }
                        newpred[trueNode->mykey].insert(trueoldkey);
                        CurNode.push(trueNode);
                        newsucc[trueoldkey].insert(trueNode->mykey);
                    } else if (nodelabel == cjumpstm->falseLabel) {
                        oldParentKey = cur->mykey;
                        falseNode = FindNextAcitveNode(ir->mynodes[it]);
                        falseoldkey = oldParentKey;
                        // phimap[falseNode->mykey][oldParentKey].push_back(cur->mykey);
                        if (falseoldkey != cur->mykey) {
                            cjumpstm->falseLabel = getNodeLabel(ir->nodes()->at(falseoldkey));
                            ActivatedStm.insert(getNodeLabelStm(ir->nodes()->at(falseoldkey)));
                            newpred[falseoldkey].insert(cur->mykey);
                            newsucc[cur->mykey].insert(falseoldkey);
                        }
                        newpred[falseNode->mykey].insert(falseoldkey);
                        CurNode.push(falseNode);
                        newsucc[falseoldkey].insert(falseNode->mykey);
                    }
                }

            } else {  // jump or useless cjump
                int oldkey = 0;
                GRAPH::Node* nextNode = 0;
                for (auto& it : (*cur->succ())) {
                    oldParentKey = cur->mykey;
                    nextNode = FindNextAcitveNode(ir->mynodes[it]);
                    oldkey = oldParentKey;
                    if (nextNode) break;
                }
                // phimap[nextNode->mykey][oldParentKey].push_back(cur->mykey);
                Temp_Label nxlb = getNodeLabel(nextNode);
                if (oldkey != cur->mykey) {
                    nxlb = getNodeLabel(ir->nodes()->at(oldkey));
                    ActivatedStm.insert(getNodeLabelStm((ir->nodes()->at(oldkey))));
                    newpred[oldkey].insert(cur->mykey);
                    newsucc[cur->mykey].insert(oldkey);
                }
                newpred[nextNode->mykey].insert(oldkey);
                CurNode.push(nextNode);
                newsucc[oldkey].insert(nextNode->mykey);
                delete blockJumpStm[cur->mykey]->stm;
                blockJumpStm[cur->mykey]->stm = new IR::Jump(nxlb);
            }
        }
        for (int i = 0; i < nodesz; i++) {
            ir->nodes()->at(i)->preds = move(newpred[i]);
            ir->nodes()->at(i)->succs = move(newsucc[i]);
        }
    };
    auto showmark = [&]() {  // func that can output ssa for debuging
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                if (ActivatedStm.count(stm)) std::cerr << "**";
                stm->printIR();
                stml = stml->tail;
            }
        }
    };
    auto cleanup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            if (it->inDegree() == 1) {
                for (auto jt : ir->Aphi[it->mykey]) {
                    auto mv = static_cast<IR::Move*>(jt.second->stm);
                    auto cl = static_cast<IR::Call*>(mv->src);
                    mv->src = cl->args[0]->quad();
                    // delete cl
                }
                ir->Aphi[it->mykey].clear();
            }
        }
    };
    auto cleanNotActiveNode = [&]() {
        auto nodes = ir->nodes();
        int len = nodes->size();
        for (int i = 0; i < len; i++) {
            if (!ActivatedBlock.count(i)) {
                ir->Aphi[i].clear();
                auto stml = static_cast<IR::StmList*>(nodes->at(i)->info);
                // delete
                stml->tail = blockJumpStm[i];
                ir->prednode[i].clear();
                for (auto j : *(nodes->at(i)->pred())) { ir->prednode[i].push_back(j); }
            }
        }
    };
    // showmark();
    setup();
    bfsMark();
    // showmark();
    elimilation();
    cleanup();
    cleanNotActiveNode();
    // showmark();
}
}  // namespace SSA