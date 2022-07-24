#include <assert.h>
#include "BuildSSA.hpp"
#include "../util/utils.hpp"
using namespace SSA;
SSAIR::SSAIR(CANON::Block blocks)
    : CFG::CFGraph(blocks) {
    // A. stmlist -> graph
    // B. Dominator tree
    dtree = new DTREE::Dtree(this);
    // C. Dominance frontiers
    dtree->computeDF();
    // D. insert phi function
    placePhi();
    // E. rename temp, output a graph
    rename();
}
void SSAIR::placePhi() {
    defsites = std::unordered_map<Temp_Temp, std::vector<int>>();
    for (int i = 0; i < nodecount; i++) {
        for (auto dst : orig[i]) { defsites[dst].push_back(i); }
    }
    std::unordered_set<int> worklist;
    for (auto dst : defsites) {
        worklist.clear();
        for (auto node : dst.second) worklist.insert(node);
        while (!worklist.empty()) {
            int node = *worklist.begin();
            worklist.erase(node);
            for (auto y : dtree->DF[node]) {
                if (Aphi[y].count(dst.first)) continue;
                IR::Stm* phifunc
                    = new IR::Move(new IR::Temp(dst.first),
                                   new IR::Call("$", IR::ExpList({})));  // push_back in rename
                IR::StmList* stmlist = (IR::StmList*)this->mynodes[y]->nodeInfo();
                stmlist->tail = new IR::StmList(phifunc, stmlist->tail);
                Aphi[y].insert(std::make_pair(dst.first, phifunc));
                if (!orig[y].count(dst.first)) {
                    worklist.insert(y);
                    orig[y].insert(dst.first);
                }
            }
        }
    }
}
void SSAIR::rename() { rename(0); }
void SSAIR::rename(int node) {
    IR::StmList* stmlist = (IR::StmList*)mynodes[node]->nodeInfo();
    IR::vector<Temp_Temp> rev = IR::vector<Temp_Temp>();
    for (; stmlist; stmlist = stmlist->tail) {
        IR::Stm* stm = stmlist->stm;
        if (stm->kind == IR::stmType::move
            && static_cast<IR::Move*>(stm)->src->kind == IR::expType::call
            && static_cast<IR::Call*>(static_cast<IR::Move*>(stm)->src)->fun[0] == '$')
            ;  // is phi,do nothing
        else {
            // change use
            std::vector<Temp_Temp*> v = getUses(stm);
            for (Temp_Temp* usev : v) {
                if (stk[*usev].empty()) {
                    *usev = -1;  // mark this var not decl, is a const 0
                } else {
                    *usev = stk[*usev].top();
                }
            }
        }
        // change dst
        Temp_Temp* dst = getDef(stm);
        if (dst != nullptr) {
            Temp_Temp temp = Temp_newtemp();
            stk[*dst].push(temp);
            rev.push_back(*dst);
            *dst = temp;
        }
    }
    for (auto succn : *mynodes[node]->succ()) {
        int succ = succn->mykey;
        for (auto it : Aphi[succ]) {
            IR::Move* phimove = static_cast<IR::Move*>(it.second);
            IR::Call* phicall = static_cast<IR::Call*>(phimove->src);
            phicall->args.push_back(new IR::Temp(stk[it.first].top()));
        }
    }
    for (auto succ : dtree->children[node]) rename(succ);
    for (auto var : rev) stk[var].pop();
}

IR::Stm* listTail(IR::StmList* sl){
    assert(sl);
    while(sl->tail!=NULL){
        sl = sl->tail;
    }
    return sl->stm;
}
IR::Stm* listHead(IR::StmList* sl){
    assert(sl);
    return sl->stm;
}

void SSAIR::edge_split(){
    auto Nodes = this->nodes();
    GRAPH::NodeList nl;
    for(auto &u: *Nodes)
        if(u->outDegree() > 1)
            nl.insert(u);

    for(auto &u: nl){
        auto succ = u->succ();
        IR::Stm* laststm = listTail((IR::StmList*)(u->nodeInfo()));
        if(laststm->kind == IR::stmType::jump){
            assert(0);//George think, if end with jump, outdegree cannot be greater than 1
        }
        else if(laststm->kind == IR::stmType::cjump){
            for(auto &v: *succ){
                if(v->inDegree()<=1)continue;
                IR::Stm* firststm = listHead((IR::StmList*)(v->nodeInfo()));
                GRAPH::Node* newnode;
                if(static_cast<IR::Cjump*>(laststm) -> trueLabel ==  static_cast<IR::Label*>(firststm)->label){//truelabel
                    Temp_Label tl = Temp_newlabel(), truelabel = static_cast<IR::Cjump*>(laststm) -> trueLabel;
                    static_cast<IR::Cjump*>(laststm) -> trueLabel = tl;
                    newnode = this->addNode(new IR::StmList(new IR::Label(tl),
                                            new IR::StmList(new IR::Jump(truelabel),NULL)));
                }
                else{//falselabel
                    Temp_Label tl = Temp_newlabel(), falselabel = static_cast<IR::Cjump*>(laststm) -> falseLabel;
                    static_cast<IR::Cjump*>(laststm) -> falseLabel = tl;
                    newnode = this->addNode(new IR::StmList(new IR::Label(tl),
                                            new IR::StmList(new IR::Jump(falselabel),NULL)));
                }
                this->rmEdge(u,v);
                this->addEdge(u,newnode);
                this->addEdge(newnode,v);
            }
        }
        else assert(0);
    }
}