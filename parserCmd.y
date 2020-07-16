%{
#include <stdio.h>
#include <stdbool.h>
#include <QString>
#include "Tester.h"
#include "parserCmd.tab.hpp"
#include "scannerCmd.yy.h"

void Testererror(Tester *pTester, const char *s);
int line_num = 0;
QString current_file;
void include_push(char *filename);
static Tester *pTester;
char buf[512];
%}
%define api.prefix {Tester}

%union {
    int ival;
    char string[512];
    float num;
}

%expect 0
%locations
%define parse.error verbose
%parse-param {Tester *a}

%token <string> TK_STRING TK_ID TK_NUM
%token TK_INCLUDE TK_ACTIVATE
%token <ival> TK_TRUE TK_FALSE
%token TK_SCOPE
%type <string> var can_signal
%%

file: statement_list
    ;
statement_list
    : statement
    | statement_list statement
    ;
statement
        : compound_statement
        | expression_statement
        | include_statement
        | unary_expression
        | activate_message_statement
//      | selection_statement
//      | iteration_statement
        ;
compound_statement
        : '{' '}'
        | '{' statement_list '}'
        ;
expression_statement
        : ';'
        | expression ';'
        ;
expression
        : assignment_expression
        ;
activate_message_statement : TK_ACTIVATE '(' TK_STRING ')' { pTester->GenActivate($3);  }
    ;
assignment_expression: var '=' unary_expression { pTester->GenAssignment($1);  }
    ;
var : TK_ID
| can_signal { printf(f)
    ;
can_signal : TK_ID TK_SCOPE TK_ID { sprintf($$, "%s::%s", $1, $3); pTester->GenPushSymbol($$, DT_CANSIG);}
    ;
include_statement : TK_INCLUDE TK_STRING { include_push($2); }
    ;
unary_expression : TK_ID { pTester->GenPushSymbol($1, DT_VAR); }
    | TK_NUM { pTester->GenPushSymbol($1, DT_FLOAT); }
    | TK_STRING { pTester->GenPushSymbol($1, DT_STRING); }
    | TK_TRUE { pTester->GenPushSymbol("1", DT_BOOL);}
    | TK_FALSE { pTester->GenPushSymbol("0", DT_BOOL); }
    | TK_ID '(' ')' { pTester->GenCallFunc($1); }
    | TK_ID '(' argument_expression_list ')' { pTester->GenCallFunc($1); }
    ;
argument_expression_list
    : unary_expression
    | argument_expression_list ',' unary_expression
    ;
%%

static int ret_code = 0;

static int readTC(Tester *pTester, const char *filePath)
{
    FILE *in;

    ::pTester = pTester;
    in = fopen(filePath, "r");
    printf("Read TC = %s fd = %p\n", filePath, in);
    Testerset_in(in);
    yylloc.last_line = 1;
    yylloc.last_column = 0;
    yyparse(pTester);
    Testerlex_destroy();
    fclose(in);

    return 0;
}

bool Tester::ReadTestCase(const char *filePath)
{
    mProg.clear();
    ::readTC(this, filePath);
    mProg.append(TERMINATE);
    return true;
}

void Testererror(Tester *dbc, const char *s)
{
   // fprintf(stderr, "%s:%i:%i: %s\n", dbc->filepath, yylloc.first_line, yylloc.first_column, s);
    ret_code = 1;
}
