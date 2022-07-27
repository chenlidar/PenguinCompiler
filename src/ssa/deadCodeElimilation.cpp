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
bool SSA::Optimizer::isNecessaryStm(IR::Stm* stm) {
    switch (stm->kind) {
    case IR::stmType::move: {
        auto movestm = static_cast<IR::Move*>(stm);
        if (movestm->dst->kind == IR::expType::mem) return true;
        // fixme: all funcs have side-effect?
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
    case IR::stmType::label: {
        // std::cerr << static_cast<IR::Label*>(stm)->label << "^^^^^" << this->ir->endlabel
        //           << std::endl;
        return static_cast<IR::Label*>(stm)->label == this->ir->endlabel;
    }
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
    vector<unordered_map<int, vector<int>>> phimap(nodesz);
    CDG::CDgraph gp(ir);

    auto setup = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {
            auto stml = static_cast<IR::StmList*>(it->info);
            while (stml) {
                auto stm = stml->stm;
                // stm->printIR();
                //  set up stm-block mapping
                stmBlockmap[stm] = it->mykey;

                // set up def-stm mapping
                auto df = getDef(stm);
                if (df) tempDef[static_cast<IR::Temp*>(*df)->tempid] = stm;
                // set up seed stm
                bool necessary = isNecessaryStm(stm);
                if (necessary) {
                    ActivatedStm.insert(stm);
                    // stm->printIR();
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
        }
    };
    int oldParentKey;
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
        auto vis = unordered_set<int>();
        while (!CurNode.empty()) {

            auto cur = CurNode.front();
            CurNode.pop();
            if (vis.count(cur->mykey)) continue;
            vis.insert(cur->mykey);
            // std::cerr << "???\n";
            auto stml = (IR::StmList*)(cur->info);
            auto head = stml, tail = stml->tail;
            while (tail) {
                if (!tail->tail) {  // the last stm
                    if (tail->stm->kind == IR::stmType::cjump) {
                        auto cjumpstm = static_cast<IR::Cjump*>(tail->stm);
                        if (ActivatedStm.count(tail->stm)) {
                            GRAPH::Node *trueNode = 0, *falseNode = 0;
                            int trueoldkey = 0, falseoldkey = 0;
                            for (auto& it : (*cur->succ())) {
                                auto nodelabel = getNodeLabel(it);
                                if (nodelabel == cjumpstm->trueLabel) {
                                    oldParentKey = cur->mykey;
                                    trueNode = FindNextAcitveNode(it);
                                    trueoldkey = oldParentKey;
                                    phimap[trueNode->mykey][oldParentKey].push_back(cur->mykey);
                                    // std::cerr << "truenode: " << trueNode << ' ' << it
                                    //           << std::endl;
                                    cjumpstm->trueLabel = getNodeLabel(trueNode);
                                } else if (nodelabel == cjumpstm->falseLabel) {
                                    oldParentKey = cur->mykey;
                                    falseNode = FindNextAcitveNode(it);
                                    falseoldkey = oldParentKey;
                                    phimap[falseNode->mykey][oldParentKey].push_back(cur->mykey);
                                    cjumpstm->falseLabel = getNodeLabel(falseNode);
                                    // std::cerr << "falsenode: " << falseNode << ' ' << it
                                    //           << std::endl;
                                }
                            }
                            // int len = ir->prednode[trueNode->mykey].size();
                            // for (int i = 0; i < len; i++) {
                            //     if (ir->prednode[trueNode->mykey][i] == trueoldkey) {
                            //         ir->prednode[trueNode->mykey][i] = cur->mykey;
                            //     }
                            // }
                            // len = ir->prednode[falseNode->mykey].size();
                            // for (int i = 0; i < len; i++) {
                            //     if (ir->prednode[falseNode->mykey][i] == falseoldkey) {
                            //         ir->prednode[falseNode->mykey][i] = cur->mykey;
                            //     }
                            // }
                            newsucc[cur->mykey].insert(trueNode);
                            newsucc[cur->mykey].insert(falseNode);
                            assert(trueNode);
                            assert(falseNode);
                            newpred[trueNode->mykey].insert(cur);
                            newpred[falseNode->mykey].insert(cur);
                            CurNode.push(trueNode);
                            CurNode.push(falseNode);
                        } else {
                            int oldkey = 0;
                            GRAPH::Node* nextNode = 0;
                            for (auto& it : (*cur->succ())) {
                                oldParentKey = cur->mykey;
                                nextNode = FindNextAcitveNode(it);
                                oldkey = oldParentKey;
                                if (nextNode) break;
                            }
                            phimap[nextNode->mykey][oldParentKey].push_back(cur->mykey);
                            // int len = ir->prednode[nextNode->mykey].size();
                            // for (int i = 0; i < len; i++) {
                            //     if (ir->prednode[nextNode->mykey][i] == oldkey) {
                            //         ir->prednode[nextNode->mykey][i] = cur->mykey;
                            //     }
                            // }

                            newsucc[cur->mykey].insert(nextNode);
                            newpred[nextNode->mykey].insert(cur);
                            CurNode.push(nextNode);
                            // fixme :delete ,memory leak
                            blockJumpStm[cur->mykey]->stm = new IR::Jump(getNodeLabel(nextNode));
                        }
                    } else if (tail->stm->kind == IR::stmType::jump) {
                        GRAPH::Node* nextNode = 0;
                        int oldkey = 0;
                        for (auto& it : (*cur->succ())) {
                            oldParentKey = cur->mykey;
                            nextNode = FindNextAcitveNode(it);
                            oldkey = oldParentKey;
                            if (nextNode) break;
                        }

                        phimap[nextNode->mykey][oldParentKey].push_back(cur->mykey);
                        // int len = ir->prednode[nextNode->mykey].size();
                        // for (int i = 0; i < len; i++) {
                        //     if (ir->prednode[nextNode->mykey][i] == oldkey) {
                        //         ir->prednode[nextNode->mykey][i] = cur->mykey;
                        //     }
                        // }
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
                    // std::cerr << "DELETE::";
                    // tail->stm->printIR();
                    // auto todelete=tail;
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
                    tail = newtail;
                    head->tail = newtail;
                    // delete todelete
                } else {
                    // std::cerr << "RESERVE::";
                    // tail->stm->printIR();
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
    vector<int> aa, bb;
    for (auto it : (*ir->nodes())) { aa.push_back(it->pred()->size()); }

    setup();
    // int len = 0;
    // for (auto it : ActivatedBlock) { std::cerr << len++ << ':' << it << std::endl; }
    bfsMark();

    auto showmark = [&]() {
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
    auto updatephi = [&]() {
        auto nodes = ir->nodes();
        for (const auto& it : (*nodes)) {

            vector<int> newprednode;
            for (auto jt : (ir->prednode[it->mykey])) {
                for (auto kt : phimap[it->mykey][jt]) { newprednode.push_back(kt); }
            }
            for (auto jt : ir->Aphi[it->mykey]) {
                auto stml = jt.second;
                auto stm = stml->stm;
                auto movestm = static_cast<IR::Move*>(stm);
                auto callexp = static_cast<IR::Call*>(movestm->src);
                vector<IR::Exp*> newargs;
                int len = (ir->prednode[it->mykey]).size();
                auto& v = (ir->prednode[it->mykey]);
                for (int i = 0; i < len; i++) {
                    for (auto kt : phimap[it->mykey][v[i]]) {
                        newargs.push_back(callexp->args[i]);
                    }
                }
                callexp->args = move(newargs);
            }
            ir->prednode[it->mykey] = move(newprednode);
        }
    };
    // std::cerr << "------------------------------------------------\n";
    // showmark();
    // std::cerr << "------------------------------------------------\n";
    elimilation();
    updatephi();
    // for (auto it : (*ir->nodes())) { bb.push_back(it->pred()->size()); }
    // int len = aa.size();
    // for (int i = 0; i < len; i++) {
    //     if (aa[i] != bb[i] && bb[i]) {
    //         std::cerr << "@@@" << getNodeLabel(ir->nodes()->at(i)) << std::endl;
    //     }
    // }
    // showmark();
}

}  // namespace SSA