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
                                std::unordered_set<Temp_Temp>* stkuse) {
    iList = new ASM::InstrList();
    assert(il);
    for (auto& inst : *il) {
        Temp_TempList *dst = nullptr, *src = nullptr;
        switch (inst->kind) {
        case ASM::InstrType::label: break;
        case ASM::InstrType::move: {
            dst = &static_cast<ASM::Move*>(inst)->dst;
            src = &static_cast<ASM::Move*>(inst)->src;
            if (cr->SpilledTemp.count(dst->at(0)) && cr->SpilledTemp.count(src->at(0))
                && cr->SpilledTemp.at(dst->at(0)) == cr->SpilledTemp.at(src->at(0))) {
                continue;
            }
        } break;
        case ASM::InstrType::oper: {
            dst = &static_cast<ASM::Oper*>(inst)->dst;
            src = &static_cast<ASM::Oper*>(inst)->src;
        } break;
        default: assert(0);
        }
        if (src)
            for (auto& temp : *src) {
                if (cr->SpilledTemp.count(temp)) {
                    if (temp < 0) {
                        int sptmp;
                        if (stkmap->count(cr->SpilledTemp.at(temp)) == 0) {
                            sptmp = Temp_newtemp();
                            stkmap->insert(std::make_pair(cr->SpilledTemp.at(temp), sptmp));
                        } else
                            sptmp = stkmap->at(cr->SpilledTemp.at(temp));
                        Temp_Temp newtemp = ~Temp_newtemp();
                        stkuse->insert(newtemp);
                        temp = newtemp;  // change temp->newtemp
                        emit(new ASM::Oper("vmov `d0,`s0", {newtemp}, {sptmp}, {}));
                    } else {
                        int offset;
                        if (stkmap->count(cr->SpilledTemp.at(temp)) == 0) {
                            stk_size += 4;
                            offset = stk_size;
                            stkmap->insert(std::make_pair(cr->SpilledTemp.at(temp), offset));
                        } else
                            offset = stkmap->at(cr->SpilledTemp.at(temp));
                        Temp_Temp newtemp = Temp_newtemp();
                        stkuse->insert(newtemp);
                        temp = newtemp;  // change temp->newtemp
                        if (offset < 4096 && offset > -4096) {
                            std::string s = "ldr `d0,[fp,#-" + std::to_string(offset) + "]";
                            emit(new ASM::Oper(s, {newtemp}, {}, {}));
                        } else {
                            Temp_Temp constTemp = Temp_newtemp();
                            emit(new ASM::Oper("`mov `d0,#" + std::to_string(offset), {constTemp},
                                               {}, {}));
                            emit(new ASM::Oper("ldr `d0,[`s0,-`s1]", {newtemp}, {11, constTemp},
                                               {}));
                        }
                    }
                }
            }
        emit(inst);
        if (dst)
            for (auto& temp : *dst) {
                if (cr->SpilledTemp.count(temp)) {
                    if (temp < 0) {
                        int sptmp;
                        if (stkmap->count(cr->SpilledTemp.at(temp)) == 0) {
                            sptmp = Temp_newtemp();
                            stkmap->insert(std::make_pair(cr->SpilledTemp.at(temp), sptmp));
                        } else
                            sptmp = stkmap->at(cr->SpilledTemp.at(temp));
                        Temp_Temp newtemp = ~Temp_newtemp();
                        stkuse->insert(newtemp);
                        temp = newtemp;  // change temp->newtemp
                        emit(new ASM::Oper("vmov `d0,`s0", {sptmp}, {newtemp}, {}));
                    } else {
                        int offset;
                        if (stkmap->count(cr->SpilledTemp.at(temp)) == 0) {
                            stk_size += 4;
                            offset = stk_size;
                            stkmap->insert(std::make_pair(cr->SpilledTemp.at(temp), offset));
                        } else
                            offset = stkmap->at(cr->SpilledTemp.at(temp));
                        Temp_Temp newtemp = Temp_newtemp();
                        stkuse->insert(newtemp);
                        temp = newtemp;  // change temp->newtemp
                        if (offset < 4096 && offset > -4096) {
                            std::string s = "str `s0,[fp,#-" + std::to_string(offset) + "]";
                            emit(new ASM::Oper(s, Temp_TempList(), Temp_TempList(1, newtemp),
                                               Temp_LabelList()));
                        } else {
                            Temp_Temp constTemp = Temp_newtemp();
                            emit(new ASM::Oper("`mov `d0,#" + std::to_string(offset),
                                               Temp_TempList(1, constTemp), Temp_TempList(),
                                               Temp_LabelList()));
                            emit(new ASM::Oper("str `s0,[`s1, -`s2]", Temp_TempList(),
                                               {newtemp, 11, constTemp}, Temp_LabelList()));
                        }
                    }
                }
            }
    }
    return iList;
}

