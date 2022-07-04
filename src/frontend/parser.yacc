%{
	#include "../ast.h"

	#include <string>
	AST::CompUnitList *root;

	extern int yylex();
	extern int yyget_lineno();
	extern void yyerror(const char *s);

%}

%union {
	int token;
	std::string *id;
	std::string *floatnumber;
	AST::mul_t mul;
	AST::rel_t rel;
	AST::equal_t equal;
	AST::CompUnitList *root;
	AST::CompUnit *compunit;
	AST::Decl *decl;
	AST::ConstDecl *constdecl;
	AST::btype_t btype;
	AST::ConstDef *constdef;
	AST::ConstDefList *constdeflist;
	AST::ConstExpList *constexplist;
	AST::ConstInitVal *constinitval;
	AST::ConstInitValList *constinitvallist;
	AST::VarDecl *vardecl;
	AST::VarDefList *vardeflist;
	AST::VarDef *vardef;
	AST::ArrayIndex *arrayindex;
	AST::InitVal *initval;
	AST::ArrayInit *arrayinit;
	AST::InitValList *initvallist;
	AST::FuncDef *funcdef;
	AST::Parameters *parameters;
	AST::Parameter *parameter;
	AST::Block *block;
	AST::BlockItemList *blockitemlist;
	AST::BlockItem *blockitem;
	AST::Stmt *stmt;
	AST::Exp *exp;
	AST::ExpList *explist;
	AST::Number *number;
	AST::Lval *lval;
	AST::unaryop_t unaryop;
}

%token<token> RETURN IF ELSE WHILE BREAK CONTINUE
%token<token> VOIDT INTT FLOATT CONST
%token<token> INTNUMBER
%token<floatnumber> FLOATNUMBER
%token<token> ADDOP SUBOP NOTOP LANDOP LOROP
%token<mul> MULOP
%token<equal> EQUALOP
%token<rel> RELOP
%token<id> ID

%type <root> COMPUNITLIST
%type <compunit> COMPUNIT
%type <decl> DECL
%type <constdecl> CONSTDECL
%type <btype> BTYPE
%type <constdef> CONSTDEF
%type <constdeflist> CONSTDEFLIST
%type <constexplist> CONSTEXPLIST
%type <constinitval> CONSTINITVAL
%type <constinitvallist> CONSTINITVALLIST
%type <vardecl> VARDECL
%type <vardeflist> VARDEFLIST
%type <vardef> VARDEF
%type <arrayindex> ARRAYINDEX
%type <initval> INITVAL
%type <arrayinit> ARRAYINIT
%type <initvallist> INITVALLIST
%type <funcdef> FUNCDEF
%type <parameters> PARAMETERS
%type <parameter> PARAMETER
%type <block> BLOCK
%type <blockitemlist> BLOCKITEMLIST
%type <blockitem> BLOCKITEM
%type <stmt> STMT IFSTMT WHILESTMT ASSIGNSTMT EXPSTMT BREAKSTMT CONTINUESTMT RETURNSTMT
%type <exp> EXP PRIMARYEXP UNARYEXP MULEXP ADDEXP RELEXP EQEXP LANDEXP LOREXP CALLEXP
%type <explist> EXPLIST
%type <number> NUMBER
%type <unaryop> UNARYOP
%type <lval> LVAL

%left LOROP
%left LANDOP
%left EQUALOP
%left RELOP
%left ADDOP SUBOP
%left MULOP
%precedence prec1
%precedence prec2
%precedence ELSE

%start COMPUNITLIST

%%

COMPUNITLIST: COMPUNITLIST COMPUNIT {
	/* CompUnitList -> CompUnitList CompUnit */
	$$ = $1;
	$$->list.push_back($2);

} | COMPUNIT {
	/* CompUnitList -> CompUnit */
	$$ = new AST::CompUnitList(yyget_lineno());
	
	root = $$;
}

COMPUNIT: DECL  {
	/* CompUnit -> Decl */
	$$ = static_cast<AST::CompUnit *>($1);
} | FUNCDEF {
	/* CompUint -> FuncDef */
	$$ = static_cast<AST::CompUnit *>($1);
}

DECL: CONSTDECL {
	/* Decl -> ConstDecl */
	$$ = static_cast<AST::Decl *>($1);
} | VARDECL {
	/* Decl -> VarDecl */
	$$ = static_cast<AST::Decl *>($1);
}

