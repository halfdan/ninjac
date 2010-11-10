%{

/*
 * scanner.l -- scanner specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"
#include "scanner.h"
#include "parser.tab.h"

static char *fileName;
static int lineNumber;
static int len;
static char *p, *q, *r;

boolean yyInit(char *srcFileName) {
  yyin = fopen(srcFileName, "r");
  if (yyin == NULL) {
    yyin = stdin;
    return FALSE;
  }
  fileName = allocate(strlen(srcFileName) + 1);
  strcpy(fileName, srcFileName);
  lineNumber = 1;
  return TRUE;
}

%}

L			[A-Za-z_]
D			[0-9]
H			[0-9A-Fa-f]
ID			{L}({L}|{D})*
DECNUM			{D}+
HEXNUM			0x{H}+
CHR			\'([^\'\\\n]|\\.)\'
STR			\"([^\"\\\n]|\\.)*\"


%%


\/\/.*			{
			  /* comment: nothing returned */
			}

[\ \t\r]+		{
			  /* white space: nothing returned */
			}

\n			{
			  /* newline: nothing returned */
			  lineNumber++;
			}

break			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return BREAK;
			}

castto			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return CASTTO;
			}

class			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return CLASS;
			}

do			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return DO;
			}

else			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return ELSE;
			}

extends			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return EXTENDS;
			}

if			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return IF;
			}

import			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return IMPORT;
			}

instanceof		{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return INSTANCEOF;
			}

local			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LOCAL;
			}

new			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return NEW;
			}

nil			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return NIL;
			}

package			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return PACKAGE;
			}

public			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return PUBLIC;
			}

return			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return RETURN;
			}

self			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return SELF;
			}

static			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return STATIC;
			}

super			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return SUPER;
			}

void			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return VOID;
			}

while			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return WHILE;
			}

\(			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LPAREN;
			}

\)			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return RPAREN;
			}

\{			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LCURL;
			}

\}			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return RCURL;
			}

\[			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LBRACK;
			}

\]			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return RBRACK;
			}

\=			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return ASGN;
			}

\,			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return COMMA;
			}

\;			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return SEMIC;
			}

\.			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return DOT;
			}

\|\|			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LOGOR;
			}

\&\&			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LOGAND;
			}

\!			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LOGNOT;
			}

\=\=			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return EQ;
			}

\!\=			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return NE;
			}

\<			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LT;
			}

\<\=			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return LE;
			}

\>			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return GT;
			}

\>\=			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return GE;
			}

\+			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return PLUS;
			}

\-			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return MINUS;
			}

\*			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return STAR;
			}

\/			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return SLASH;
			}

\%			{
			  yylval.noVal.file = fileName;
			  yylval.noVal.line = lineNumber;
			  return PERCENT;
			}

{DECNUM}		{
			  yylval.intVal.file = fileName;
			  yylval.intVal.line = lineNumber;
			  yylval.intVal.val = strtoul(yytext, NULL, 10);
			  return INTLIT;
			}

{HEXNUM}		{
			  yylval.intVal.file = fileName;
			  yylval.intVal.line = lineNumber;
			  yylval.intVal.val = strtoul(yytext, NULL, 16);
			  return INTLIT;
			}

false			{
			  yylval.intVal.file = fileName;
			  yylval.intVal.line = lineNumber;
			  yylval.intVal.val = 0;
			  return BOOLEANLIT;
			}

true			{
			  yylval.intVal.file = fileName;
			  yylval.intVal.line = lineNumber;
			  yylval.intVal.val = 1;
			  return BOOLEANLIT;
			}

{CHR}			{
			  if (yytext[1] == '\\') {
			    switch (yytext[2]) {
			      case 'n':
			        yytext[1] = '\n';
			        break;
			      case 'r':
			        yytext[1] = '\r';
			        break;
			      case 't':
			        yytext[1] = '\t';
			        break;
			      default:
			        yytext[1] = yytext[2];
			        break;
			    }
			  }
			  yylval.intVal.file = fileName;
			  yylval.intVal.line = lineNumber;
			  yylval.intVal.val = yytext[1];
			  return CHARLIT;
			}

{STR}			{
			  len = strlen(yytext);
			  yytext[len - 1] = '\0';
			  len--;
			  p = yytext;
			  q = p + 1;
			  while (1) {
			    *p = *q;
			    if (*p == '\0') {
			      break;
			    }
			    p++;
			    q++;
			  }
			  len--;
			  r = yytext;
			  while (*r != '\0') {
			    if (*r == '\\') {
			      p = r;
			      q = p + 1;
			      switch (*q) {
			        case 'n':
			          *p = '\n';
			          break;
			        case 'r':
			          *p = '\r';
			          break;
			        case 't':
			          *p = '\t';
			          break;
			        default:
			          *p = *q;
			          break;
			      }
			      p++;
			      q++;
			      while (1) {
			        *p = *q;
			        if (*p == '\0') {
			          break;
			        }
			        p++;
			        q++;
			      }
			      len--;
			    }
			    r++;
			  }
			  yylval.stringVal.file = fileName;
			  yylval.stringVal.line = lineNumber;
			  yylval.stringVal.val = allocate(len + 1);
			  strcpy(yylval.stringVal.val, yytext);
			  return STRINGLIT;
			}

