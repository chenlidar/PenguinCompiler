#include <stdio.h>
#include "./frontend/parser.hpp"
#include "ast.h"
extern AST::CompUnitList* root;
int main() {
    yyparse();
    return 0;
}