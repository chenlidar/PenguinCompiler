#include "color.hpp"
#include <assert.h>
using namespace COLOR;
void COL_result::makeWorklist() {
    for (auto& node : ig->mynodes) {
        if (node.isreg()) continue;
        if (node.degree() >= REGNUM) {
            SpillWorklist.insert(node.mykey);
        } else if (!node.mv_adj.empty()) {
            CombineWorklist.insert(node.mykey);
        } else {
            SimplifyWorklist.insert(node.mykey);
        }
    }
}
void COL_result::simplify() {
    int node = *SimplifyWorklist.begin();
    assert(ig->mynodes[node].mv_adj.empty());
    SimplifyWorklist.erase(node);
    SelectStack.push(node);
    while (!ig->mynodes[node].adj.empty()) {
        int adjnode=*ig->mynodes[node].adj.begin();
        ig->p2hEdge(node, adjnode);
        changeWorklist(adjnode);
    }
}
void COL_result::changeWorklist(int node) {
    if (ig->mynodes[node].isreg()) return;
    if (SimplifyWorklist.count(node)) {
        if (ig->mynodes[node].degree() >= REGNUM) {
            SimplifyWorklist.erase(node);
            SpillWorklist.insert(node);
        }
    } else if (CombineWorklist.count(node)) {
        if (ig->mynodes[node].degree() >= REGNUM) {
            CombineWorklist.erase(node);
            SpillWorklist.insert(node);
        } else if (ig->mynodes[node].mv_adj.empty()) {
            CombineWorklist.erase(node);
            SimplifyWorklist.insert(node);
        }
    } else if (FreezeWorklist.count(node)) {
        if (ig->mynodes[node].degree() >= REGNUM) {
            FreezeWorklist.erase(node);
            SpillWorklist.insert(node);
        } else if (ig->mynodes[node].mv_adj.empty()) {
            FreezeWorklist.erase(node);
            SimplifyWorklist.insert(node);
        }
    } else if (SpillWorklist.count(node)) {
        if (ig->mynodes[node].degree() >= REGNUM)
            ;
        else if (ig->mynodes[node].mv_adj.empty()) {
            SpillWorklist.erase(node);
            SimplifyWorklist.insert(node);
        } else if (!ig->mynodes[node].mv_adj.empty()) {
            SpillWorklist.erase(node);
            CombineWorklist.insert(node);
        }
    } else
        assert(0);
}
void COL_result::combine(int u, int v) {  // u may be a precolor, v(from CombineWorklist) will be deleted
    assert(!ig->mynodes[v].isreg());
    for (auto adj : ig->mynodes[v].adj) ig->addEdge(u, adj);
    for (auto h_adj : ig->mynodes[v].h_adj) ig->addEdge_h(u, h_adj);
    for (auto mv_adj : ig->mynodes[v].mv_adj) ig->addEdge_mv(u, mv_adj);
    while (!ig->mynodes[v].adj.empty()) ig->rmEdge(v, *ig->mynodes[v].adj.begin());
    while (!ig->mynodes[v].h_adj.empty()) ig->rmEdge_h(v, *ig->mynodes[v].h_adj.begin());
    while (!ig->mynodes[v].mv_adj.empty()) ig->rmEdge_mv(v, *ig->mynodes[v].mv_adj.begin());
    for (auto tp : ig->mynodes[v].temp) ig->mynodes[u].temp.push_back(tp);
    for (auto tp : ig->mynodes[v].temp) ig->TempNodeMap[tp] = u;
}
bool COL_result::briggs(int u, int v) {
    std::set<int> cnt;
    for (auto node : ig->mynodes[u].adj) {
        if (ig->mynodes[node].degree() >= REGNUM) cnt.insert(node);
    }
    for (auto node : ig->mynodes[v].adj) {
        if (ig->mynodes[node].degree() >= REGNUM) cnt.insert(node);
    }
    return cnt.size() < REGNUM;
}
void COL_result::unionNode() {
    int node = *CombineWorklist.begin();
    CombineWorklist.erase(node);
    std::vector<int> mv_adj;
    for (auto mvadj : ig->mynodes[node].mv_adj) mv_adj.push_back(mvadj);
    bool del=false;
    for (auto adj : mv_adj) {
        assert (adj != node);
        if (ig->mynodes[adj].adj.count(node)) {
            ig->mynodes[node].mv_adj.erase(adj);
            ig->mynodes[adj].mv_adj.erase(node);
            changeWorklist(adj);
        } else if (briggs(node, adj)) {  // adj may be a reg
            combine(adj, node);
            del=true;
            break;
        }
    }
    if(!del)FreezeWorklist.insert(node);
}
void COL_result::freezeMove(int node) {  // must be a no-precolored node
    while (!ig->mynodes[node].mv_adj.empty()) {
        int adjnode=*ig->mynodes[node].mv_adj.begin();
        ig->rmEdge_mv(node,adjnode);
        changeWorklist(adjnode);
    }
}
void COL_result::freeze() {
    int node = *FreezeWorklist.begin();
    assert(!ig->mynodes[node].isreg());
    freezeMove(node);
    FreezeWorklist.erase(node);
    SimplifyWorklist.insert(node);
}
void COL_result::selectSpill() {
    int tnode = -1;
    for (auto node : SpillWorklist) {
        if (stkuse->count(ig->mynodes[node].nodeTemp())) continue;
        assert(!ig->mynodes[node].isreg());
        tnode = node;
        break;
    }
    assert(tnode != -1);
    freezeMove(tnode);
    SpillWorklist.erase(tnode);
    SimplifyWorklist.insert(tnode);
}
void COL_result::assignColor() {
    for(auto precolor:Precolored){
        int node=ig->temp2node(precolor);
        if(node==-1)continue;
        for(auto temp:ig->mynodes[node].temp){
            ColorMap[temp]=precolor;
        }
    }
    while (!SelectStack.empty()) {
        int node = SelectStack.top();
        SelectStack.pop();
        std::set<Temp_Temp> okcolor = Precolored;
        while(!ig->mynodes[node].h_adj.empty())ig->h2pEdge(node,*ig->mynodes[node].h_adj.begin());
        for (auto adjnode : ig->mynodes[node].adj) {
            if (ColorMap.count(ig->mynodes[adjnode].nodeTemp())) {
                okcolor.erase(ColorMap[ig->mynodes[adjnode].nodeTemp()]);
            }
        }
        if (okcolor.empty()) {
            SpilledTemp.insert(ig->mynodes[node].nodeTemp());
        } else {
            int color = *okcolor.begin();
            for (int temp : ig->mynodes[node].temp) ColorMap[temp] = color;
        }
    }
}
void COL_result::COL_Color() {
    makeWorklist();
    int cnt = 0;
    while (!SimplifyWorklist.empty() || !SpillWorklist.empty() || !CombineWorklist.empty()
           || !FreezeWorklist.empty()) {
        if (!SimplifyWorklist.empty())
            simplify();
        else if (!CombineWorklist.empty())
            unionNode();
        else if (!FreezeWorklist.empty())
            freeze();
        else if (!SpillWorklist.empty())
            selectSpill();
    }
    assignColor();
    return;
}