CONSTDECL: CONST BTYPE CONSTDEFLIST ';' {
	/* ConstDecl -> const Btype ConstDef (, ConstDef)* ; */
	$$ = new AST::ConstDecl($2, $3, yyget_lineno());
}

CONSTDEFLIST: CONSTDEF {
	$$ = new AST::ConstDefList(yyget_lineno());
} | CONSTDEFLIST ',' CONSTDEF {
	$$ = $1;
	$1->list.push_back($3);
}

BTYPE: INTT {
	$$ = AST::btype_t::INT;
} | FLOATT {
	$$ = AST::btype_t::FLOAT;
} | VOIDT {
	$$ = AST::btype_t::VOID;
}

CONSTDEF: ID CONSTEXPLIST '=' CONSTINITVAL {
	/* ConstDef -> ID ( [Exp] )* = ConstInitVal */
	$$ = new AST::ConstDef($1, $2, $4, yyget_lineno());
}

CONSTEXPLIST: %empty {
	$$ = new AST::ConstExpList(yyget_lineno());
} | CONSTEXPLIST '[' EXP ']' {
	$$ = $1;
	$$->list.push_back($3);
}

CONSTINITVAL: EXP {
	/* ConstInitVal -> Exp */
	$$ = $1;
} | '{' CONSTINITVALLIST '}' {
	/* ConstInitVal -> Array constant */
	$$ = $2;
}

CONSTINITVALLIST: %empty {
	/* Empty */
	$$ = new AST::ConstInitValList(yyget_lineno());
} | CONSTINITVAL {
	$$ = new AST::ConstInitValList(yyget_lineno());
	$$->list.push_back($1);
} | CONSTINITVALLIST ',' CONSTINITVAL {
	$$ = $1;
	$$->list.push_back($3);
}

VARDECL: BTYPE VARDEFLIST ';' {
	$$ = new AST::VarDecl($1, $2, yyget_lineno());
}

VARDEFLIST: VARDEF {
	$$ = new AST::VarDefList(yyget_lineno());
	$$->list.push_back($1);
} | VARDEFLIST ',' VARDEF {
	$$ = $1;
	$$->list.push_back($3);
}

VARDEF: ID ARRAYINDEX {
	$$ = new AST::VarDef($1, $2, nullptr, yyget_lineno());
} | ID ARRAYINDEX '=' INITVAL {
	$$ = new AST::VarDef($1, $2, $4, yyget_lineno());
}

ARRAYINDEX: %empty {
	$$ = new AST::ArrayIndex(yyget_lineno());
} | ARRAYINDEX '[' EXP ']' {
	$$ = $1;
	$$->list.push_back($3);
}

INITVAL: EXP {
	$$ = $1;
} | ARRAYINIT {
	$$ = $1;
}

ARRAYINIT: '{' INITVALLIST '}' {
	$$ = $2;
}

INITVALLIST: INITVAL {
	$$ = new AST::InitValList($1, yyget_lineno());
} | INITVALLIST ',' INITVAL {
	$$ = $1;
	$$->list.push_back($3);
}

FUNCDEF: BTYPE ID '(' PARAMETERS ')' BLOCK {
	$$ = new AST::FuncDef($1, $2, $4, $6, yyget_lineno());
}

PARAMETERS: %empty {
	$$ = new AST::Parameters(yyget_lineno());
} | PARAMETERS ',' PARAMETER {
	$$ = $1;
	$$->list.push_back($3);
}

PARAMETER: BTYPE ID {
	$$ = new AST::Parameter($1, $2, nullptr, yyget_lineno());
} | BTYPE ID '[' ']' ARRAYINDEX {
	$$ = new AST::Parameter($1, $2, $5, yyget_lineno());
}

BLOCK: '{' BLOCKITEMLIST '}' {
	$$ = new AST::Block($2, yyget_lineno());
}

BLOCKITEMLIST: %empty {
	$$ = new AST::BlockItemList(yyget_lineno());
} | BLOCKITEMLIST BLOCKITEM {
	$$ = $1;
	$$->list.push_back($2);
}

BLOCKITEM: DECL {
	$$ = $1;
} | STMT {
	$$ = $1;
}

