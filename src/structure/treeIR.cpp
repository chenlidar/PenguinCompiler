#include "treeIR.hpp"
using namespace IR;
RelOp commute(RelOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
        case RelOp::T_eq:
            return RelOp::T_eq;
        case RelOp::T_ne:
            return RelOp::T_ne;
        case RelOp::T_lt:
            return RelOp::T_gt;
        case RelOp::T_ge:
            return RelOp::T_le;
        case RelOp::T_gt:
            return RelOp::T_lt;
        case RelOp::T_le:
            return RelOp::T_ge;
        case RelOp::T_ult:
            return RelOp::T_ugt;
        case RelOp::T_uge:
            return RelOp::T_ule;
        case RelOp::T_ule:
            return RelOp::T_uge;
        case RelOp::T_ugt:
            return RelOp::T_ult;
    }
}
RelOp notRel(RelOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
        case RelOp::T_eq:
            return RelOp::T_ne;
        case RelOp::T_ne:
            return RelOp::T_eq;
        case RelOp::T_lt:
            return RelOp::T_ge;
        case RelOp::T_ge:
            return RelOp::T_lt;
        case RelOp::T_gt:
            return RelOp::T_le;
        case RelOp::T_le:
            return RelOp::T_gt;
        case RelOp::T_ult:
            return RelOp::T_uge;
        case RelOp::T_uge:
            return RelOp::T_ult;
        case RelOp::T_ule:
            return RelOp::T_ugt;
        case RelOp::T_ugt:
            return RelOp::T_ule;
    }
}