{ID}			{
			  len = strlen(yytext);
			  yylval.stringVal.file = fileName;
			  yylval.stringVal.line = lineNumber;
			  yylval.stringVal.val = allocate(len + 1);
			  strcpy(yylval.stringVal.val, yytext);
			  return IDENT;
			}

.			{
			  if (yytext[0] == '\'') {
			    error("malformed character literal in file %s, "
			          "line %d", fileName, lineNumber);
			  }
			  if (yytext[0] == '\"') {
			    error("malformed string literal in file %s, "
			          "line %d", fileName, lineNumber);
			  }
			  error("illegal character 0x%02x in file %s, "
			        "line %d", (unsigned char) yytext[0],
			        fileName, lineNumber);
			}


%%


int yywrap(void) {
  if (yyin != stdin) {
    fclose(yyin);
    yyin = stdin;
  }
  return 1;
}


void showToken(int token) {
  char *p;

  printf("TOKEN = ");
  switch (token) {
    case 0:
      printf("-- EOF --");
      break;
    case BREAK:
      printf("BREAK in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case CASTTO:
      printf("CASTTO in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case CLASS:
      printf("CLASS in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case DO:
      printf("DO in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case ELSE:
      printf("ELSE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case EXTENDS:
      printf("EXTENDS in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case IF:
      printf("IF in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case IMPORT:
      printf("IMPORT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case INSTANCEOF:
      printf("INSTANCEOF in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LOCAL:
      printf("LOCAL in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case NEW:
      printf("NEW in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case NIL:
      printf("NIL in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case PACKAGE:
      printf("PACKAGE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case PUBLIC:
      printf("PUBLIC in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case RETURN:
      printf("RETURN in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case SELF:
      printf("SELF in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case STATIC:
      printf("STATIC in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case SUPER:
      printf("SUPER in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case VOID:
      printf("VOID in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case WHILE:
      printf("WHILE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LPAREN:
      printf("LPAREN in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case RPAREN:
      printf("RPAREN in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LCURL:
      printf("LCURL in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case RCURL:
      printf("RCURL in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LBRACK:
      printf("LBRACK in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case RBRACK:
      printf("RBRACK in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case ASGN:
      printf("ASGN in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case COMMA:
      printf("COMMA in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case SEMIC:
      printf("SEMIC in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case DOT:
      printf("DOT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LOGOR:
      printf("LOGOR in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LOGAND:
      printf("LOGAND in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LOGNOT:
      printf("LOGNOT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case EQ:
      printf("EQ in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case NE:
      printf("NE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LT:
      printf("LT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case LE:
      printf("LE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case GT:
      printf("GT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case GE:
      printf("GE in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case PLUS:
      printf("PLUS in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case MINUS:
      printf("MINUS in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case STAR:
      printf("STAR in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case SLASH:
      printf("SLASH in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case PERCENT:
      printf("PERCENT in file %s, line %d",
             yylval.noVal.file, yylval.noVal.line);
      break;
    case INTLIT:
      printf("INTLIT in file %s, line %d, value = %d (0x%08X)",
             yylval.intVal.file, yylval.intVal.line,
             yylval.intVal.val, yylval.intVal.val);
      break;
    case BOOLEANLIT:
      printf("BOOLEANLIT in file %s, line %d, value = %s",
             yylval.intVal.file, yylval.intVal.line,
             yylval.intVal.val ? "true" : "false");
      break;
    case CHARLIT:
      printf("CHARLIT in file %s, line %d, value = '",
             yylval.intVal.file, yylval.intVal.line);
      switch (yylval.intVal.val) {
        case '\n':
          printf("\\n");
          break;
        case '\r':
          printf("\\r");
          break;
        case '\t':
          printf("\\t");
          break;
        case '\'':
          printf("\\\'");
          break;
        case '\"':
          printf("\\\"");
          break;
        case '\\':
          printf("\\\\");
          break;
        default:
          printf("%c", yylval.intVal.val);
          break;
      }
      printf("'");
      break;
    case STRINGLIT:
      printf("STRINGLIT in file %s, line %d, value = \"",
             yylval.stringVal.file, yylval.stringVal.line);
      p = yylval.stringVal.val;
      while (*p != '\0') {
        switch (*p) {
          case '\n':
            printf("\\n");
            break;
          case '\r':
            printf("\\r");
            break;
          case '\t':
            printf("\\t");
            break;
          case '\'':
            printf("\\\'");
            break;
          case '\"':
            printf("\\\"");
            break;
          case '\\':
            printf("\\\\");
            break;
          default:
            printf("%c", *p);
            break;
        }
        p++;
      }
      printf("\"");
      break;
    case IDENT:
      printf("IDENT in file %s, line %d, value = \"%s\"",
             yylval.stringVal.file, yylval.stringVal.line,
             yylval.stringVal.val);
      break;
    default:
      /* this should never happen */
      error("unknown token %d in showToken", token);
  }
  printf("\n");
}
