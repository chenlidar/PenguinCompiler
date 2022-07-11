#pragma once
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include "../util/templabel.hpp"
#include "../util/temptemp.hpp"

namespace ASM {
typedef Temp_LabelList Targets;
enum class InstrType { oper, move, label };
struct Instr {
    virtual void print() = 0;
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
    void print() {
        int i = 0;
        std::string out = assem;
        for (auto d : dst) {
            std::string s = "`d" + std::to_string(i++);
            out = out.replace(out.find(s), s.size(), Temp_tempname(d));
        }
        i = 0;
        for (auto sr : src) {
            std::string s = "`s" + std::to_string(i++);
            out = out.replace(out.find(s), s.size(), Temp_tempname(sr));
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
    void print() { std::cout << label << ":" << std::endl; }
};
struct Move : public Instr {
    std::string assem;
    Temp_Temp dst, src;
    Move(std::string _assem, Temp_Temp _dst, Temp_Temp _src)
        : assem(_assem)
        , dst(_dst)
        , src(_src) {
        this->kind = InstrType::move;
    }
    void print() {
        std::string out = assem;
        out = out.replace(out.find("`d0"), 3, Temp_tempname(dst));
        out = out.replace(out.find("`s0"), 3, Temp_tempname(src));
        std::cout << out << std::endl;
    }
};
struct Proc {
    InstrList body;
    void print() {
        for (auto instr : body) { instr->print(); }
    }
};
}  // namespace ASM