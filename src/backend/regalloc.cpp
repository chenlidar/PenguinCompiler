#include "regalloc.hpp"
#include <assert.h>
static int stk_size = 0;
static ASM::InstrList* iList = nullptr;
static void emit(ASM::Instr* inst) {
    if (iList == nullptr) { iList = new ASM::InstrList(); }
    iList->push_back(inst);
}
// This is to find the register that's assigned to temp t based on tm
// If return NULL, then it's a Spill
using TempMap = std::unordered_map<Temp_Temp, Temp_Temp>;

static ASM::InstrList* RA_Spill(ASM::InstrList* il, const COLOR::COL_result* cr, TempMap* stkmap,
                                TempMap* stkuse) {
    iList = new ASM::InstrList();
    assert(il);
    for (auto& inst : *il) {
        Temp_TempList *dst = nullptr, *src = nullptr;
        switch (inst->kind) {
        case ASM::InstrType::label: break;
        case ASM::InstrType::move: {
            dst = &static_cast<ASM::Move*>(inst)->dst;
            src = &static_cast<ASM::Move*>(inst)->src;
        } break;
        case ASM::InstrType::oper: {
            dst = &static_cast<ASM::Oper*>(inst)->dst;
            src = &static_cast<ASM::Oper*>(inst)->src;
        } break;
        default: assert(0);
        }
        if (src)
            for (auto& temp : *src) {
                if (cr->SpilledNode.count(temp)) {
                    int offset;
                    if (stkmap->count(temp) == 0) {
                        stk_size += 4;
                        offset = stk_size;
                        stkmap->insert(std::make_pair(temp, offset));
                    } else
                        offset = stkmap->at(temp);
                    Temp_Temp newtemp = Temp_newtemp();
                    stkuse->insert(std::make_pair(newtemp, -2));
                    temp = newtemp;  // change temp->newtemp
                    if (offset < 4096 && offset > -4096) {
                        std::string s = "ldr `d0,[fp,#-" + std::to_string(offset) + "]";
                        emit(new ASM::Oper(s, Temp_TempList(1, newtemp), Temp_TempList(),
                                           Temp_LabelList()));
                    } else {
                        Temp_Temp constTemp = Temp_newtemp();
                        emit(new ASM::Oper("movw `d0,#:lower16:" + std::to_string(offset),
                                           Temp_TempList(1, constTemp), Temp_TempList(),
                                           Temp_LabelList()));
                        if(offset>65535)emit(new ASM::Oper("movt `d0,#:upper16:" + std::to_string(offset),
                                           Temp_TempList(1, constTemp), Temp_TempList(),
                                           Temp_LabelList()));
                        Temp_TempList ll = Temp_TempList(1, 11);
                        ll.push_back(constTemp);
                        emit(new ASM::Oper("sub `d0,`s0,`s1", Temp_TempList(1, constTemp), ll,
                                           Temp_LabelList()));
                        emit(new ASM::Oper("ldr `d0,[`s0]", Temp_TempList(1, newtemp),
                                           Temp_TempList(1, constTemp), Temp_LabelList()));
                    }
                }
            }
        emit(inst);
        if (dst)
            for (auto& temp : *dst) {
                if (cr->SpilledNode.count(temp)) {
                    int offset;
                    if (stkmap->count(temp) == 0) {
                        stk_size += 4;
                        offset = stk_size;
                        stkmap->insert(std::make_pair(temp, offset));
                    } else
                        offset = stkmap->at(temp);
                    Temp_Temp newtemp = Temp_newtemp();
                    stkuse->insert(std::make_pair(newtemp, -2));
                    temp = newtemp;  // change temp->newtemp
                    if (offset < 4096 && offset > -4096) {
                        std::string s = "str `s0,[fp,#-" + std::to_string(offset) + "]";
                        emit(new ASM::Oper(s, Temp_TempList(), Temp_TempList(1, newtemp),
                                           Temp_LabelList()));
                    } else {
                        Temp_Temp constTemp = Temp_newtemp();
                        emit(new ASM::Oper("movw `d0,#:lower16:" + std::to_string(offset),
                                           Temp_TempList(1, constTemp), Temp_TempList(),
                                           Temp_LabelList()));
                        if(offset>65535)emit(new ASM::Oper("movt `d0,#:upper16:" + std::to_string(offset),
                                           Temp_TempList(1, constTemp), Temp_TempList(),
                                           Temp_LabelList()));
                        Temp_TempList ll = Temp_TempList(1, 11);
                        ll.push_back(constTemp);
                        emit(new ASM::Oper("sub `d0,`s0,`s1", Temp_TempList(1, constTemp), ll,
                                           Temp_LabelList()));
                        ll.clear();
                        ll.push_back(newtemp);
                        ll.push_back(constTemp);
                        emit(
                            new ASM::Oper("str `s0,[`s1]", Temp_TempList(), ll, Temp_LabelList()));
                    }
                }
            }
    }
    return iList;
}

static ASM::InstrList* funcEntryExit3(ASM::InstrList* il, TempMap* colormap,
                                      std::set<ASM::Move*>* clearMove) {
    assert(il);
    ASM::InstrList* out = new ASM::InstrList();
    out->push_back(il->at(0));  // label
    // entry
    out->push_back(new ASM::Oper("stmdb sp, {r4, r5, r6, r7, r8, r9, r10, fp, lr}",
                                 Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    out->push_back(new ASM::Oper("mov fp,sp", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    std::string s1 = "sub sp,sp,#" + std::to_string(stk_size & 0xff);
    out->push_back(new ASM::Oper(s1, Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    for (int i = 8; i < 31; i++) {
        if ((stk_size & (1 << i)) == 0) continue;
        std::string s1 = "sub sp,sp,#" + std::to_string(stk_size & (1 << i));
        out->push_back(new ASM::Oper(s1, Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    }

    // body
    for (int i = 1; i < il->size(); i++) {
        auto instrr = il->at(i);
        if (instrr->kind == ASM::InstrType::move) {
            ASM::Move* instr = static_cast<ASM::Move*>(instrr);
            if (clearMove->count(instr)) continue;
        }
        out->push_back(instrr);
    }
    // exit
    out->push_back(new ASM::Oper("mov sp,fp", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    out->push_back(new ASM::Oper("ldmdb sp, {r4, r5, r6, r7, r8, r9, r10, fp, lr}",
                                 Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    out->push_back(new ASM::Oper("bx lr", Temp_TempList(), Temp_TempList(), Temp_LabelList()));

    return out;
}
ASM::InstrList* RA::RA_RegAlloc(ASM::InstrList* il, int stksize) {
    stk_size = stksize;
    TempMap* stkmap = new TempMap();
    TempMap* stkuse = new TempMap();
    while (1) {
        FLOW::FlowGraph* flowgraph = new FLOW::FlowGraph(il);
        LIVENESS::Liveness* live = new LIVENESS::Liveness(flowgraph);
        IG::ConfGraph* ig = new IG::ConfGraph(live);
        COLOR::COL_result* cr = new COLOR::COL_result(ig, stkuse);
        if (!cr->SpilledNode.empty())
            il = RA_Spill(il, cr, stkmap, stkuse);
        else {
            il = funcEntryExit3(il, &cr->ColorMap, &cr->UnionMove);  // add stack instr
            for (auto it : *il) { it->print(&cr->ColorMap); }
            delete flowgraph;
            delete live;
            delete ig;
            delete cr;
            break;
        }
        delete flowgraph;
        delete live;
        delete ig;
        delete cr;
    }
    return iList;
}
