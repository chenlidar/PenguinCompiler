#include "treeIR.hpp"
T_relOp T_commute(T_relOp op) {  // a op b    ==    b commute(op) a
    switch (op) {
        case T_relOp::T_eq:
            return T_relOp::T_eq;
        case T_relOp::T_ne:
            return T_relOp::T_ne;
        case T_relOp::T_lt:
            return T_relOp::T_gt;
        case T_relOp::T_ge:
            return T_relOp::T_le;
        case T_relOp::T_gt:
            return T_relOp::T_lt;
        case T_relOp::T_le:
            return T_relOp::T_ge;
        case T_relOp::T_ult:
            return T_relOp::T_ugt;
        case T_relOp::T_uge:
            return T_relOp::T_ule;
        case T_relOp::T_ule:
            return T_relOp::T_uge;
        case T_relOp::T_ugt:
            return T_relOp::T_ult;
    }
}
T_relOp T_notRel(T_relOp op) {  // a op b    ==     not(a notRel(op) b)
    switch (op) {
        case T_relOp::T_eq:
            return T_relOp::T_ne;
        case T_relOp::T_ne:
            return T_relOp::T_eq;
        case T_relOp::T_lt:
            return T_relOp::T_ge;
        case T_relOp::T_ge:
            return T_relOp::T_lt;
        case T_relOp::T_gt:
            return T_relOp::T_le;
        case T_relOp::T_le:
            return T_relOp::T_gt;
        case T_relOp::T_ult:
            return T_relOp::T_uge;
        case T_relOp::T_uge:
            return T_relOp::T_ult;
        case T_relOp::T_ule:
            return T_relOp::T_ugt;
        case T_relOp::T_ugt:
            return T_relOp::T_ule;
    }
}