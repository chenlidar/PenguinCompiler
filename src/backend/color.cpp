#include "color.hpp"
#include <assert.h>
#include <stack>
using namespace COLOR;
#define REGNUM 12
COL_result* COLOR::COL_Color(GRAPH::NodeList* ig,
                             std::unordered_map<Temp_Temp, Temp_Temp>* stkuse) {
    COL_result* cr = new COL_result();

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
                Temp_Temp n = (Temp_Temp)(uint64_t)(uint64_t)(node->nodeInfo());
                if (n >= 100 && node->outDegree() < REGNUM) {
                    node->mygraph->rmNode(node);
                    change = 1;
                    stk.push(node);
                } else {
                    x->insert(node);
                }
            }
            ig = x;
        }
        // spill
        empty = 1;
        auto x = new GRAPH::NodeList();
        bool spilled = false;
        for (auto node : *ig) {
            Temp_Temp n = (Temp_Temp)(uint64_t)node->nodeInfo();
            if (spilled)
                x->insert(node);
            else if (n >= 100
                     && stkuse->find(n)
                            == stkuse->end()) {  // spill,not spill spilled temp,spilled temp
                // must can be delete at last
                assert(node->outDegree() >= REGNUM);
                node->mygraph->rmNode(node);
                empty = 0;
                stk.push(node);
                stk_spill.push(node);
                spilled = true;
            } else {
                x->insert(node);
            }
        }
        ig = x;
    }
    // precolor
    for (auto node : *ig) {
        Temp_Temp n = (Temp_Temp)(uint64_t)node->nodeInfo();
        if(n>=100){
            for(auto it:*node->succ())std::cerr<<(uint64_t)it->nodeInfo()<<std::endl;
        }
        assert(n < 100);
    }
    for (int i = 0; i < 16; i++) { cr->coloring->insert(std::make_pair(i, i)); }
    // color
    int vis[12];

    while (!stk.empty()) {
        GRAPH::Node* node = stk.top();
        stk.pop();
        Temp_Temp n = (Temp_Temp)(uint64_t)node->nodeInfo();
        for (int i = 0; i < 12; i++) vis[i] = 0;
        for (auto& node1 : *node->succ()) {
            Temp_Temp n1 = (Temp_Temp)(uint64_t)node1->nodeInfo();
            int num = cr->coloring->at(n1);
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
                cr->spills->push_back(n);
                cr->coloring->insert(std::make_pair(n, -1));
            } else {
                cr->coloring->insert(std::make_pair(n, col));
            }
        } else {
            assert(col != -1);
            cr->coloring->insert(std::make_pair(n, col));
        }
    }
    return cr;
}
