#include "color.hpp"
#include <assert.h>
#include <stack>
using namespace COLOR;
#define REGNUM 12
COL_result COL_Color(GRAPH::NodeList* ig, std::unordered_map<Temp_Temp, Temp_Temp>* stkuse) {
    struct COL_result cr = COL_result();

    // DONE:
    int empty = 0;
    std::stack<GRAPH::Node*> stk, stk_spill;
    while (!empty) {
        // delete node degree<k
        int change = 1;
        while (change) {
            change = 0;
            auto x = new GRAPH::NodeList();
            for (auto node : *ig) {
                Temp_Temp n = (Temp_Temp)node->nodeInfo();
                if (n >= 100 && node->outDegree() < REGNUM) {
                    node->mygraph->rmNode(node);
                    change = 1;
                    stk.push(node);
                } else {
                    x->push_back(node);
                }
            }
            ig = x;
        }
        // spill
        empty = 1;
        auto x = new GRAPH::NodeList();
        for (auto node : *ig) {
            Temp_Temp n = (Temp_Temp)node->nodeInfo();
            if (n >= 100
                && stkuse->find(n)
                       == stkuse->end()) {  // spill,not spill spilled temp,spilled temp
                // must can be delete at last
                assert(node->outDegree() >= REGNUM);
                node->mygraph->rmNode(node);
                empty = 0;
                stk_spill.push(node);
                break;
            } else {
                x->push_back(node);
            }
        }
        ig = x;
    }
    // precolor
    for (auto node : *ig) {
        Temp_Temp n = (Temp_Temp)node->nodeInfo();
        assert(n < 100);
        cr.coloring->insert(std::make_pair(n, n));
    }
    // color
    int vis[12];
    while (!stk.empty()) {
        GRAPH::Node* node = stk.top();
        stk.pop();
        Temp_Temp n = (Temp_Temp)node->nodeInfo();
        for (int i = 0; i < 12; i++) vis[i] = 0;
        for (auto node1 : node->succ()) {
            Temp_Temp n1 = (Temp_Temp)node1->nodeInfo();
            int num = cr.coloring->at(n1);
            if (num >= 0 && num <= 10)
                vis[num] = 1;
            else if (num == 14)
                vis[11] = 1;
        }
        node->mygraph->reverseNode(node);
        int col = -1;
        if (vis[11] == 0)
            col = 14;
        else
            for (int i = 0; i <= 10; i++) {
                if (vis[i] == 0) {
                    col = i;
                    break;
                }
            }
        if (stk_spill.size() && node == stk_spill.top()) {  // spill
            stk_spill.pop();
            if (col == -1) {  // really spill
                cr.spills->push_back(n);
                cr.coloring->insert(std::make_pair(n, -1));
            } else {
                cr.coloring->insert(std::make_pair(n, col));
            }
        } else {
            assert(col != -1);
            cr.coloring->insert(std::make_pair(n, col));
        }
    }
    return cr;
}
