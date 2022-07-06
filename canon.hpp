#pragma once
#include <string>
#include <vector>
#include "treeIR.hpp"
#include "util/temptemp.hpp"
using namespace IR;
namespace CANON{
StmList* linearize(Stm *stm);

}