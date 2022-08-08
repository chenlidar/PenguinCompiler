#pragma once
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"
#include <assert.h>

namespace ASM {
typedef Temp_LabelList Targets;
enum class InstrType { oper, move, label };
struct Instr {
    virtual void print(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap = nullptr) = 0;
    InstrType kind;
};
typedef std::vector<Instr*> InstrList;
struct Oper : public Instr {
    std::string assem;
    Temp_TempList dst, src;
    Targets jumps;
    Oper(std::string _assem, Temp_TempList _dst, Temp_TempList _src, Targets _jumps)
        : assem(_assem)
        , dst(_dst)
        , src(_src)
        , jumps(_jumps) {
        this->kind = InstrType::oper;
    }
    void print(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap = nullptr) {
        int i = 0;
        std::string out = assem;
        for (auto d : dst) {
            std::string s = "`d" + std::to_string(i++);
            if (out.find(s) == std::string::npos) break;
            out = out.replace(out.find(s), s.size(), Temp_tempname(tempMap, d));
        }
        i = 0;
        for (auto sr : src) {
            std::string s = "`s" + std::to_string(i++);
            if (out.find(s) == std::string::npos) break;
            out = out.replace(out.find(s), s.size(), Temp_tempname(tempMap, sr));
        }

        if (out.size() > 4 && out.substr(0, 4) == "`mov") {
            int len = out.size(), p = -1;
            for (int i = 0; i < len; i++) {
                if (out[i] == '#') {
                    p = i + 1;
                    break;
                }
            }
            assert(p != -1);
            int x = std::stoi(out.substr(p, len - p));
            std::string s = "`mov";
            if (x <= 256 && x >= 0) {
                out = out.replace(out.find(s), s.size(), "mov");
                std::cout << out << std::endl;
            } else if (x < 0 && x >= -257) {
                out = out.substr(0, p);
                out = out.replace(out.find(s), s.size(), "mvn") + to_string(-x - 1);
                std::cout << out << std::endl;
            } else {
                auto out1 = out.substr(0, p), out2 = out.substr(0, p);
                out1 = out1.replace(out1.find(s), s.size(), "movw") + ":lower16:" + to_string(x);
                std::cout << out1 << std::endl;
                if (!(x >= 0 && x <= 65535)) {
                    out2 = out2.replace(out2.find(s), s.size(), "movt")
                           + ":upper16:" + to_string(x);
                    std::cout << out2 << std::endl;
                }
            }
            return;
        }

        std::cout << out << std::endl;
    }
};
struct Label : public Instr {
    Temp_Label label;
    Label(Temp_Label _label)
        : label(_label) {
        this->kind = InstrType::label;
    }
    void print(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap = nullptr) {
        std::cout << label << ":" << std::endl;
    }
};
struct Move : public Instr {
    std::string assem;
    Temp_TempList dst, src;
    Move(std::string _assem, Temp_TempList _dst, Temp_TempList _src)
        : assem(_assem)
        , dst(_dst)
        , src(_src) {
        assert(_dst.size() == 1 && _src.size() == 1);
        this->kind = InstrType::move;
    }
    void print(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap = nullptr) {
        std::string out = assem;
        out = out.replace(out.find("`d0"), 3, Temp_tempname(tempMap, dst[0]));
        out = out.replace(out.find("`s0"), 3, Temp_tempname(tempMap, src[0]));
        std::cout << out << std::endl;
    }
};
struct Proc {
    InstrList body;
    void print(std::unordered_map<Temp_Temp, Temp_Temp>* tempMap = nullptr) {
        for (auto instr : body) { instr->print(tempMap); }
    }
};
}  // namespace ASM