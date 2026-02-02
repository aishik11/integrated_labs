%{
#include <string.h>
#include "parser.tab.h"

//- Function to handle labels
static int handle_label(char* label) {
    //- Remove the trailing ":"
    label[strlen(label) - 1] = '\0';
    yylval.sval = strdup(label);
    return T_LABEL;
}
%}

%option noyywrap
%option yylineno
%option noinput
%option nounput

%%

"PUSH"      { return T_PUSH; }
"POP"       { return T_POP; }
"DUP"       { return T_DUP; }
"PEEKPRINT" { return T_PEEKPRINT; }
"HALT"      { return T_HALT; }
"ADD"       { return T_ADD; }
"SUB"       { return T_SUB; }
"MUL"       { return T_MUL; }
"DIV"       { return T_DIV; }
"CMP"       { return T_CMP; }
"AND"       { return T_AND; }
"OR"        { return T_OR; }
"XOR"       { return T_XOR; }
"NOT"       { return T_NOT; }
"SHL"       { return T_SHL; }
"SHR"       { return T_SHR; }
"JMP"       { return T_JMP; }
"JZ"        { return T_JZ; }
"JNZ"       { return T_JNZ; }
"STORE"     { return T_STORE; }
"LOAD"      { return T_LOAD; }
"CALL"      { return T_CALL; }
"RET"       { return T_RET; }

[a-zA-Z_][a-zA-Z0-9_]*:  { return handle_label(yytext); }
[a-zA-Z_][a-zA-Z0-9_]*   { yylval.sval = strdup(yytext); return T_ID; }

-?[0-9]+    { yylval.ival = atoi(yytext); return T_INTEGER; }
0x[0-9a-fA-F]+ { yylval.ival = (int)strtol(yytext, NULL, 0); return T_INTEGER; }

[ 	]+      { /* ignore whitespace */ }
\n          { return '\n'; }
;.*         { /* ignore comments */ }

.           { fprintf(stderr, "Line %d: Unexpected character '%c'\n", yylineno, *yytext); }

%%
