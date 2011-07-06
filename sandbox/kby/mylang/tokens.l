%{
#include <string>
#include <stack>
#include "AstNode.h"
#include "parser.hpp"
#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval.token = t)
extern "C" int yywrap() { }

int yyerror(char const * s )
{
    printf("ERROR %s in %s at line %d, col ??\n", s, yytext, yylineno);
}

std::string g_str;

/* globals to track current indentation */
int current_line_indent = 0;   /* indentation of the current line */
int indent_level = 0;          /* indentation level passed to the parser */
std::stack<int> curr_indents;
int first_time = 1 ;

%}


%option yylineno

%x indent 
%s normal 
%x str qstr
%x comment

%%
            if( first_time ) {
                first_time = 0;
                curr_indents.push(0);
                BEGIN indent;
            }

\@\{             BEGIN(comment);
<comment>\@\}    BEGIN(normal);
<comment>.*  ;/* eat everything */
<comment>\n  ;/* eat everything */

\"      g_str = ""; BEGIN(str);
'       g_str = ""; BEGIN(qstr);

<str>\" {   BEGIN(normal);
            yylval.string = new std::string(g_str);
            return TSTR;
        }
<qstr>' {   BEGIN(normal);
            yylval.string = new std::string(g_str);
            return TSTR;
        }

<str,qstr>\\n   g_str += "\n";
<str,qstr>\\t   g_str += "\t";
<str,qstr>\\r   g_str += "\r";
<str>\\\"       g_str += "\"";
<qstr>\\\'      g_str += "'";

<str,qstr>\\(.|\n)  g_str += yytext[1];

<str>[^\\\"]+  g_str += std::string(yytext);
<qstr>[^\\']+  g_str += std::string(yytext);

<indent>" "      { current_line_indent++; }
<indent>"\t"     { current_line_indent = (current_line_indent + 8) & ~7; }
<indent>"\n"     { current_line_indent = 0; /*ignoring blank line */ }
<indent>.        {
                   unput(*yytext);
                   if (current_line_indent > curr_indents.top()) {
                       curr_indents.push(current_line_indent);
                       return TOKEN(INDENT);
                   } else if (current_line_indent < curr_indents.top()) {
                       curr_indents.pop();
                       return TOKEN(UNINDENT);
                   } else {
                       BEGIN(normal);
                   }
                 }

<normal>"\n"     { current_line_indent = 0; BEGIN( indent); }
<<EOF>>          { if( curr_indents.size() > 1 ) {
                        curr_indents.pop();
                        return TOKEN(UNINDENT);
                    }
                    yyterminate();
                 }
                  

\;.*                    /* comments one line til nl */
"if"                    return TOKEN(TIF);
"else"                  return TOKEN(TELSE);
"return"[ \t]*[\n]      return TOKEN(TRETURN_SIMPLE);
"return"                return TOKEN(TRETURN);
"not"                   return TOKEN(TNOT);
"and"                   return TOKEN(TAND);
"or"                    return TOKEN(TOR);
"def"                   return TOKEN(TDEF);
"var"                   return TOKEN(TVAR);
"while"                 return TOKEN(TWHILE);
"is"                    return TOKEN(IS);
#[tf]/[ \t\n),]          SAVE_TOKEN; return TBOOL;
[ \t\n]                 ;
[a-zA-Z_][a-zA-Z0-9_&%\$\?\-]*  SAVE_TOKEN; return TIDENTIFIER;
-?[0-9]+\.[0-9]*        SAVE_TOKEN; return TDOUBLE;
-?[0-9]+                SAVE_TOKEN; return TINTEGER;
"="                     return TOKEN(TEQUAL);
"=="                    return TOKEN(TCEQ);
"!="                    return TOKEN(TCNE);
"<"                     return TOKEN(TCLT);
"<="                    return TOKEN(TCLE);
">"                     return TOKEN(TCGT);
">="                    return TOKEN(TCGE);
"("                     return TOKEN(TLPAREN);
")"                     return TOKEN(TRPAREN);
"."                     return TOKEN(TDOT);
","                     return TOKEN(TCOMMA);
"+"                     return TOKEN(TPLUS);
"-"                     return TOKEN(TMINUS);
"*"                     return TOKEN(TMUL);
"/"                     return TOKEN(TDIV);
":"                     return TOKEN(TCOLON);
.                       printf("line %d, len %d Unknown token %s !\n", yylineno, yyleng, yytext); yyterminate();

%%
