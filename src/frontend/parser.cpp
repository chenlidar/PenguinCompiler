/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.yacc"

	#include "../structure/ast.h"

	#include <string>
	#include <cstdio>
	#include <iostream>
	AST::CompUnitList *root;

	extern int yylex();
	extern int yyget_lineno();
	void yyerror(const char *s) { std::cerr << s << std::endl;}


#line 84 "parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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
/* Tokens.  */
#define RETURN 258
#define IF 259
#define ELSE 260
#define WHILE 261
#define BREAK 262
#define CONTINUE 263
#define VOIDT 264
#define INTT 265
#define FLOATT 266
#define CONST 267
#define INTNUMBER 268
#define GETINT 269
#define GETCH 270
#define GETFLOAT 271
#define GETARRAY 272
#define GETFARRAY 273
#define PUTINT 274
#define PUTCH 275
#define PUTARRAY 276
#define PUTFLOAT 277
#define PUTFARRAY 278
#define PUTF 279
#define STARTTIME 280
#define STOPTIME 281
#define FLOATNUMBER 282
#define ADDOP 283
#define SUBOP 284
#define NOTOP 285
#define LANDOP 286
#define LOROP 287
#define MULOP 288
#define EQUALOP 289
#define RELOP 290
#define ID 291
#define prec1 292
#define prec2 293

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

#line 248 "parser.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   357

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  48
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  57
/* YYNRULES -- Number of rules.  */
#define YYNRULES  113
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  214

#define YYUNDEFTOK  2
#define YYMAXUTOK   293


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      46,    47,     2,     2,    40,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    39,
       2,    41,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    42,     2,    43,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    44,     2,    45,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   109,   109,   116,   124,   127,   132,   135,   140,   145,
     148,   153,   155,   157,   161,   166,   170,   173,   178,   180,
     184,   186,   191,   193,   197,   201,   203,   205,   210,   214,
     216,   218,   223,   225,   234,   238,   240,   245,   247,   251,
     253,   255,   257,   259,   261,   263,   265,   267,   269,   271,
     273,   275,   277,   279,   281,   283,   287,   291,   295,   297,
     301,   305,   309,   313,   315,   319,   323,   327,   331,   335,
     339,   343,   347,   351,   353,   355,   357,   359,   361,   363,
     365,   367,   369,   371,   373,   375,   377,   381,   385,   389,
     391,   393,   397,   399,   403,   407,   412,   414,   416,   420,
     422,   424,   429,   433,   435,   439,   443,   447,   451,   455,
     459,   463,   467,   471
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "RETURN", "IF", "ELSE", "WHILE", "BREAK",
  "CONTINUE", "VOIDT", "INTT", "FLOATT", "CONST", "INTNUMBER", "GETINT",
  "GETCH", "GETFLOAT", "GETARRAY", "GETFARRAY", "PUTINT", "PUTCH",
  "PUTARRAY", "PUTFLOAT", "PUTFARRAY", "PUTF", "STARTTIME", "STOPTIME",
  "FLOATNUMBER", "ADDOP", "SUBOP", "NOTOP", "LANDOP", "LOROP", "MULOP",
  "EQUALOP", "RELOP", "ID", "prec1", "prec2", "';'", "','", "'='", "'['",
  "']'", "'{'", "'}'", "'('", "')'", "$accept", "COMPUNITLIST", "COMPUNIT",
  "DECL", "CONSTDECL", "CONSTDEFLIST", "BTYPE", "CONSTDEF", "VARDECL",
  "VARDEFLIST", "VARDEF", "ARRAYINDEX", "INITVAL", "ARRAYINIT",
  "INITVALLIST", "FUNCDEF", "PARAMETERS", "PARAMETER", "BLOCK",
  "BLOCKITEMLIST", "BLOCKITEM", "STMT", "ASSIGNSTMT", "EXPSTMT", "IFSTMT",
  "WHILESTMT", "BREAKSTMT", "CONTINUESTMT", "RETURNSTMT", "PUTINTSTMT",
  "PUTCHSTMT", "PUTARRAYSTMT", "PUTFLOATSTMT", "PUTFARRAYSTMT", "PUTFSTMT",
  "STARTTIMESTMT", "STOPTIMESTMT", "EXP", "IDEXP", "LVAL", "PRIMARYEXP",
  "NUMBER", "UNARYEXP", "CALLEXP", "UNARYOP", "EXPLIST", "MULEXP",
  "ADDEXP", "RELEXP", "EQEXP", "LANDEXP", "LOREXP", "GETINTEXP",
  "GETCHEXP", "GETFLOATEXP", "GETARRAYEXP", "GETFARRAYEXP", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,    59,
      44,    61,    91,    93,   123,   125,    40,    41
};
# endif

