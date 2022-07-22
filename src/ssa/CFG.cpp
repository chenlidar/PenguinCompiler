#include "CFG.hpp"
namespace CFG {
CFGraph::CFGraph(IR::StmList* stmlist) {
    LNTable=std::unordered_map<Temp_Label, GRAPH::Node*>();
    GRAPH::Node* prev = nullptr;
    GRAPH::Node* curr = nullptr;
    GRAPH::NodeList* jumpList = new GRAPH::NodeList();
    for (; stmlist; stmlist = stmlist->tail) {
        IR::Stm* stm = stmlist->stm;
        assert(stm != nullptr);
        curr = this->addNode(stm);
        bool isjump = false;
        switch (stm->kind) {
        case IR::stmType::jump:
        case IR::stmType::cjump: {
            isjump=true;
            jumpList->insert(curr);
        } break;
        case IR::stmType::label: {
            LNTable.insert(std::make_pair(static_cast<IR::Label*>(stm)->label,curr));
        } break;
        case IR::stmType::exp:
        case IR::stmType::move: break;
        default:assert(0);
        }
        if (prev != nullptr) { prev->mygraph->addEdge(prev, curr); }
        prev = isjump ? nullptr:curr;
    }
    for(auto & node:*jumpList){
        IR::Stm* x=(IR::Stm*)(node->nodeInfo());
        if(x->kind==IR::stmType::jump){
            IR::Jump* jmp=static_cast<IR::Jump*>(x);
            assert(jmp->jumps.size()==1);
            Temp_Label label=jmp->jumps.at(0);
            GRAPH::Node* dst=LNTable.at(label);
            node->mygraph->addEdge(node,dst);
        }
        else if(x->kind==IR::stmType::cjump){
            IR::Cjump* cjmp=static_cast<IR::Cjump*>(x);
            Temp_Label tl=cjmp->trueLabel;
            Temp_Label fl=cjmp->falseLabel;
            GRAPH::Node* dst=LNTable.at(tl);
            node->mygraph->addEdge(node,dst);
            dst=LNTable.at(fl);
            node->mygraph->addEdge(node,dst);
        }
        else assert(0);
    }
    delete jumpList;
}
};  // namespace CFG