static ASM::InstrList* funcEntryExit3(ASM::InstrList* il) {
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
        out->push_back(instrr);
    }
    // exit
    out->push_back(new ASM::Oper("mov sp,fp", Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    out->push_back(new ASM::Oper("ldmdb sp, {r4, r5, r6, r7, r8, r9, r10, fp, lr}",
                                 Temp_TempList(), Temp_TempList(), Temp_LabelList()));
    out->push_back(new ASM::Oper("bx lr", Temp_TempList(), Temp_TempList(), Temp_LabelList()));

    return out;
}
static ASM::InstrList* transAsm(ASM::InstrList* il, TempMap* colormap, bool isf) {
    assert(il);
    ASM::InstrList* out = new ASM::InstrList();
    for (int i = 0; i < il->size(); i++) {
        auto instrr = il->at(i);
        switch (instrr->kind) {
        case ASM::InstrType::move: {
            ASM::Move* instr = static_cast<ASM::Move*>(instrr);
            int idx = 0, cnt = 0;
            while ((idx = instr->assem.find("`d", idx)) != -1) {
                if (isf && instr->dst[cnt] < 0 || !isf && instr->dst[cnt] >= 0)
                    instr->dst[cnt] = colormap->at(instr->dst[cnt]);
                idx++;
                cnt++;
            }
            idx = 0, cnt = 0;
            while ((idx = instr->assem.find("`s", idx)) != -1) {
                if (isf && instr->src[cnt] < 0 || !isf && instr->src[cnt] >= 0)
                    instr->src[cnt] = colormap->at(instr->src[cnt]);
                idx++;
                cnt++;
            }
            if (instr->dst.at(0) == instr->src.at(0)) continue;
        } break;
        case ASM::InstrType::oper: {
            ASM::Oper* instr = static_cast<ASM::Oper*>(instrr);
            int idx = 0, cnt = 0;
            while ((idx = instr->assem.find("`d", idx)) != -1) {
                if (isf && instr->dst[cnt] < 0 || !isf && instr->dst[cnt] >= 0)
                    instr->dst[cnt] = colormap->at(instr->dst[cnt]);
                idx++;
                cnt++;
            }
            idx = 0, cnt = 0;
            while ((idx = instr->assem.find("`s", idx)) != -1) {
                if (isf && instr->src[cnt] < 0 || !isf && instr->src[cnt] >= 0)
                    instr->src[cnt] = colormap->at(instr->src[cnt]);
                idx++;
                cnt++;
            }
        } break;
        case ASM::InstrType::label: break;
        default: assert(0);
        }
        out->push_back(instrr);
    }
    return out;
}
ASM::InstrList* RA::RA_RegAlloc(ASM::InstrList* il, int stksize) {
    stk_size = stksize;
    TempMap* stkmap = new TempMap();
    std::unordered_set<Temp_Temp>* stkuse = new std::unordered_set<Temp_Temp>();
    while (1) {  // first do float regalloc ,since can generate temp when spill
        FLOW::FlowGraph* flowgraph = new FLOW::FlowGraph(il);
        LIVENESS::Liveness* live = new LIVENESS::Liveness(flowgraph);
        IG::ConfGraph* ig = new IG::ConfGraph(live, true);
        COLOR::COL_result* cr = new COLOR::COL_result(ig, stkuse);
        if (!cr->SpilledTemp.empty())
            il = RA_Spill(il, cr, stkmap, stkuse);
        else {
            il = transAsm(il, &cr->ColorMap, true);
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
    while (1) {
        FLOW::FlowGraph* flowgraph = new FLOW::FlowGraph(il);
        LIVENESS::Liveness* live = new LIVENESS::Liveness(flowgraph);
        IG::ConfGraph* ig = new IG::ConfGraph(live, false);
        COLOR::COL_result* cr = new COLOR::COL_result(ig, stkuse);
        if (!cr->SpilledTemp.empty())
            il = RA_Spill(il, cr, stkmap, stkuse);
        else {
            il = transAsm(il, &cr->ColorMap, false);
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
    il = funcEntryExit3(il);  // add stack instr
    for (auto it : *il) { it->print(); }
    return iList;
}
