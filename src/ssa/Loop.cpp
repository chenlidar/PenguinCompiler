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
    IR::RelOp rel = cjump->op;
    if (cjump->left->kind == IR::expType::constx && cjump->right->kind == IR::expType::temp) {
        info.end = static_cast<IR::Const*>(cjump->left)->val;
        info.inducetemp = static_cast<IR::Temp*>(cjump->right)->tempid;
        rel = notRel(rel);
    } else if (cjump->right->kind == IR::expType::constx
               && cjump->left->kind == IR::expType::temp) {
        info.end = static_cast<IR::Const*>(cjump->right)->val;
        info.inducetemp = static_cast<IR::Temp*>(cjump->left)->tempid;
    } else {
        info.canUnroll = false;
        return info;
    }
    if (cjump->trueLabel == static_cast<IR::Label*>(ir->blocklabel[nxtnode]->stm)->label)
        rel = notRel(rel);
    // get start ,step
    auto rtn = findCircle(loopHead, info.inducetemp);
    if (!rtn.first) {
        info.canUnroll = false;
        return info;
    }
    info.begin = rtn.second.first;
    info.step = rtn.second.second;
    // cal times.loop until temp relop end
    int bd = info.end - info.begin;
    switch (rel) {
    case RelOp::T_ge: {
        if (info.begin >= info.end || info.step <= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = bd / info.step;
        if (info.times * info.step < bd) info.times += 1;
    } break;
    case RelOp::T_gt: {
        if (info.begin > info.end || info.step <= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = bd / info.step;
        if (info.times * info.step <= bd) info.times += 1;
    } break;
    case RelOp::T_le: {
        if (info.begin <= info.end || info.step >= 0) {
            info.canUnroll = false;
            return info;
        }
        info.times = -bd / -info.step;
        if (info.times * -info.step < -bd) info.times += 1;
    } break;
    case RelOp::T_lt: {
        if (info.begin < info.end || info.step >= 0) {
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
    cnt-=2;
    if (cnt > 5000 || info.times > 5000 || info.times * cnt > 5000) {
        info.canUnroll = false;
        return info;
    }
    info.canUnroll = true;
    return info;
}
void Loop::loopUnroll() {
    findLoop();
    for (auto loop : loops) {
        LoopInfo info = analyse(loop.first);
        if (!info.canUnroll) continue;
        // std::cerr << info.times << "\n";
        IR::StmList* stmlist = (IR::StmList*)ir->mynodes[loop.first]->info;
        IR::StmList *head = new IR::StmList(nullptr, nullptr), *tail;
        tail = head;
        for (int i = 0; i < info.times - 1; i++) {
            for (auto list = stmlist; list; list = list->tail) {
                if (list->stm->kind == IR::stmType::label || list->stm->kind == IR::stmType::cjump)
                    continue;
                tail = tail->tail = new IR::StmList(list->stm->dCopy(), nullptr);
            }
        }
        int precnt = 0;
        for (auto pred : ir->prednode[loop.first]) {
            if (!loop.second.count(pred)) break;
            precnt++;
        }
        assert(precnt < 2);
        // remove phi in begin
        int len=0;
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
                    = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->args[1 - precnt];
            }
        }
        // concat
        tail->tail=ir->blockjump[loop.first];
        getLast(stmlist)->tail=head->tail;
        // rename
        std::map<int,int> tempmap;
        for(auto list=stmlist;list;list=list->tail){
            IR::Stm* stm=list->stm;
            auto v=getUses(stm);
            for(auto use:v){
                int& usetemp=static_cast<IR::Temp*>(*use)->tempid;
                if(tempmap.count(usetemp)){
                    usetemp=tempmap[usetemp];
                }
            }
            IR::Exp** dstp=getDef(stm);
            if(dstp==nullptr)continue;
            int& dsttemp=static_cast<IR::Temp*>(*dstp)->tempid;
            int nwtemp=Temp_newtemp();
            tempmap[dsttemp]=nwtemp;
            dsttemp=nwtemp;
        }
        // maintain outtemp same
        std::map<int,int> remap;
        for(auto it:tempmap){
            remap.insert({it.second,it.first});
        }
        for(auto list=stmlist;list;list=list->tail){
            IR::Stm* stm=list->stm;
            auto v=getUses(stm);
            for(auto use:v){
                int& usetemp=static_cast<IR::Temp*>(*use)->tempid;
                if(remap.count(usetemp)){
                    usetemp=remap[usetemp];
                }
            }
            IR::Exp** dstp=getDef(stm);
            if(dstp==nullptr)continue;
            int& dsttemp=static_cast<IR::Temp*>(*dstp)->tempid;
            if(remap.count(dsttemp)){
                dsttemp=remap[dsttemp];
            }
        }
        // relink block
        int delnode=ir->prednode[loop.first][1-precnt];
        int prenode=ir->prednode[loop.first][precnt];
        ir->rmEdge(ir->mynodes[delnode],ir->mynodes[loop.first]);
        ir->rmEdge(ir->mynodes[loop.first],ir->mynodes[delnode]);
        ir->prednode[delnode].clear();
        ir->prednode[loop.first].clear();
        ir->prednode[loop.first].push_back(prenode);
        Temp_Label exitlabel=static_cast<IR::Label*>(ir->blocklabel[*ir->mynodes[loop.first]->succs.begin()]->stm)->label;
        ir->blockjump[loop.first]->stm=new IR::Jump(exitlabel);
        ir->Aphi[loop.first].clear();
    }
}
std::pair<bool, std::pair<int, int>> Loop::findCircle(int block, int ctemp) {
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
    int step = 0, begin = 0;
    for (int i = v.size() - 1; i >= 0; i--) {
        IR::Stm* stm = v[i];
        if (!isphifunc(stm) && !ismovebi(stm)) continue;
        IR::Exp** dst = getDef(stm);
        assert(dst);
        int dsttemp = static_cast<IR::Temp*>(*dst)->tempid;
        if (dsttemp != nwtemp) continue;
        if (isphifunc(stm)) {
            ExpList& params = static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->args;
            if (params[precnt]->kind == IR::expType::constx
                && params[1 - precnt]->kind == IR::expType::temp
                && static_cast<IR::Temp*>(params[1 - precnt])->tempid == ctemp) {
                endtemp = dsttemp;
                begin = static_cast<IR::Const*>(params[precnt])->val;
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