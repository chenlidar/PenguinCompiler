#include "BuildSSA.hpp"
#include <queue>
#include "../util/utils.hpp"
namespace SSA {
void Loop::findLoop() {
    ir->dtree = new DTREE::Dtree(ir, 0);
    loops.clear();
    std::stack<int> nodestk;
    std::vector<bool> instk = std::vector<bool>(ir->nodecount, false);
    std::vector<int> childcnt = std::vector<int>(ir->nodecount, 0);
    parent = std::vector<int>(ir->nodecount, -1);
    nodestk.push(0);
    instk[0] = true;
    // first : find backward
    while (!nodestk.empty()) {
        int node = nodestk.top();
        if (childcnt[node] == ir->dtree->children[node].size()) {
            for (int succ : ir->mynodes[node]->succs) {  // find backward edge
                if (instk[succ]) {
                    loops[succ].insert(node);
                    loops[succ].insert(succ);
                }
            }
            nodestk.pop();
            instk[node] = false;
        } else {
            int dom = ir->dtree->children[node][childcnt[node]++];
            nodestk.push(dom);
            instk[dom] = true;
        }
    }
    // second : find loop
    for (auto& loop : loops) {
        int head = loop.first;
        std::queue<int> worklist;
        for (auto n : loop.second) {
            if (n == head) continue;
            worklist.push(n);
        }
        while (!worklist.empty()) {
            int node = worklist.front();
            worklist.pop();
            for (auto pred : ir->mynodes[node]->preds) {
                if (loop.second.count(pred)) continue;
                loop.second.insert(pred);
                worklist.push(pred);
            }
        }
    }
    nodestk.push(0);
    std::fill(childcnt.begin(), childcnt.end(), 0);
    // third : build tree
    while (!nodestk.empty()) {
        int node = nodestk.top();
        if (childcnt[node] == ir->dtree->children[node].size()) {
            if (loops.count(node)) {
                for (int lpnode : loops[node]) {  // find children parent
                    if (parent[lpnode] == -1) parent[lpnode] = node;
                }
            }
            nodestk.pop();
        } else {
            int dom = ir->dtree->children[node][childcnt[node]++];
            nodestk.push(dom);
        }
    }
}
Loop::LoopInfo Loop::analyse(int loopHead) {
    LoopInfo info;
    auto loop = loops[loopHead];
    auto nxtnode = -1;
    for (auto n : loops[loopHead]) {
        if (n != loopHead) nxtnode = n;
    }
    if (loop.size() != 2) {  // only unroll one basic block loop
        info.canUnroll = false;
        return info;
    }
    if (ir->mynodes[loopHead]->inDegree() != 2) {  // dont have phi,so dont have induce var
        info.canUnroll = false;
        return info;
    }
    IR::Stm* jumpstm = ir->blockjump[loopHead]->stm;
    assert(jumpstm->kind == IR::stmType::cjump);
    IR::Cjump* cjump = static_cast<IR::Cjump*>(jumpstm);
    if (cjump->op == IR::RelOp::T_eq || cjump->op == IR::RelOp::T_ne) {
        info.canUnroll = false;
        return info;
    }
    // get end and temp
    std::set<int> defset;
    for (auto list = ir->blocklabel[loopHead]; list; list = list->tail) {
        IR::Exp** dstp = getDef(list->stm);
        if (dstp == nullptr) continue;
        defset.insert(static_cast<IR::Temp*>(*dstp)->tempid);
    }
    IR::RelOp rel = cjump->op;
    if (cjump->left->kind == IR::expType::constx && cjump->right->kind == IR::expType::temp) {
        info.end = Uexp(cjump->left);
        info.inducetemp = static_cast<IR::Temp*>(cjump->right)->tempid;
        info.isConst = true;
        rel = notRel(rel);
        cjump->left = new IR::Temp(info.inducetemp);
        cjump->right = info.end.toExp();
        cjump->op = rel;
    } else if (cjump->right->kind == IR::expType::constx
               && cjump->left->kind == IR::expType::temp) {
        info.end = Uexp(cjump->right);
        info.inducetemp = static_cast<IR::Temp*>(cjump->left)->tempid;
        info.isConst = true;
    } else if (cjump->left->kind == IR::expType::temp && cjump->right->kind == IR::expType::temp) {
        int ltmp = static_cast<IR::Temp*>(cjump->left)->tempid;
        int rtmp = static_cast<IR::Temp*>(cjump->right)->tempid;
        if (!defset.count(rtmp)) {
            info.end = Uexp(cjump->right);
            info.inducetemp = ltmp;
        } else if (!defset.count(ltmp)) {
            info.end = Uexp(cjump->left);
            info.inducetemp = rtmp;
            rel = notRel(rel);
            cjump->left = new IR::Temp(rtmp);
            cjump->right = new IR::Temp(ltmp);
            cjump->op = rel;
        } else {
            info.canUnroll = false;
            return info;
        }
        info.isConst = false;
    } else {
        info.canUnroll = false;
        return info;
    }
    if (cjump->trueLabel == static_cast<IR::Label*>(ir->blocklabel[nxtnode]->stm)->label) {
        std::swap(cjump->falseLabel, cjump->trueLabel);
        rel = notRel(rel);
        cjump->op = rel;
    }
    // get start ,step
    auto rtn = findCircle(loopHead, info.inducetemp);
    if (!rtn.first) {
        info.canUnroll = false;
        return info;
    }
    info.begin = rtn.second.first;
    info.step = rtn.second.second;
    if (abs(info.step) > 1e7) {
        info.canUnroll = false;
        return info;
    }
    if (info.begin.kind != IR::expType::constx) info.isConst = false;
    if (!info.isConst) {
        info.canUnroll = true;
        return info;
    }
    assert(info.begin.kind == IR::expType::constx && info.end.kind == IR::expType::constx);
    // cal times.loop until temp relop end
    int bd = info.end.val - info.begin.val;
    switch (rel) {
    case RelOp::T_ge: {
        if (info.begin.val >= info.end.val || info.step <= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = bd / info.step;
        if (info.times * info.step < bd) info.times += 1;
    } break;
    case RelOp::T_gt: {
        if (info.begin.val > info.end.val || info.step <= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = bd / info.step;
        if (info.times * info.step <= bd) info.times += 1;
    } break;
    case RelOp::T_le: {
        if (info.begin.val <= info.end.val || info.step >= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = -bd / -info.step;
        if (info.times * -info.step < -bd) info.times += 1;
    } break;
    case RelOp::T_lt: {
        if (info.begin.val < info.end.val || info.step >= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = -bd / -info.step;
        if (info.times * -info.step <= -bd) info.times += 1;
    } break;
    default: assert(0);
    }
    // get code length
    int cnt = 0;
    IR::StmList* stmlist = (IR::StmList*)ir->mynodes[loopHead]->info;
    for (; stmlist; stmlist = stmlist->tail) cnt++;
    cnt -= 2;
    if (cnt > 5000 || info.times > 5000 || info.times * cnt > 5000) {
        info.canUnroll = false;
        return info;
    }
    info.canUnroll = true;
    return info;
}
bool Loop::loopUnroll() {
    findLoop();
    bool done = false;
    for (auto loop : loops) {
        if (doneloop.count(loop.first)) continue;
        LoopInfo info = analyse(loop.first);
        if (!info.canUnroll) continue;
        done = true;
        int precnt = 0;
        for (auto pred : ir->prednode[loop.first]) {
            if (!loop.second.count(pred)) break;
            precnt++;
        }
        assert(precnt < 2);
        int prenode = ir->prednode[loop.first][precnt];
        int delnode = ir->prednode[loop.first][1 - precnt];
        IR::StmList* stmlist = (IR::StmList*)ir->mynodes[loop.first]->info;
        IR::StmList *head = new IR::StmList(nullptr, nullptr), *tail;
        tail = head;
        if (info.isConst) {
            // std::cerr<<"CONSTLOOP\n";
            // delnode stm to loopnode
            for (auto list = ir->blocklabel[delnode]; list; list = list->tail) {
                if (list->stm->kind == IR::stmType::label || list->stm->kind == IR::stmType::jump)
                    continue;
                tail = tail->tail = new IR::StmList(list->stm->dCopy(), nullptr);
            }
            tail->tail = ir->blockjump[loop.first];
            getLast(stmlist)->tail = head->tail;
            head->tail = nullptr;
            tail = head;
            // unroll
            for (int i = 0; i < info.times - 1; i++) {
                for (auto list = stmlist; list; list = list->tail) {
                    if (list->stm->kind == IR::stmType::label
                        || list->stm->kind == IR::stmType::cjump)
                        continue;
                    tail = tail->tail = new IR::StmList(list->stm->dCopy(), nullptr);
                }
            }
            // remove phi in begin
            int len = 0;
            for (auto list = stmlist; list; list = list->tail) {
                if (list->stm->kind == IR::stmType::label || list->stm->kind == IR::stmType::cjump)
                    continue;
                IR::Stm* stm = list->stm;
                if (isphifunc(stm)) {
                    static_cast<IR::Move*>(stm)->src
                        = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->args[precnt];
                }
                len++;
            }
            // remove phi times-1
            for (auto list = head->tail; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                if (isphifunc(stm)) {
                    static_cast<IR::Move*>(stm)->src
                        = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)
                              ->args[1 - precnt];
                }
            }
            // concat
            tail->tail = ir->blockjump[loop.first];
            getLast(stmlist)->tail = head->tail;
            // rename
            std::map<int, int> tempmap;
            for (auto list = stmlist; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                auto v = getUses(stm);
                for (auto use : v) {
                    int& usetemp = static_cast<IR::Temp*>(*use)->tempid;
                    if (tempmap.count(usetemp)) { usetemp = tempmap[usetemp]; }
                }
                IR::Exp** dstp = getDef(stm);
                if (dstp == nullptr) continue;
                int& dsttemp = static_cast<IR::Temp*>(*dstp)->tempid;
                int nwtemp = Temp_newtemp();
                tempmap[dsttemp] = nwtemp;
                dsttemp = nwtemp;
            }
            // maintain outtemp same
            std::map<int, int> remap;
            for (auto it : tempmap) { remap.insert({it.second, it.first}); }
            for (auto list = stmlist; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                auto v = getUses(stm);
                for (auto use : v) {
                    int& usetemp = static_cast<IR::Temp*>(*use)->tempid;
                    if (remap.count(usetemp)) { usetemp = remap[usetemp]; }
                }
                IR::Exp** dstp = getDef(stm);
                if (dstp == nullptr) continue;
                int& dsttemp = static_cast<IR::Temp*>(*dstp)->tempid;
                if (remap.count(dsttemp)) { dsttemp = remap[dsttemp]; }
            }
            // relink block
            ir->rmEdge(ir->mynodes[delnode], ir->mynodes[loop.first]);
            ir->rmEdge(ir->mynodes[loop.first], ir->mynodes[delnode]);
            ir->prednode[delnode].clear();
            ir->prednode[loop.first].clear();
            ir->prednode[loop.first].push_back(prenode);
            Temp_Label exitlabel
                = static_cast<IR::Label*>(
                      ir->blocklabel[*ir->mynodes[loop.first]->succs.begin()]->stm)
                      ->label;
            ir->blockjump[loop.first]->stm = new IR::Jump(exitlabel);
            ir->Aphi[loop.first].clear();
        } else {
            /**  prenode
             *  ____|____
             * |    1    |________
             * |_________|        |
             *  ____|____         |
             * |    2    |<----   |
             * |(*4times)|____|   |
             *  ____|____         |
             * |    3    |<--------
             * |_________|
             *      |
             *   loophead
             */
            // std::cerr << static_cast<int>(info.begin.kind) << info.begin.val << "!!!\n";
            // std::cerr<<"VARLOOP\n";
            // unlink prenode node
            ir->rmEdge(ir->mynodes[prenode], ir->mynodes[loop.first]);
            // add new node
            Temp_Label label1 = Temp_newlabel();
            Temp_Label label2 = Temp_newlabel();
            Temp_Label label3 = Temp_newlabel();
            Temp_Label looplabel = static_cast<IR::Label*>(ir->blocklabel[loop.first]->stm)->label;
            IR::Cjump* cmp = static_cast<IR::Cjump*>(ir->blockjump[loop.first]->stm);
            int tt = Temp_newtemp();
            IR::StmList* block1 = new IR::StmList(
                new IR::Label(label1),
                new IR::StmList(new IR::Move(new IR::Temp(tt),
                                             new IR::Binop(IR::binop::T_plus, info.end.toExp(),
                                                           new IR::Const(-4 * info.step))),
                                new IR::StmList(new IR::Cjump(cmp->op, info.begin.toExp(),
                                                              new IR::Temp(tt), label3, label2),
                                                nullptr)));
            int block1num = ir->nodecount;
            ir->addNode(block1);
            ir->blocklabel.push_back(block1);
            ir->blockjump.push_back(block1->tail->tail);
            ir->prednode.push_back({prenode});
            ir->orig.push_back(std::set<Temp_Temp>());
            ir->addEdge(ir->mynodes[prenode], ir->mynodes[block1num]);
            tt = Temp_newtemp();
            IR::StmList* block2 = new IR::StmList(
                new IR::Label(label2),
                new IR::StmList(
                    new IR::Move(new IR::Temp(tt),
                                 new IR::Binop(IR::binop::T_plus, info.end.toExp(),
                                               new IR::Const(-4 * info.step))),
                    new IR::StmList(new IR::Cjump(cmp->op, new IR::Temp(info.inducetemp),
                                                  new IR::Temp(tt), label3, label2),
                                    nullptr)));
            int block2num = ir->nodecount;
            ir->addNode(block2);
            ir->blocklabel.push_back(block2);
            ir->blockjump.push_back(block2->tail->tail);
            if (precnt == 0)
                ir->prednode.push_back({block1num, block2num});
            else
                ir->prednode.push_back({block2num, block1num});
            ir->orig.push_back(std::set<Temp_Temp>());
            ir->addEdge(ir->mynodes[block1num], ir->mynodes[block2num]);
            ir->addEdge(ir->mynodes[block2num], ir->mynodes[block2num]);
            IR::StmList* block3 = new IR::StmList(
                new IR::Label(label3), new IR::StmList(new IR::Jump(looplabel), nullptr));
            int block3num = ir->nodecount;
            ir->addNode(block3);
            ir->blocklabel.push_back(block3);
            ir->blockjump.push_back(block3->tail);
            ir->prednode.push_back({block2num, block1num});
            ir->orig.push_back(std::set<Temp_Temp>());
            ir->addEdge(ir->mynodes[block1num], ir->mynodes[block3num]);
            ir->addEdge(ir->mynodes[block2num], ir->mynodes[block3num]);
            ir->addEdge(ir->mynodes[block3num], ir->mynodes[loop.first]);
            ir->prednode[loop.first][precnt] = block3num;
            // unroll 4 times
            for (int i = 0; i < 4; i++) {
                for (auto list = stmlist; list; list = list->tail) {
                    if (list->stm->kind == IR::stmType::label
                        || list->stm->kind == IR::stmType::cjump)
                        continue;
                    IR::Stm* stm = list->stm->dCopy();
                    if (i != 0 && isphifunc(stm)) {
                        static_cast<IR::Move*>(stm)->src
                            = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)
                                  ->args[1 - precnt];
                    }
                    tail = tail->tail = new IR::StmList(stm, nullptr);
                }
                for (auto list = ir->blocklabel[delnode]; list; list = list->tail) {
                    if (list->stm->kind == IR::stmType::label
                        || list->stm->kind == IR::stmType::jump)
                        continue;
                    IR::Stm* stm = list->stm->dCopy();
                    assert(!isphifunc(stm));
                    tail = tail->tail = new IR::StmList(stm, nullptr);
                }
            }
            // concat
            tail->tail = block2->tail;
            block2->tail = head->tail;
            // rename
            std::map<int, int> tempmap;
            for (auto list = block2; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                auto v = getUses(stm);
                for (auto use : v) {
                    int& usetemp = static_cast<IR::Temp*>(*use)->tempid;
                    if (tempmap.count(usetemp)) { usetemp = tempmap[usetemp]; }
                }
                IR::Exp** dstp = getDef(stm);
                if (dstp == nullptr) continue;
                int& dsttemp = static_cast<IR::Temp*>(*dstp)->tempid;
                int nwtemp = Temp_newtemp();
                tempmap[dsttemp] = nwtemp;
                dsttemp = nwtemp;
            }
            // phi rename
            for (auto list = block2; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                if (!isphifunc(stm)) continue;
                auto v = getUses(stm);
                for (auto use : v) {
                    int& usetemp = static_cast<IR::Temp*>(*use)->tempid;
                    if (tempmap.count(usetemp)) { usetemp = tempmap[usetemp]; }
                }
                IR::Exp** dstp = getDef(stm);
                assert(dstp != nullptr);
                int dsttemp = static_cast<IR::Temp*>(*dstp)->tempid;
                ir->Aphi[block2num].insert({dsttemp, list});
            }
            // block3 insert phi
            head->tail = nullptr;
            tail = head;
            for (auto list = stmlist; list; list = list->tail) {
                IR::Stm* stm = list->stm;
                if (!isphifunc(stm)) continue;
                int dsttmp = Temp_newtemp();
                IR::Exp*& exp
                    = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->args[precnt];
                int looptmp = static_cast<IR::Temp*>(
                                  static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)
                                      ->args[1 - precnt])
                                  ->tempid;
                tail = tail->tail = new IR::StmList(
                    new IR::Move(new IR::Temp(dsttmp),
                                 new IR::Call("$", {new IR::Temp(tempmap.at(looptmp)), exp})),
                    nullptr);
                ir->Aphi[block3num].insert({dsttmp, tail});
                exp = new IR::Temp(dsttmp);
            }
            tail->tail = ir->blocklabel[block3num]->tail;
            ir->blocklabel[block3num]->tail = head->tail;
            // cut edge
            ir->cut_edge();
            doneloop.insert(block2num);
            doneloop.insert(loop.first);
        }
    }
    return done;
}
std::pair<bool, std::pair<Uexp, int>> Loop::findCircle(int block, int ctemp) {
    int precnt = 0;
    for (auto pred : ir->prednode[block]) {
        if (!loops[block].count(pred)) break;
        precnt++;
    }
    assert(precnt < 2);
    IR::StmList* stmlist = (IR::StmList*)ir->mynodes[block]->info;
    std::vector<IR::Stm*> v;
    for (; stmlist; stmlist = stmlist->tail) { v.push_back(stmlist->stm); }
    int endtemp = -1, nwtemp = ctemp;
    int step = 0;
    Uexp begin;
    for (int i = v.size() - 1; i >= 0; i--) {
        IR::Stm* stm = v[i];
        if (!isphifunc(stm) && !ismovebi(stm)) continue;
        IR::Exp** dst = getDef(stm);
        assert(dst);
        int dsttemp = static_cast<IR::Temp*>(*dst)->tempid;
        if (dsttemp != nwtemp) continue;
        if (isphifunc(stm)) {
            ExpList& params = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->args;
            if (params[1 - precnt]->kind == IR::expType::temp
                && static_cast<IR::Temp*>(params[1 - precnt])->tempid == ctemp) {
                endtemp = dsttemp;
                begin = Uexp(params[precnt]);
            }
            break;
        } else {
            IR::Binop* biexp = static_cast<IR::Binop*>(static_cast<IR::Move*>(stm)->src);
            if (biexp->op == IR::binop::T_plus) {
                if (biexp->left->kind == IR::expType::temp
                    && biexp->right->kind == IR::expType::constx) {
                    step += static_cast<IR::Const*>(biexp->right)->val;
                    nwtemp = static_cast<IR::Temp*>(biexp->left)->tempid;
                } else if (biexp->right->kind == IR::expType::temp
                           && biexp->left->kind == IR::expType::constx) {
                    step += static_cast<IR::Const*>(biexp->left)->val;
                    nwtemp = static_cast<IR::Temp*>(biexp->right)->tempid;
                } else
                    break;
            } else if (biexp->op == IR::binop::T_minus) {
                if (biexp->left->kind == IR::expType::temp
                    && biexp->right->kind == IR::expType::constx) {
                    step -= static_cast<IR::Const*>(biexp->right)->val;
                    nwtemp = static_cast<IR::Temp*>(biexp->left)->tempid;
                } else
                    break;
            } else
                break;
        }
    }
    return {endtemp != -1, {begin, step}};
}
}  // namespace SSA