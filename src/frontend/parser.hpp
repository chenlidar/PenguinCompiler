/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    RETURN = 258,
    IF = 259,
    ELSE = 260,
    WHILE = 261,
    BREAK = 262,
    CONTINUE = 263,
    VOIDT = 264,
    INTT = 265,
    FLOATT = 266,
    CONST = 267,
    INTNUMBER = 268,
    GETINT = 269,
    GETCH = 270,
    GETFLOAT = 271,
    GETARRAY = 272,
    GETFARRAY = 273,
    PUTINT = 274,
    PUTCH = 275,
    PUTARRAY = 276,
    PUTFLOAT = 277,
    PUTFARRAY = 278,
    PUTF = 279,
    STARTTIME = 280,
    STOPTIME = 281,
    FLOATNUMBER = 282,
    ADDOP = 283,
    SUBOP = 284,
    NOTOP = 285,
    LANDOP = 286,
    LOROP = 287,
    MULOP = 288,
    EQUALOP = 289,
    RELOP = 290,
    ID = 291,
    prec1 = 292,
    prec2 = 293
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 15 "parser.yacc"

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
	AST::IdExp *idexp;
	AST::ExpList *explist;
	AST::Number *number;
	AST::Lval *lval;
	AST::unaryop_t unaryop;

#line 132 "parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