#define YYPACT_NINF (-192)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      24,  -192,  -192,  -192,    76,    83,  -192,  -192,  -192,   -24,
    -192,  -192,   -24,  -192,  -192,  -192,   -14,  -192,   -41,    22,
    -192,  -192,  -192,   -24,    76,    30,  -192,   -24,    60,  -192,
    -192,   -24,   -31,  -192,    61,   124,  -192,    61,   -27,    76,
     -15,  -192,    -5,     4,     6,    34,    50,  -192,  -192,  -192,
    -192,    61,   124,  -192,  -192,   314,    66,  -192,  -192,  -192,
    -192,  -192,   124,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,   291,  -192,   -23,  -192,  -192,  -192,
     -16,     0,    68,   124,   124,  -192,   -34,    35,   124,   124,
     124,   124,   124,   124,   124,   124,    84,  -192,  -192,  -192,
     170,  -192,  -192,  -192,    88,   223,    61,  -192,  -192,    92,
      92,   322,    75,  -192,   133,    85,   314,   -26,    84,   124,
      97,    98,  -192,  -192,    99,   100,   101,   102,   103,   109,
     110,   111,  -192,  -192,  -192,   -24,  -192,  -192,  -192,   119,
     120,  -192,  -192,   126,   132,   162,   163,   164,   165,   166,
     174,   196,   197,   198,   314,   131,  -192,  -192,  -192,   124,
    -192,   314,   124,   124,   124,   124,   124,   124,   124,   124,
     191,   192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,   124,   314,   231,   240,   248,
     257,    11,   265,   304,   -17,  -192,  -192,   314,   204,   204,
    -192,  -192,   124,  -192,   124,  -192,   236,  -192,   274,   282,
     204,  -192,  -192,  -192
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,    13,    11,    12,     0,     0,     3,     4,     6,     0,
       7,     5,     0,     1,     2,    87,     0,    16,    20,     0,
       9,    20,    15,     0,    29,    18,     8,     0,     0,    17,
      20,     0,     0,    30,     0,     0,    10,     0,    32,     0,
       0,    92,     0,     0,     0,     0,     0,    93,    96,    97,
      98,    25,     0,    19,    23,    22,    20,    90,    73,    91,
      74,    81,     0,    75,    76,    77,    78,    79,    80,    82,
      83,    84,    85,    86,     0,    14,     0,    31,    35,    28,
       0,     0,     0,     0,     0,    26,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    99,    88,    94,    21,    20,
       0,   109,   110,   111,     0,     0,     0,    24,    89,   103,
     104,   107,   108,   102,   106,   105,   100,     0,    33,    63,
       0,     0,    61,    62,     0,     0,     0,     0,     0,     0,
       0,     0,    40,    34,    37,     0,    42,    36,    38,     0,
       0,    43,    44,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    57,    90,   112,   113,    27,     0,
      95,    64,     0,     0,     0,     0,     0,     0,     0,    99,
       0,     0,    39,    41,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,     0,   101,     0,     0,     0,
       0,     0,     0,     0,     0,    71,    72,    56,     0,     0,
      65,    66,     0,    68,     0,    70,    58,    60,     0,     0,
       0,    67,    69,    59
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -192,  -192,   237,   144,  -192,  -192,    -2,   218,  -192,  -192,
     224,   -18,   -33,  -192,  -192,  -192,  -192,   207,   209,  -192,
    -192,  -191,  -192,  -192,  -192,  -192,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192,   -35,     1,   -99,
    -192,  -192,  -192,  -192,  -192,   115,  -192,  -192,  -192,  -192,
    -192,  -192,  -192,  -192,  -192,  -192,  -192
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     5,     6,     7,     8,    19,     9,    20,    10,    16,
      17,    25,    53,    54,    86,    11,    32,    33,   136,   100,
     137,   138,   139,   140,   141,   142,   143,   144,   145,   146,
     147,   148,   149,   150,   151,   152,   153,    55,    56,    57,
      58,    59,    60,    61,    62,   117,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      74,   155,    12,    28,    75,    24,   106,   206,   207,    39,
      18,   107,    15,    21,   159,    76,    40,    87,    85,   213,
      99,   160,    31,   159,    30,    22,    23,    97,    21,    78,
     205,   101,    38,     1,     2,     3,     4,    31,    96,    88,
      89,    80,    90,    91,    92,    93,    94,   102,   104,   105,
      81,   202,    82,   109,   110,   111,   112,   113,   114,   115,
     116,    26,    27,    88,    89,   154,    90,    91,    92,    93,
      94,    34,    35,   158,    41,    42,    43,    44,    45,    46,
      83,   118,   108,    13,   161,     1,     2,     3,    47,    48,
      49,    50,     1,     2,     3,     4,    84,    15,   135,   155,
     155,    37,    35,    88,    89,    51,    90,    52,    92,    93,
      94,   155,    95,    88,    89,   103,    88,    89,    92,    90,
      91,    92,    93,    94,   186,    92,    35,   187,   188,   189,
     190,   191,   192,   193,   116,   156,    30,    41,    42,    43,
      44,    45,    46,   162,   163,   164,   165,   166,   167,   168,
     197,    47,    48,    49,    50,   169,   170,   171,   172,   173,
      15,    88,    89,   154,   154,   174,    92,   208,    94,   209,
      52,   175,   185,   119,   120,   154,   121,   122,   123,     1,
       2,     3,     4,    41,    42,    43,    44,    45,    46,   124,
     125,   126,   127,   128,   129,   130,   131,    47,    48,    49,
      50,   176,   177,   178,   179,   180,    15,   119,   120,   132,
     121,   122,   123,   181,    78,   133,    52,    41,    42,    43,
      44,    45,    46,   124,   125,   126,   127,   128,   129,   130,
     131,    47,    48,    49,    50,   182,   183,   184,   195,   196,
      15,   210,    14,   132,   134,    36,    77,    29,    78,    79,
      52,    88,    89,     0,    90,    91,    92,    93,    94,    88,
      89,     0,    90,    91,    92,    93,    94,     0,    88,    89,
     157,    90,    91,    92,    93,    94,    88,    89,   198,    90,
      91,    92,    93,    94,   194,    88,    89,   199,    90,    91,
      92,    93,    94,    88,    89,   200,    90,    91,    92,    93,
      94,     0,    88,    89,   201,    90,    91,    92,    93,    94,
      88,    89,   203,    90,    91,    92,    93,    94,     0,    88,
      89,   211,    90,    91,    92,    93,    94,     0,     0,   212,
       0,     0,    88,    89,    98,    90,    91,    92,    93,    94,
       0,     0,    88,    89,   204,    90,    91,    92,    93,    94,
      88,    89,     0,     0,     0,    92,    93,    94
};