STMT: ASSIGNSTMT ';' {
	$$ = $1;
} | ';' {
	$$ = nullptr;
} | EXPSTMT ';' {
	$$ = $1;
} | BLOCK {
	$$ = $1;
} | IFSTMT {
	$$ = $1;
} | WHILESTMT {
	$$ = $1;
} | BREAKSTMT ';' {
	$$ = $1;
} | CONTINUESTMT ';' {
	$$ = $1;
} | RETURNSTMT ';' {
	$$ = $1;
}

ASSIGNSTMT: LVAL '=' EXP {
	$$ = new AST::AssignStmt($1, $3, yyget_lineno());
}

EXPSTMT: EXP {
	$$ = new AST::ExpStmt($1, yyget_lineno());
}

IFSTMT: IF '(' EXP ')' STMT %prec prec2 {
	$$ = new AST::IfStmt($3, $5, nullptr, yyget_lineno());
} | IF '(' EXP ')' STMT ELSE STMT {
	$$ = new AST::IfStmt($3, $5, $7, yyget_lineno());
}

WHILESTMT: WHILE '(' EXP ')' STMT {
	$$ = new AST::WhileStmt($3, $5, yyget_lineno());
}

BREAKSTMT: BREAK {
	$$ = new AST::BreakStmt(yyget_lineno());
}

CONTINUESTMT: CONTINUE {
	$$ = new AST::ContinueStmt(yyget_lineno());
}

RETURNSTMT: RETURN {
	$$ = new AST::ReturnStmt(nullptr, yyget_lineno());
} | RETURN EXP {
	$$ = new AST::ReturnStmt($2, yyget_lineno());
}

EXP: PRIMARYEXP {
	$$ = $1;
} | UNARYEXP {
	$$ = $1;
} | MULEXP {
	$$ = $1;
} | ADDEXP {
	$$ = $1;
} | RELEXP {
	$$ = $1;
} | EQEXP {
	$$ = $1;
} | LANDEXP {
	$$ = $1;
} | LOREXP {
	$$ = $1;
} | CALLEXP {
	$$ = $1;
}

LVAL: ID ARRAYINDEX {
	$$ = new AST::Lval($1, $2, yyget_lineno());
}

PRIMARYEXP: '(' EXP ')' {
	$$ = $2;
} | LVAL {
	$$ = $1;
} | NUMBER {
	$$ = $1;
}

NUMBER: INTNUMBER {
	$$ = new AST::IntNumber($1, yyget_lineno());
} | FLOATNUMBER {
	$$ = new AST::FloatNumber($1, yyget_lineno());
}

UNARYEXP: UNARYOP EXP %prec prec1 {
	$$ = new AST::UnaryExp($1, $2, yyget_lineno());
} 

CALLEXP: ID '(' EXPLIST ')' {
	$$ = new AST::CallExp($1, $3, yyget_lineno());
}


UNARYOP: ADDOP {
	$$ = AST::unaryop_t::ADD;
} | SUBOP {
	$$ = AST::unaryop_t::SUB;
} | NOTOP {
	$$ = AST::unaryop_t::NOT;
}

EXPLIST: %empty {
	$$ = new AST::ExpList(yyget_lineno());
} | EXP {
	$$ = new AST::ExpList($1, yyget_lineno());
} | EXPLIST ',' EXP {
	$$ = $1;
	$$->list.push_back($3);
}

MULEXP: EXP MULOP EXP {
	$$ = new AST::MulExp($1, $2, $3, yyget_lineno());
}

ADDEXP: EXP ADDOP EXP {
	$$ = new AST::AddExp($1, AST::addop_t::ADD, $3, yyget_lineno());
} | EXP SUBOP EXP {
	$$ = new AST::AddExp($1, AST::addop_t::SUB, $3, yyget_lineno());
}

RELEXP: EXP RELOP EXP {
	$$ = new AST::RelExp($1, $2, $3, yyget_lineno());
}

EQEXP: EXP EQUALOP EXP {
	$$ = new AST::EqExp($1, $2, $3, yyget_lineno());
}

LANDEXP: EXP LANDOP EXP {
	$$ = new AST::LAndExp($1, $3, yyget_lineno());
}

LOREXP: EXP LOROP EXP {
	$$ = new AST::LOrExp($1, $3, yyget_lineno());
}



%%