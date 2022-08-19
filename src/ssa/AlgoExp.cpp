#include "BuildSSA.hpp"
#include "../util/utils.hpp"
#include <queue>
#include <algorithm>
namespace SSA {
std::pair<IR::StmList*, IR::StmList*> Optimizer::handelexp(IR::StmList* begin, IR::StmList* end) {
    IR::StmList *head = nullptr, *tail = nullptr;
    auto emit = [&](IR::Stm* stm) {
        if (head == nullptr) {
            head = tail = new IR::StmList(stm, nullptr);
        } else
            tail = tail->tail = new IR::StmList(stm, nullptr);
    };
    auto t_TT = [&](int dst, IR::binop bop, int t1, int t2) {
        return new IR::Move(new IR::Temp(dst),
                            new IR::Binop(bop, new IR::Temp(t1), new IR::Temp(t2)));
    };
    auto t_CT = [&](int dst, IR::binop bop, int v1, int t2) {
        return new IR::Move(new IR::Temp(dst),
                            new IR::Binop(bop, new IR::Const(v1), new IR::Temp(t2)));
    };
    auto t_NT = [&](int dst, IR::binop bop, std::string n1, int t2) {
        return new IR::Move(new IR::Temp(dst),
                            new IR::Binop(bop, new IR::Name(n1), new IR::Temp(t2)));
    };
    auto t_CN = [&](int dst, IR::binop bop, int v1, std::string n2) {
        return new IR::Move(new IR::Temp(dst),
                            new IR::Binop(bop, new IR::Const(v1), new IR::Name(n2)));
    };
    auto t_NN = [&](int dst, IR::binop bop, std::string n1, std::string n2) {
        return new IR::Move(new IR::Temp(dst),
                            new IR::Binop(bop, new IR::Name(n1), new IR::Name(n2)));
    };
    auto t_T
        = [&](int dst, int src) { return new IR::Move(new IR::Temp(dst), new IR::Temp(src)); };
    auto t_C = [&](int dst, int v) { return new IR::Move(new IR::Temp(dst), new IR::Const(v)); };
    auto t_N
        = [&](int dst, std::string n) { return new IR::Move(new IR::Temp(dst), new IR::Name(n)); };
    std::unordered_set<IR::StmList*> bset;
    for (auto list = begin; list != end; list = list->tail) {
        if (ismovebi(list->stm)
            && (static_cast<IR::Binop*>(static_cast<IR::Move*>(list->stm)->src)->op
                    == IR::binop::T_plus
                || static_cast<IR::Binop*>(static_cast<IR::Move*>(list->stm)->src)->op
                       == IR::binop::T_mul))
            bset.insert(list);
    }
    std::unordered_map<int, IR::Move*> mergemap;
    for (auto list = begin; list != end; list = list->tail) {
        IR::Stm* stm = list->stm;
        if (!ismovebi(stm)
            || !opCommutable(static_cast<IR::Binop*>(static_cast<IR::Move*>(stm)->src)
                                 ->op)) {  // can not use other,can not be used by other
            emit(stm);
            continue;
        }
        IR::Move* mv = static_cast<IR::Move*>(stm);
        IR::Binop* biexp = static_cast<IR::Binop*>(mv->src);
        IR::binop bop = biexp->op;
        int dsttmp = static_cast<IR::Temp*>(mv->dst)->tempid;
        if (usemap.at(dsttmp).size() == 1 && bset.count(usemap.at(dsttmp)[0])
            && static_cast<IR::Binop*>(static_cast<IR::Move*>(usemap.at(dsttmp)[0]->stm)->src)->op
                   == bop) {  // can fall to the use place
            mergemap.insert({dsttmp, mv});
            continue;
        } else {  // a root
            std::vector<int> valv;
            std::map<int, int> tempv;
            std::map<std::string, int> namev;
            // collect leaves
            std::queue<IR::Exp*> q;
            q.push(biexp->left);
            q.push(biexp->right);
            while (!q.empty()) {
                IR::Exp* exp = q.front();
                q.pop();
                switch (exp->kind) {
                case IR::expType::temp: {
                    int bk = static_cast<IR::Temp*>(exp)->tempid;
                    if (mergemap.count(bk)) {
                        IR::Move* nw = mergemap.at(bk);
                        IR::Binop* bp = static_cast<IR::Binop*>(nw->src);
                        q.push(bp->left);
                        q.push(bp->right);
                        mergemap.erase(bk);
                    } else
                        tempv[bk] += 1;
                } break;
                case IR::expType::constx: {
                    valv.push_back(static_cast<IR::Const*>(exp)->val);
                } break;
                case IR::expType::name: {
                    namev[static_cast<IR::Name*>(exp)->name] += 1;
                } break;
                default: assert(0);
                }
            }
            // cal
            int ltemp = -1;
            std::string lname = "";
            // const ford
            if (bop == IR::binop::T_plus) {
                int lnum = 0;
                for (auto it : valv) lnum = lnum + it;
                std::queue<int> tempq;
                // name
                for (auto it : namev) {
                    if (it.second > 1) {
                        int nwtmp = Temp_newtemp();
                        emit(t_CN(nwtmp, IR::binop::T_mul, it.second, it.first));
                        tempq.push(nwtmp);
                    }
                }
                for (auto it : namev) {
                    if (it.second != 1) continue;
                    if (lnum) {
                        int nwtmp = Temp_newtemp();
                        emit(t_CN(nwtmp, bop, lnum, it.first));
                        tempq.push(nwtmp);
                        lnum = 0;
                    } else if (lname == "") {
                        lname = it.first;
                    } else if (lname != "") {
                        int nwtmp = Temp_newtemp();
                        emit(t_NN(nwtmp, bop, lname, it.first));
                        tempq.push(nwtmp);
                        lname = "";
                    } else
                        assert(0);
                }
                while (!tempq.empty()) {
                    int tp = tempq.front();
                    tempq.pop();
                    if (ltemp == -1) {
                        ltemp = tp;
                    } else {
                        int nwtmp = Temp_newtemp();
                        emit(t_TT(nwtmp, bop, ltemp, tp));
                        ltemp = nwtmp;
                    }
                }
                // temp
                for (auto it : tempv) {
                    if (it.second > 1) {
                        int nwtmp = Temp_newtemp();
                        emit(t_CT(nwtmp, IR::binop::T_mul, it.second, it.first));
                        tempq.push(nwtmp);
                    } else
                        tempq.push(it.first);
                }
                while (!tempq.empty()) {
                    int tp = tempq.front();
                    tempq.pop();
                    if (lnum) {
                        ltemp = Temp_newtemp();
                        emit(t_CT(ltemp, bop, lnum, tp));
                        lnum = 0;
                    } else if (lname != "") {
                        ltemp = Temp_newtemp();
                        emit(t_NT(ltemp, bop, lname, tp));
                        lname = "";
                    } else if (ltemp != -1) {
                        int tt = Temp_newtemp();
                        emit(t_TT(tt, bop, ltemp, tp));
                        ltemp = tt;
                    } else if (ltemp == -1) {
                        ltemp = tp;
                    } else
                        assert(0);
                }
                if (ltemp != -1)
                    emit(t_T(dsttmp, ltemp));
                else if (lname != "")
                    emit(t_N(dsttmp, lname));
                else
                    assert(0);
            } else if (bop == IR::binop::T_mul) {
                int lnum = 1;
                for (auto it : valv) lnum = lnum * it;
                if (lnum == 0) {
                    emit(t_C(dsttmp, 0));
                } else {
                    std::queue<int> tempq;
                    // name
                    for (auto it : namev) {
                        assert(it.second == 1);  // address can not mul
                        if (lnum != 1) {
                            ltemp = Temp_newtemp();
                            emit(t_CN(ltemp, bop, lnum, it.first));
                            lnum = 1;
                        } else if (lname != "") {
                            ltemp = Temp_newtemp();
                            emit(t_NN(ltemp, bop, lname, it.first));
                            lname = "";
                        } else if (ltemp != -1) {
                            int tt = Temp_newtemp();
                            emit(t_NT(tt, bop, it.first, ltemp));
                            ltemp = tt;
                        } else {
                            lname = it.first;
                        }
                    }
                    // temp
                    for (auto it : tempv) {
                        int sz = it.second;
                        int btmp, etmp;
                        btmp = -1;
                        etmp = it.first;
                        int nwtmp;
                        while (sz != 0) {
                            if (sz & 1) {
                                nwtmp = Temp_newtemp();
                                if (btmp == -1)
                                    emit(t_T(nwtmp, etmp));
                                else
                                    emit(t_TT(nwtmp, bop, btmp, etmp));
                                btmp = nwtmp;
                            }
                            nwtmp = Temp_newtemp();
                            emit(t_TT(nwtmp, bop, etmp, etmp));
                            etmp = nwtmp;
                            sz >>= 1;
                        }
                        if (lnum != 1) {
                            ltemp = Temp_newtemp();
                            emit(t_CT(ltemp, bop, lnum, btmp));
                            lnum = 1;
                        } else if (lname != "") {
                            ltemp = Temp_newtemp();
                            emit(t_NT(ltemp, bop, lname, btmp));
                            lname = "";
                        } else if (ltemp != -1) {
                            int tt = Temp_newtemp();
                            emit(t_TT(tt, bop, ltemp, btmp));
                            ltemp = tt;
                        } else {
                            ltemp = btmp;
                        }
                    }
                    assert(ltemp != -1);
                    if (ltemp != -1)
                        emit(t_T(dsttmp, ltemp));
                    else if (lname != "")
                        emit(t_N(dsttmp, lname));
                    else
                        assert(0);
                }
            } else if (bop == IR::binop::F_plus) {
                int lnum = digit_i2f(0);
                for (auto it : valv) lnum = encode(decode(lnum) + decode(it));
                std::queue<int> tempq;
                // name
                assert(namev.size()==0);
                // temp
                for (auto it : tempv) {
                    if (it.second > 1) {
                        int nwtmp = Temp_newtemp();
                        emit(t_CT(nwtmp, IR::binop::F_mul, digit_i2f(it.second), it.first));
                        tempq.push(nwtmp);
                    } else
                        tempq.push(it.first);
                }
                while (!tempq.empty()) {
                    int tp = tempq.front();
                    tempq.pop();
                    if (lnum!=digit_i2f(0)) {
                        ltemp = Temp_newtemp();
                        emit(t_CT(ltemp, IR::binop::F_plus, lnum, tp));
                        lnum = 0;
                    } else if (ltemp != -1) {
                        int tt = Temp_newtemp();
                        emit(t_TT(tt, IR::binop::F_plus, ltemp, tp));
                        ltemp = tt;
                    } else if (ltemp == -1) {
                        ltemp = tp;
                    } else
                        assert(0);
                }
                if (ltemp != -1)
                    emit(t_T(dsttmp, ltemp));
                else
                    assert(0);
            } else if (bop == IR::binop::F_mul) {
                int lnum = digit_i2f(1);
                for (auto it : valv) lnum = encode(decode(lnum) * decode(it));
                if (lnum == digit_i2f(0)) {
                    emit(t_C(dsttmp, digit_i2f(0)));
                } else {
                    std::queue<int> tempq;
                    // name
                    assert(namev.size()==0);
                    // temp
                    for (auto it : tempv) {
                        int sz = it.second;
                        int btmp, etmp;
                        btmp = -1;
                        etmp = it.first;
                        int nwtmp;
                        while (sz != 0) {
                            if (sz & 1) {
                                nwtmp = Temp_newtemp();
                                if (btmp == -1)
                                    emit(t_T(nwtmp, etmp));
                                else
                                    emit(t_TT(nwtmp, IR::binop::F_mul, btmp, etmp));
                                btmp = nwtmp;
                            }
                            nwtmp = Temp_newtemp();
                            emit(t_TT(nwtmp, IR::binop::F_mul, etmp, etmp));
                            etmp = nwtmp;
                            sz >>= 1;
                        }
                        if (lnum != digit_i2f(1)) {
                            ltemp = Temp_newtemp();
                            emit(t_CT(ltemp, IR::binop::F_mul, lnum, btmp));
                            lnum = digit_i2f(1);
                        } else if (ltemp != -1) {
                            int tt = Temp_newtemp();
                            emit(t_TT(tt, IR::binop::F_mul, ltemp, btmp));
                            ltemp = tt;
                        } else {
                            ltemp = btmp;
                        }
                    }
                    assert(ltemp != -1);
                    if (ltemp != -1)
                        emit(t_T(dsttmp, ltemp));
                    else
                        assert(0);
                }
            } else
                assert(0);
        }
    }
    assert(mergemap.empty());
    return {head, tail};
}
void Optimizer::combExp() {
    usemap.clear();
    // A. collect info
    for (auto node : ir->mynodes) {
        if (node->inDegree() == 0 && node->mykey != 0) continue;
        IR::StmList* stmlist = (IR::StmList*)node->nodeInfo();
        cleanExpStm(stmlist);
        for (auto list = stmlist; list; list = list->tail) {
            std::vector<IR::Exp**> v = getUses(list->stm);
            for (auto it : v) {
                int usetmp = static_cast<IR::Temp*>(*it)->tempid;
                usemap[usetmp].push_back(list);
            }
        }
    }
    // B. handel each block
    for (auto node : ir->mynodes) {
        if (node->inDegree() == 0 && node->mykey != 0) continue;
        if (node->mykey == ir->exitnum) continue;
        IR::StmList* stmlist = (IR::StmList*)node->nodeInfo();
        IR::StmList *head = ir->blocklabel[node->mykey], *tail = ir->blockjump[node->mykey];
        while (isphifunc(head->tail->stm)) head = head->tail;
        IR::StmList* begin = head->tail;
        if (begin == tail) continue;
        // now head is a phi,begin not phi,tail is jump
        std::pair<IR::StmList*, IR::StmList*> nstml = handelexp(begin, tail);
        head->tail = nstml.first;
        nstml.second->tail = tail;
    }
}
}  // namespace SSA