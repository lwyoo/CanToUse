%option noinput
%option nounput
%option nounistd
%option never-interactive 
%option batch
%option nodefault
%option prefix="Tester"

%{
#include <stdio.h>
#include <stdbool.h>
#include <vector>
#include <QString>
#include <QDir>
#include "Tester.h"
#include "value_string.h"
#include "parserCmd.tab.hpp"
#define yylval Testerlval
#define yylloc Testerlloc
//ftp://ftp.gnu.org/old-gnu/Manuals/flex-2.5.4/html_mono/flex.html#SEC5
extern int line_num;
extern QString current_file;
/*#ifdef YY_NO_UNISTD_H
int isatty(int fd)
{
	return 0;
}
#endif*/

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line; \
    yylloc.first_column = yylloc.last_column; \
    for (const char *c = yytext; *c != '\0'; c++) \
    { \
        if (*c == '\n') \
        { \
            yylloc.last_line++; \
            yylloc.last_column = 0; \
        } \
        else \
        { \
            yylloc.last_column++; \
        } \
    }


struct IncludeInfo {
    QString filename;
    YY_BUFFER_STATE bufState;
    int lineNum;
};


std::vector<IncludeInfo> includeStack;

int str_pos;

void add_string_char(char c) {
    yylval.string[str_pos++] = c;
    yylval.string[str_pos] = '\0';
}

void include_push(char *filename) {
        QString filePath(filename);
        QDir azzuassasa(filePath);

    if (includeStack.size() > 32) {
        printf("Only 32 levels of nested Include allowed in scene files.");
        exit(1);
    }

    const QString new_file = azzuassasa.absolutePath();
    std::string strPath = new_file.toStdString();
    FILE *f = fopen(strPath.c_str(), "r");
    if (!f)
        printf("Unable to open included scene file \"%s\"", strPath.c_str());
    else {

        IncludeInfo ii;
        ii.filename = current_file;
        ii.bufState = YY_CURRENT_BUFFER;
        ii.lineNum = line_num;
        includeStack.push_back(ii);

        yyin = f;
        current_file = new_file;
        line_num = 1;

        yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
    }
}

void include_pop() {
    fclose(yyin);
    yy_delete_buffer(YY_CURRENT_BUFFER);
    yy_switch_to_buffer(includeStack.back().bufState);
    current_file = includeStack.back().filename;
    line_num = includeStack.back().lineNum;
    includeStack.pop_back();
}

%}
WHITESPACE [ \t\r]+
NUMBER [-+]?([0-9]+|(([0-9]+\.[0-9]*)|(\.[0-9]+)))([eE][-+]?[0-9]+)?
IDENT [a-zA-Z_][a-zA-Z_0-9]*
%x STR COMMENT INCL INCL_FILE
%%
Include                 { return TK_INCLUDE; }
"#" { BEGIN COMMENT; }
<COMMENT>. /* eat it up */
<COMMENT>\n { line_num++; BEGIN INITIAL; }
{WHITESPACE} /* do nothing */
\n { line_num++; }
{NUMBER} {
    //yylval.num = (float) atof(yytext);
    strcpy(yylval.string, yytext);
    return TK_NUM;
}

"true" {return TK_TRUE; }
"false" {return TK_FALSE; }
"::" { return TK_SCOPE; }
"Activate" { return TK_ACTIVATE; }
{IDENT} {
    strcpy(yylval.string, yytext);
    return TK_ID;
}
";" { return ';'; }
"=" { return '='; }
"(" { return '('; }
")" { return ')'; }

\" { BEGIN STR; str_pos = 0; }
<STR>\\n {add_string_char('\n');}
<STR>\\t {add_string_char('\t');}
<STR>\\r {add_string_char('\r');}
<STR>\\b {add_string_char('\b');}
<STR>\\f {add_string_char('\f');}
<STR>\\\" {add_string_char('\"');}
<STR>\\\\ {add_string_char('\\');}
<STR>\\[0-9]{3} {
  int val = atoi(yytext+1);
  while (val > 256)
    val -= 256;
  add_string_char(val);
}

<STR>\\\n {line_num++;}
<STR>\\. { add_string_char(yytext[1]);}
<STR>\" {BEGIN INITIAL; return TK_STRING;}
<STR>. {add_string_char(yytext[0]);}
<STR>\n {printf("Unterminated string!");}

. { printf( "Illegal character: %c (0x%x)", yytext[0], int(yytext[0])); }
%%
int yywrap() {
    if (includeStack.size() == 0) return 1;
    include_pop();
    return 0;
}
