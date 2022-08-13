#include "BuildSSA.hpp"
#include <queue>
namespace SSA {
void Loop::findLoop() {
    ir->dtree=new DTREE::Dtree(ir,0);
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
                if (instk[succ]) loops[succ].insert(node);
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
    for (auto loop : loops) {
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
}  // namespace SSA