static const yytype_int16 yycheck[] =
{
      35,   100,     4,    21,    37,    46,    40,   198,   199,    40,
       9,    45,    36,    12,    40,    42,    47,    52,    51,   210,
      43,    47,    24,    40,    23,    39,    40,    62,    27,    44,
      47,    47,    31,     9,    10,    11,    12,    39,    56,    28,
      29,    46,    31,    32,    33,    34,    35,    47,    83,    84,
      46,    40,    46,    88,    89,    90,    91,    92,    93,    94,
      95,    39,    40,    28,    29,   100,    31,    32,    33,    34,
      35,    41,    42,   106,    13,    14,    15,    16,    17,    18,
      46,    99,    47,     0,   119,     9,    10,    11,    27,    28,
      29,    30,     9,    10,    11,    12,    46,    36,   100,   198,
     199,    41,    42,    28,    29,    44,    31,    46,    33,    34,
      35,   210,    46,    28,    29,    47,    28,    29,    33,    31,
      32,    33,    34,    35,   159,    33,    42,   162,   163,   164,
     165,   166,   167,   168,   169,    47,   135,    13,    14,    15,
      16,    17,    18,    46,    46,    46,    46,    46,    46,    46,
     185,    27,    28,    29,    30,    46,    46,    46,    39,    39,
      36,    28,    29,   198,   199,    39,    33,   202,    35,   204,
      46,    39,    41,     3,     4,   210,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    39,    39,    39,    39,    39,    36,     3,     4,    39,
       6,     7,     8,    39,    44,    45,    46,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    39,    39,    39,    47,    47,
      36,     5,     5,    39,   100,    27,    39,    23,    44,    40,
      46,    28,    29,    -1,    31,    32,    33,    34,    35,    28,
      29,    -1,    31,    32,    33,    34,    35,    -1,    28,    29,
      47,    31,    32,    33,    34,    35,    28,    29,    47,    31,
      32,    33,    34,    35,   169,    28,    29,    47,    31,    32,
      33,    34,    35,    28,    29,    47,    31,    32,    33,    34,
      35,    -1,    28,    29,    47,    31,    32,    33,    34,    35,
      28,    29,    47,    31,    32,    33,    34,    35,    -1,    28,
      29,    47,    31,    32,    33,    34,    35,    -1,    -1,    47,
      -1,    -1,    28,    29,    43,    31,    32,    33,    34,    35,
      -1,    -1,    28,    29,    40,    31,    32,    33,    34,    35,
      28,    29,    -1,    -1,    -1,    33,    34,    35
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     9,    10,    11,    12,    49,    50,    51,    52,    54,
      56,    63,    54,     0,    50,    36,    57,    58,    86,    53,
      55,    86,    39,    40,    46,    59,    39,    40,    59,    58,
      86,    54,    64,    65,    41,    42,    55,    41,    86,    40,
      47,    13,    14,    15,    16,    17,    18,    27,    28,    29,
      30,    44,    46,    60,    61,    85,    86,    87,    88,    89,
      90,    91,    92,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104,    85,    60,    42,    65,    44,    66,
      46,    46,    46,    46,    46,    60,    62,    85,    28,    29,
      31,    32,    33,    34,    35,    46,    59,    85,    43,    43,
      67,    47,    47,    47,    85,    85,    40,    45,    47,    85,
      85,    85,    85,    85,    85,    85,    85,    93,    59,     3,
       4,     6,     7,     8,    19,    20,    21,    22,    23,    24,
      25,    26,    39,    45,    51,    54,    66,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    87,    47,    47,    60,    40,
      47,    85,    46,    46,    46,    46,    46,    46,    46,    46,
      46,    46,    39,    39,    39,    39,    39,    39,    39,    39,
      39,    39,    39,    39,    39,    41,    85,    85,    85,    85,
      85,    85,    85,    85,    93,    47,    47,    85,    47,    47,
      47,    47,    40,    47,    40,    47,    69,    69,    85,    85,
       5,    47,    47,    69
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    48,    49,    49,    50,    50,    51,    51,    52,    53,
      53,    54,    54,    54,    55,    56,    57,    57,    58,    58,
      59,    59,    60,    60,    61,    62,    62,    62,    63,    64,
      64,    64,    65,    65,    66,    67,    67,    68,    68,    69,
      69,    69,    69,    69,    69,    69,    69,    69,    69,    69,
      69,    69,    69,    69,    69,    69,    70,    71,    72,    72,
      73,    74,    75,    76,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    85,    85,    85,    85,    85,    85,    85,
      85,    85,    85,    85,    85,    85,    85,    86,    87,    88,
      88,    88,    89,    89,    90,    91,    92,    92,    92,    93,
      93,    93,    94,    95,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   104
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     4,     1,
       3,     1,     1,     1,     4,     3,     1,     3,     2,     4,
       0,     4,     1,     1,     3,     0,     1,     3,     6,     0,
       1,     3,     2,     5,     3,     0,     2,     1,     1,     2,
       1,     2,     1,     1,     1,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     3,     1,     5,     7,
       5,     1,     1,     1,     2,     4,     4,     6,     4,     6,
       4,     3,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
       1,     1,     1,     1,     2,     4,     1,     1,     1,     0,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     4,     4
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 109 "parser.yacc"
                                    {
	/* CompUnitList -> CompUnitList CompUnit */
	(yyval.root) = (yyvsp[-1].root);
	(yyval.root)->list.push_back((yyvsp[0].compunit));

	root = (yyval.root);

}
#line 1616 "parser.cpp"
    break;

  case 3:
#line 116 "parser.yacc"
             {
	/* CompUnitList -> CompUnit */
	(yyval.root) = new AST::CompUnitList(yyget_lineno());
	(yyval.root)->list.push_back((yyvsp[0].compunit));
	
	root = (yyval.root);
}
#line 1628 "parser.cpp"
    break;

  case 4:
#line 124 "parser.yacc"
                {
	/* CompUnit -> Decl */
	(yyval.compunit) = static_cast<AST::CompUnit *>((yyvsp[0].decl));
}
#line 1637 "parser.cpp"
    break;

  case 5:
#line 127 "parser.yacc"
            {
	/* CompUint -> FuncDef */
	(yyval.compunit) = static_cast<AST::CompUnit *>((yyvsp[0].funcdef));
}
#line 1646 "parser.cpp"
    break;

  case 6:
#line 132 "parser.yacc"
                {
	/* Decl -> ConstDecl */
	(yyval.decl) = static_cast<AST::Decl *>((yyvsp[0].constdecl));
}
#line 1655 "parser.cpp"
    break;

  case 7:
#line 135 "parser.yacc"
            {
	/* Decl -> VarDecl */
	(yyval.decl) = static_cast<AST::Decl *>((yyvsp[0].vardecl));
}
#line 1664 "parser.cpp"
    break;

  case 8:
#line 140 "parser.yacc"
                                        {
	/* ConstDecl -> const Btype ConstDef (, ConstDef)* ; */
	(yyval.constdecl) = new AST::ConstDecl((yyvsp[-2].btype), (yyvsp[-1].constdeflist), yyget_lineno());
}
#line 1673 "parser.cpp"
    break;

  case 9:
#line 145 "parser.yacc"
                       {
	(yyval.constdeflist) = new AST::ConstDefList(yyget_lineno());
	(yyval.constdeflist)->list.push_back((yyvsp[0].constdef));
}
#line 1682 "parser.cpp"
    break;

  case 10:
#line 148 "parser.yacc"
                              {
	(yyval.constdeflist) = (yyvsp[-2].constdeflist);
	(yyvsp[-2].constdeflist)->list.push_back((yyvsp[0].constdef));
}
#line 1691 "parser.cpp"
    break;

  case 11:
#line 153 "parser.yacc"
            {
	(yyval.btype) = AST::btype_t::INT;
}
#line 1699 "parser.cpp"
    break;

  case 12:
#line 155 "parser.yacc"
           {
	(yyval.btype) = AST::btype_t::FLOAT;
}
#line 1707 "parser.cpp"
    break;

  case 13:
#line 157 "parser.yacc"
          {
	(yyval.btype) = AST::btype_t::VOID;
}
#line 1715 "parser.cpp"
    break;

  case 14:
#line 161 "parser.yacc"
                                       {
	/* ConstDef -> ID ( [Exp] )* = InitVal */
	(yyval.constdef) = new AST::ConstDef((yyvsp[-3].idexp), (yyvsp[-2].arrayindex), (yyvsp[0].initval), yyget_lineno());
}
#line 1724 "parser.cpp"
    break;

  case 15:
#line 166 "parser.yacc"
                              {
	(yyval.vardecl) = new AST::VarDecl((yyvsp[-2].btype), (yyvsp[-1].vardeflist), yyget_lineno());
}
#line 1732 "parser.cpp"
    break;

  case 16:
#line 170 "parser.yacc"
                   {
	(yyval.vardeflist) = new AST::VarDefList(yyget_lineno());
	(yyval.vardeflist)->list.push_back((yyvsp[0].vardef));
}
#line 1741 "parser.cpp"
    break;

  case 17:
#line 173 "parser.yacc"
                          {
	(yyval.vardeflist) = (yyvsp[-2].vardeflist);
	(yyval.vardeflist)->list.push_back((yyvsp[0].vardef));
}
#line 1750 "parser.cpp"
    break;

  case 18:
#line 178 "parser.yacc"
                         {
	(yyval.vardef) = new AST::VarDef((yyvsp[-1].idexp), (yyvsp[0].arrayindex), nullptr, yyget_lineno());
}
#line 1758 "parser.cpp"
    break;

  case 19:
#line 180 "parser.yacc"
                                 {
	(yyval.vardef) = new AST::VarDef((yyvsp[-3].idexp), (yyvsp[-2].arrayindex), (yyvsp[0].initval), yyget_lineno());
}
#line 1766 "parser.cpp"
    break;

  case 20:
#line 184 "parser.yacc"
             {
	(yyval.arrayindex) = new AST::ArrayIndex(yyget_lineno());
}
#line 1774 "parser.cpp"
    break;

  case 21:
#line 186 "parser.yacc"
                           {
	(yyval.arrayindex) = (yyvsp[-3].arrayindex);
	(yyval.arrayindex)->list.push_back((yyvsp[-1].exp));
}
#line 1783 "parser.cpp"
    break;

  case 22:
#line 191 "parser.yacc"
             {
	(yyval.initval) = (yyvsp[0].exp);
}
#line 1791 "parser.cpp"
    break;

  case 23:
#line 193 "parser.yacc"
              {
	(yyval.initval) = (yyvsp[0].arrayinit);
}
#line 1799 "parser.cpp"
    break;

  case 24:
#line 197 "parser.yacc"
                               {
	(yyval.arrayinit) = (yyvsp[-1].initvallist);
}
#line 1807 "parser.cpp"
    break;

  case 25:
#line 201 "parser.yacc"
             {
	(yyval.initvallist) = new AST::InitValList(yyget_lineno());
}
#line 1815 "parser.cpp"
    break;

  case 26:
#line 203 "parser.yacc"
            {
	(yyval.initvallist) = new AST::InitValList((yyvsp[0].initval), yyget_lineno());
}
#line 1823 "parser.cpp"
    break;

  case 27:
#line 205 "parser.yacc"
                            {
	(yyval.initvallist) = (yyvsp[-2].initvallist);
	(yyval.initvallist)->list.push_back((yyvsp[0].initval));
}
#line 1832 "parser.cpp"
    break;

  case 28:
#line 210 "parser.yacc"
                                              {
	(yyval.funcdef) = new AST::FuncDef((yyvsp[-5].btype), (yyvsp[-4].idexp), (yyvsp[-2].parameters), (yyvsp[0].block), yyget_lineno());
}
#line 1840 "parser.cpp"
    break;

  case 29:
#line 214 "parser.yacc"
             {
	(yyval.parameters) = new AST::Parameters(yyget_lineno());
}
#line 1848 "parser.cpp"
    break;

  case 30:
#line 216 "parser.yacc"
              {
	(yyval.parameters) = new AST::Parameters((yyvsp[0].parameter), yyget_lineno());
}
#line 1856 "parser.cpp"
    break;

  case 31:
#line 218 "parser.yacc"
                             {
	(yyval.parameters) = (yyvsp[-2].parameters);
	(yyval.parameters)->list.push_back((yyvsp[0].parameter));
}
#line 1865 "parser.cpp"
    break;

  case 32:
#line 223 "parser.yacc"
                       {
	(yyval.parameter) = new AST::Parameter((yyvsp[-1].btype), (yyvsp[0].idexp), new AST::ArrayIndex(yyget_lineno()), yyget_lineno());
}
#line 1873 "parser.cpp"
    break;

  case 33:
#line 225 "parser.yacc"
                                   {
	auto x = new AST::ArrayIndex(yyget_lineno());
	x->list.push_back(new AST::IntNumber(1, yyget_lineno()));
	for (const auto &y:(yyvsp[0].arrayindex)->list) {
		x->list.push_back(y);
	}
	(yyval.parameter) = new AST::Parameter((yyvsp[-4].btype), (yyvsp[-3].idexp), x, yyget_lineno());
}
#line 1886 "parser.cpp"
    break;

  case 34:
#line 234 "parser.yacc"
                             {
	(yyval.block) = new AST::Block((yyvsp[-1].blockitemlist), yyget_lineno());
}
#line 1894 "parser.cpp"
    break;

  case 35:
#line 238 "parser.yacc"
                {
	(yyval.blockitemlist) = new AST::BlockItemList(yyget_lineno());
}
#line 1902 "parser.cpp"
    break;

  case 36:
#line 240 "parser.yacc"
                            {
	(yyval.blockitemlist) = (yyvsp[-1].blockitemlist);
	(yyval.blockitemlist)->list.push_back((yyvsp[0].blockitem));
}
#line 1911 "parser.cpp"
    break;

  case 37:
#line 245 "parser.yacc"
                {
	(yyval.blockitem) = (yyvsp[0].decl);
}
#line 1919 "parser.cpp"
    break;

  case 38:
#line 247 "parser.yacc"
         {
	(yyval.blockitem) = (yyvsp[0].stmt);
}
#line 1927 "parser.cpp"
    break;

  case 39:
#line 251 "parser.yacc"
                     {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 1935 "parser.cpp"
    break;

  case 40:
#line 253 "parser.yacc"
        {
	(yyval.stmt) = new AST::ExpStmt(new AST::IntNumber(0, yyget_lineno()), yyget_lineno());
}
#line 1943 "parser.cpp"
    break;

  case 41:
#line 255 "parser.yacc"
                {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 1951 "parser.cpp"
    break;

  case 42:
#line 257 "parser.yacc"
          {
	(yyval.stmt) = (yyvsp[0].block);
}
#line 1959 "parser.cpp"
    break;

  case 43:
#line 259 "parser.yacc"
           {
	(yyval.stmt) = (yyvsp[0].stmt);
}
#line 1967 "parser.cpp"
    break;

  case 44:
#line 261 "parser.yacc"
              {
	(yyval.stmt) = (yyvsp[0].stmt);
}
#line 1975 "parser.cpp"
    break;

  case 45:
#line 263 "parser.yacc"
                  {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 1983 "parser.cpp"
    break;

  case 46:
#line 265 "parser.yacc"
                     {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 1991 "parser.cpp"
    break;

  case 47:
#line 267 "parser.yacc"
                   {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 1999 "parser.cpp"
    break;

  case 48:
#line 269 "parser.yacc"
                   {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2007 "parser.cpp"
    break;

  case 49:
#line 271 "parser.yacc"
                  {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2015 "parser.cpp"
    break;

  case 50:
#line 273 "parser.yacc"
                     {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2023 "parser.cpp"
    break;

  case 51:
#line 275 "parser.yacc"
                     {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2031 "parser.cpp"
    break;

  case 52:
#line 277 "parser.yacc"
                      {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2039 "parser.cpp"
    break;

  case 53:
#line 279 "parser.yacc"
                 {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2047 "parser.cpp"
    break;

  case 54:
#line 281 "parser.yacc"
                      {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2055 "parser.cpp"
    break;

  case 55:
#line 283 "parser.yacc"
                     {
	(yyval.stmt) = (yyvsp[-1].stmt);
}
#line 2063 "parser.cpp"
    break;

  case 56:
#line 287 "parser.yacc"
                         {
	(yyval.stmt) = new AST::AssignStmt((yyvsp[-2].lval), (yyvsp[0].exp), yyget_lineno());
}
#line 2071 "parser.cpp"
    break;

  case 57:
#line 291 "parser.yacc"
             {
	(yyval.stmt) = new AST::ExpStmt((yyvsp[0].exp), yyget_lineno());
}
#line 2079 "parser.cpp"
    break;

  case 58:
#line 295 "parser.yacc"
                                        {
	(yyval.stmt) = new AST::IfStmt((yyvsp[-2].exp), (yyvsp[0].stmt), nullptr, yyget_lineno());
}
#line 2087 "parser.cpp"
    break;

  case 59:
#line 297 "parser.yacc"
                                  {
	(yyval.stmt) = new AST::IfStmt((yyvsp[-4].exp), (yyvsp[-2].stmt), (yyvsp[0].stmt), yyget_lineno());
}
#line 2095 "parser.cpp"
    break;

  case 60:
#line 301 "parser.yacc"
                                  {
	(yyval.stmt) = new AST::WhileStmt((yyvsp[-2].exp), (yyvsp[0].stmt), yyget_lineno());
}
#line 2103 "parser.cpp"
    break;

  case 61:
#line 305 "parser.yacc"
                 {
	(yyval.stmt) = new AST::BreakStmt(yyget_lineno());
}
#line 2111 "parser.cpp"
    break;

  case 62:
#line 309 "parser.yacc"
                       {
	(yyval.stmt) = new AST::ContinueStmt(yyget_lineno());
}
#line 2119 "parser.cpp"
    break;

  case 63:
#line 313 "parser.yacc"
                   {
	(yyval.stmt) = new AST::ReturnStmt(nullptr, yyget_lineno());
}
#line 2127 "parser.cpp"
    break;

  case 64:
#line 315 "parser.yacc"
               {
	(yyval.stmt) = new AST::ReturnStmt((yyvsp[0].exp), yyget_lineno());
}
#line 2135 "parser.cpp"
    break;

  case 65:
#line 319 "parser.yacc"
                               {
	(yyval.stmt) = new AST::PutintStmt((yyvsp[-1].exp), yyget_lineno());
}
#line 2143 "parser.cpp"
    break;

  case 66:
#line 323 "parser.yacc"
                             {
	(yyval.stmt) = new AST::PutchStmt((yyvsp[-1].exp), yyget_lineno());
}
#line 2151 "parser.cpp"
    break;

  case 67:
#line 327 "parser.yacc"
                                           {
	(yyval.stmt) = new AST::PutarrayStmt((yyvsp[-3].exp), (yyvsp[-1].exp), yyget_lineno());
}
#line 2159 "parser.cpp"
    break;

  case 68:
#line 331 "parser.yacc"
                                   {
	(yyval.stmt) = new AST::PutfloatStmt((yyvsp[-1].exp), yyget_lineno());
}
#line 2167 "parser.cpp"
    break;

  case 69:
#line 335 "parser.yacc"
                                             {
	(yyval.stmt) = new AST::PutfarrayStmt((yyvsp[-3].exp), (yyvsp[-1].exp), yyget_lineno());
}
#line 2175 "parser.cpp"
    break;

  case 70:
#line 339 "parser.yacc"
                               {
	(yyval.stmt) = new AST::PutfStmt((yyvsp[-1].explist), yyget_lineno());
}
#line 2183 "parser.cpp"
    break;

  case 71:
#line 343 "parser.yacc"
                                 {
	(yyval.stmt) = new AST::StarttimeStmt(yyget_lineno());
}
#line 2191 "parser.cpp"
    break;

  case 72:
#line 347 "parser.yacc"
                               {
	(yyval.stmt) = new AST::StoptimeStmt(yyget_lineno());
}
#line 2199 "parser.cpp"
    break;

  case 73:
#line 351 "parser.yacc"
                {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2207 "parser.cpp"
    break;

  case 74:
#line 353 "parser.yacc"
             {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2215 "parser.cpp"
    break;

  case 75:
#line 355 "parser.yacc"
           {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2223 "parser.cpp"
    break;

  case 76:
#line 357 "parser.yacc"
           {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2231 "parser.cpp"
    break;

  case 77:
#line 359 "parser.yacc"
           {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2239 "parser.cpp"
    break;

  case 78:
#line 361 "parser.yacc"
          {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2247 "parser.cpp"
    break;

  case 79:
#line 363 "parser.yacc"
            {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2255 "parser.cpp"
    break;

  case 80:
#line 365 "parser.yacc"
           {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2263 "parser.cpp"
    break;

  case 81:
#line 367 "parser.yacc"
            {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2271 "parser.cpp"
    break;

  case 82:
#line 369 "parser.yacc"
              {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2279 "parser.cpp"
    break;

  case 83:
#line 371 "parser.yacc"
             {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2287 "parser.cpp"
    break;

  case 84:
#line 373 "parser.yacc"
                {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2295 "parser.cpp"
    break;

  case 85:
#line 375 "parser.yacc"
                {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2303 "parser.cpp"
    break;

  case 86:
#line 377 "parser.yacc"
                 {
	(yyval.exp) = (yyvsp[0].exp);
}
#line 2311 "parser.cpp"
    break;

  case 87:
#line 381 "parser.yacc"
          {
	(yyval.idexp) = new AST::IdExp((yyvsp[0].id), yyget_lineno());
}
#line 2319 "parser.cpp"
    break;

  case 88:
#line 385 "parser.yacc"
                       {
	(yyval.lval) = new AST::Lval((yyvsp[-1].idexp), (yyvsp[0].arrayindex), yyget_lineno());
}
#line 2327 "parser.cpp"
    break;

  case 89:
#line 389 "parser.yacc"
                        {
	(yyval.exp) = (yyvsp[-1].exp);
}
#line 2335 "parser.cpp"
    break;

  case 90:
#line 391 "parser.yacc"
         {
	(yyval.exp) = (yyvsp[0].lval);
}
#line 2343 "parser.cpp"
    break;

  case 91:
#line 393 "parser.yacc"
           {
	(yyval.exp) = (yyvsp[0].number);
}
#line 2351 "parser.cpp"
    break;

  case 92:
#line 397 "parser.yacc"
                  {
	(yyval.number) = new AST::IntNumber((yyvsp[0].token), yyget_lineno());
}
#line 2359 "parser.cpp"
    break;

  case 93:
#line 399 "parser.yacc"
                {
	(yyval.number) = new AST::FloatNumber((yyvsp[0].floatnumber), yyget_lineno());
}
#line 2367 "parser.cpp"
    break;

  case 94:
#line 403 "parser.yacc"
                                  {
	(yyval.exp) = new AST::UnaryExp((yyvsp[-1].unaryop), (yyvsp[0].exp), yyget_lineno());
}
#line 2375 "parser.cpp"
    break;

  case 95:
#line 407 "parser.yacc"
                               {
	(yyval.exp) = new AST::CallExp((yyvsp[-3].idexp), (yyvsp[-1].explist), yyget_lineno());
}
#line 2383 "parser.cpp"
    break;

  case 96:
#line 412 "parser.yacc"
               {
	(yyval.unaryop) = AST::unaryop_t::ADD;
}
#line 2391 "parser.cpp"
    break;

  case 97:
#line 414 "parser.yacc"
          {
	(yyval.unaryop) = AST::unaryop_t::SUB;
}
#line 2399 "parser.cpp"
    break;

  case 98:
#line 416 "parser.yacc"
          {
	(yyval.unaryop) = AST::unaryop_t::NOT;
}
#line 2407 "parser.cpp"
    break;

  case 99:
#line 420 "parser.yacc"
          {
	(yyval.explist) = new AST::ExpList(yyget_lineno());
}
#line 2415 "parser.cpp"
    break;

  case 100:
#line 422 "parser.yacc"
        {
	(yyval.explist) = new AST::ExpList((yyvsp[0].exp), yyget_lineno());
}
#line 2423 "parser.cpp"
    break;

  case 101:
#line 424 "parser.yacc"
                    {
	(yyval.explist) = (yyvsp[-2].explist);
	(yyval.explist)->list.push_back((yyvsp[0].exp));
}
#line 2432 "parser.cpp"
    break;

  case 102:
#line 429 "parser.yacc"
                      {
	(yyval.exp) = new AST::MulExp((yyvsp[-2].exp), (yyvsp[-1].mul), (yyvsp[0].exp), yyget_lineno());
}
#line 2440 "parser.cpp"
    break;

  case 103:
#line 433 "parser.yacc"
                      {
	(yyval.exp) = new AST::AddExp((yyvsp[-2].exp), AST::addop_t::ADD, (yyvsp[0].exp), yyget_lineno());
}
#line 2448 "parser.cpp"
    break;

  case 104:
#line 435 "parser.yacc"
                  {
	(yyval.exp) = new AST::AddExp((yyvsp[-2].exp), AST::addop_t::SUB, (yyvsp[0].exp), yyget_lineno());
}
#line 2456 "parser.cpp"
    break;

  case 105:
#line 439 "parser.yacc"
                      {
	(yyval.exp) = new AST::RelExp((yyvsp[-2].exp), (yyvsp[-1].rel), (yyvsp[0].exp), yyget_lineno());
}
#line 2464 "parser.cpp"
    break;

  case 106:
#line 443 "parser.yacc"
                       {
	(yyval.exp) = new AST::EqExp((yyvsp[-2].exp), (yyvsp[-1].equal), (yyvsp[0].exp), yyget_lineno());
}
#line 2472 "parser.cpp"
    break;

  case 107:
#line 447 "parser.yacc"
                        {
	(yyval.exp) = new AST::LAndExp((yyvsp[-2].exp), (yyvsp[0].exp), yyget_lineno());
}
#line 2480 "parser.cpp"
    break;

  case 108:
#line 451 "parser.yacc"
                      {
	(yyval.exp) = new AST::LOrExp((yyvsp[-2].exp), (yyvsp[0].exp), yyget_lineno());
}
#line 2488 "parser.cpp"
    break;

  case 109:
#line 455 "parser.yacc"
                          {
	(yyval.exp) = new AST::GetintExp(yyget_lineno());
}
#line 2496 "parser.cpp"
    break;

  case 110:
#line 459 "parser.yacc"
                        {
	(yyval.exp) = new AST::GetchExp(yyget_lineno());
}
#line 2504 "parser.cpp"
    break;

  case 111:
#line 463 "parser.yacc"
                              {
	(yyval.exp) = new AST::GetfloatExp(yyget_lineno());
}
#line 2512 "parser.cpp"
    break;

  case 112:
#line 467 "parser.yacc"
                                  {
	(yyval.exp) = new AST::GetarrayExp((yyvsp[-1].exp), yyget_lineno());
}
#line 2520 "parser.cpp"
    break;

  case 113:
#line 471 "parser.yacc"
                                    {
	(yyval.exp) = new AST::GetfarrayExp((yyvsp[-1].exp), yyget_lineno());
}
#line 2528 "parser.cpp"
    break;


#line 2532 "parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 476 "parser.yacc"
