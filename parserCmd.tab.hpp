/* A Bison parser, made by GNU Bison 3.4.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
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

#ifndef YY_TESTER_PARSERCMD_TAB_HPP_INCLUDED
# define YY_TESTER_PARSERCMD_TAB_HPP_INCLUDED
/* Debug traces.  */
#ifndef TESTERDEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define TESTERDEBUG 1
#  else
#   define TESTERDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define TESTERDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined TESTERDEBUG */
#if TESTERDEBUG
extern int Testerdebug;
#endif

/* Token type.  */
#ifndef TESTERTOKENTYPE
# define TESTERTOKENTYPE
  enum Testertokentype
  {
    TK_STRING = 258,
    TK_ID = 259,
    TK_NUM = 260,
    TK_INCLUDE = 261,
    TK_ACTIVATE = 262,
    TK_TRUE = 263,
    TK_FALSE = 264,
    TK_SCOPE = 265
  };
#endif

/* Value type.  */
#if ! defined TESTERSTYPE && ! defined TESTERSTYPE_IS_DECLARED
union TESTERSTYPE
{
#line 18 "parserCmd.y"

    int ival;
    char string[512];
    float num;

#line 82 "parserCmd.tab.hpp"

};
typedef union TESTERSTYPE TESTERSTYPE;
# define TESTERSTYPE_IS_TRIVIAL 1
# define TESTERSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined TESTERLTYPE && ! defined TESTERLTYPE_IS_DECLARED
typedef struct TESTERLTYPE TESTERLTYPE;
struct TESTERLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define TESTERLTYPE_IS_DECLARED 1
# define TESTERLTYPE_IS_TRIVIAL 1
#endif


extern TESTERSTYPE Testerlval;
extern TESTERLTYPE Testerlloc;
int Testerparse (Tester *a);

#endif /* !YY_TESTER_PARSERCMD_TAB_HPP_INCLUDED  */
