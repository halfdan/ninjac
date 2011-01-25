%{

/*
 * parser.y -- parser specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"
#include "scanner.h"
#include "parser.h"

Absyn *fileTree;
int arrayDims = 1;

static Absyn *p;

%}

%union {
  NoVal noVal;
  IntVal intVal;
  StringVal stringVal;
  Absyn *node;
}

%token	<noVal>		BREAK CASTTO CLASS DO ELSE EXTENDS
%token	<noVal>		IF INSTANCEOF LOCAL NEW NIL PUBLIC ASM
%token	<noVal>		RETURN SELF STATIC SUPER VOID WHILE
%token	<noVal>		LPAREN RPAREN LCURL RCURL LBRACK RBRACK
%token	<noVal>		ASGN COMMA SEMIC DOT
%token	<noVal>		LOGOR LOGAND LOGNOT
%token	<noVal>		EQ NE LT LE GT GE REQ RNE
%token	<noVal>		PLUS MINUS STAR SLASH PERCENT
%token	<intVal>	INTLIT
%token	<intVal>	BOOLEANLIT
%token	<intVal>	CHARLIT
%token	<stringVal>	STRINGLIT
%token	<stringVal>	ASMCODE
%token	<stringVal>	IDENT

%type	<node>		class_dec_list
%type	<node>		class_dec
%type	<node>		member_dec_list
%type	<node>		member_dec
%type	<node>		field_dec
%type	<node>		method_dec
%type	<node>		void
%type	<node>		type
%type	<node>		more_dims
%type	<node>		par_dec_list
%type	<node>		non_empty_par_dec_list
%type	<node>		par_dec
%type	<node>		var_dec_list
%type	<node>		var_dec
%type	<node>		stm_list
%type	<node>		stm
%type	<node>		empty_stm
%type	<node>		compound_stm
%type	<node>		assign_stm
%type	<node>		if_stm
%type	<node>		while_stm
%type	<node>		do_stm
%type	<node>		break_stm
%type	<node>		return_stm
%type	<node>		call_stm
%type	<node>		asm_stm
%type	<node>		arg_list
%type	<node>		non_empty_arg_list
%type	<node>		exp
%type	<node>		or_exp
%type	<node>		and_exp
%type	<node>		equ_exp
%type	<node>		rel_exp
%type	<node>		add_exp
%type	<node>		mul_exp
%type	<node>		unary_exp
%type	<node>		primary_exp
%type	<node>		new_obj_spec
%type	<node>		var

%start			source_file


%%


source_file		: class_dec_list
			  {
			    fileTree = newFile("<dummy>", 0, $1);
			  }
			;

class_dec_list		: /* empty */
			  {
			    $$ = emptyClsList();
			  }
			| class_dec class_dec_list
			  {
			    $$ = newClsList($1, $2);
			  }
			;

class_dec		: PUBLIC CLASS IDENT EXTENDS IDENT
			  LCURL member_dec_list RCURL
			  {
			    $$ = newClassDec($2.file, $2.line,
			                     TRUE, newSym($3.val),
			                     newSym($5.val), $7);
			  }
                        | PUBLIC CLASS IDENT EXTENDS NIL
			  LCURL member_dec_list RCURL
			  {
			    $$ = newClassDec($2.file, $2.line,
			                     TRUE, newSym($3.val),
			                     NULL, $7);
			  }
			| CLASS IDENT EXTENDS IDENT
			  LCURL member_dec_list RCURL
			  {
			    $$ = newClassDec($1.file, $1.line,
			                     FALSE, newSym($2.val),
			                     newSym($4.val), $6);
			  }
			;

member_dec_list		: /* empty */
			  {
			    $$ = emptyMbrList();
			  }
			| member_dec member_dec_list
			  {
			    $$ = newMbrList($1, $2);
			  }
			;

member_dec		: PUBLIC STATIC field_dec
			  {
			    $3->u.fieldDec.publ = TRUE;
			    $3->u.fieldDec.stat = TRUE;
			    $$ = $3;
			  }
			| PUBLIC STATIC method_dec
			  {
			    $3->u.methodDec.publ = TRUE;
			    $3->u.methodDec.stat = TRUE;
			    $$ = $3;
			  }
			| PUBLIC field_dec
			  {
			    $2->u.fieldDec.publ = TRUE;
			    $2->u.fieldDec.stat = FALSE;
			    $$ = $2;
			  }
			| PUBLIC method_dec
			  {
			    $2->u.methodDec.publ = TRUE;
			    $2->u.methodDec.stat = FALSE;
			    $$ = $2;
			  }
			| STATIC field_dec
			  {
			    $2->u.fieldDec.publ = FALSE;
			    $2->u.fieldDec.stat = TRUE;
			    $$ = $2;
			  }
			| STATIC method_dec
			  {
			    $2->u.methodDec.publ = FALSE;
			    $2->u.methodDec.stat = TRUE;
			    $$ = $2;
			  }
			| field_dec
			  {
			    $1->u.fieldDec.publ = FALSE;
			    $1->u.fieldDec.stat = FALSE;
			    $$ = $1;
			  }
			| method_dec
			  {
			    $1->u.methodDec.publ = FALSE;
			    $1->u.methodDec.stat = FALSE;
			    $$ = $1;
			  }
			;

field_dec		: type IDENT SEMIC
			  {
			    $$ = newFieldDec($2.file, $2.line,
			                     FALSE, FALSE,
			                     newSym($2.val), $1);
			  }
			;

method_dec		: void IDENT LPAREN par_dec_list RPAREN
			  LCURL var_dec_list stm_list RCURL
			  {
			    $$ = newMethodDec($2.file, $2.line,
			                      FALSE, FALSE, FALSE,
			                      newSym($2.val),
			                      $1, $4, $7, $8);
			  }
                        | IDENT LPAREN par_dec_list RPAREN
			  LCURL var_dec_list stm_list RCURL
			  {
			    $$ = newMethodDec($1.file, $1.line,
			                      FALSE, FALSE, TRUE,
			                      newSym($1.val),
			                      NULL, $3, $6, $7);
			  }
			| type IDENT LPAREN par_dec_list RPAREN
			  LCURL var_dec_list stm_list RCURL
			  {
			    $$ = newMethodDec($2.file, $2.line,
			                      FALSE, FALSE, FALSE,
			                      newSym($2.val),
			                      $1, $4, $7, $8);
			  }
			;

void			: VOID
			  {
			    $$ = newVoidTy($1.file, $1.line);
			  }
			;

type			: IDENT
			  {
			    $$ = newSimpleTy($1.file, $1.line,
			                     newSym($1.val));
			  }
			| IDENT LBRACK RBRACK more_dims
			  {                            
			    $$ = newArrayTy($2.file, $2.line,
                                             newSym($1.val), arrayDims);
                            arrayDims = 1; /* reset array dimension counter */
			  }
			;

more_dims		: /* empty */
			  {
			    $$ = NULL;
			  }
			| LBRACK RBRACK more_dims
			  {
                            arrayDims++;
			  }
			;

par_dec_list		: /* empty */
			  {
			    $$ = emptyParList();
			  }
			| non_empty_par_dec_list
			  {
			    $$ = $1;
			  }
			;

non_empty_par_dec_list	: par_dec
			  {
			    $$ = newParList($1, emptyParList());
			  }
			| par_dec COMMA non_empty_par_dec_list
			  {
			    $$ = newParList($1, $3);
			  }
			;

par_dec			: type IDENT
			  {
			    $$ = newParDec($2.file, $2.line,
			                   newSym($2.val), $1);
			  }
			;

var_dec_list		: /* empty */
			  {
			    $$ = emptyVarList();
			  }
			| var_dec var_dec_list
			  {
			    $$ = newVarList($1, $2);
			  }
			;

var_dec			: LOCAL type IDENT SEMIC
			  {
			    $$ = newVarDec($3.file, $3.line,
			                   newSym($3.val), $2);
			  }
			;

stm_list		: /* empty */
			  {
			    $$ = emptyStmList();
			  }
			| stm stm_list
			  {
			    $$ = newStmList($1, $2);
			  }
			;

stm			: empty_stm
			  {
			    $$ = $1;
			  }
			| compound_stm
			  {
			    $$ = $1;
			  }
			| assign_stm
			  {
			    $$ = $1;
			  }
			| if_stm
			  {
			    $$ = $1;
			  }
			| while_stm
			  {
			    $$ = $1;
			  }
			| do_stm
			  {
			    $$ = $1;
			  }
			| break_stm
			  {
			    $$ = $1;
			  }
			| return_stm
			  {
			    $$ = $1;
			  }
			| call_stm
			  {
			    $$ = $1;
			  }
                        | asm_stm
                          {
                            $$ = $1;
                          }
			;

empty_stm		: SEMIC
			  {
			    $$ = newEmptyStm($1.file, $1.line);
			  }
			;

compound_stm		: LCURL stm_list RCURL
			  {
			    $$ = newCompStm($1.file, $1.line, $2);
			  }
			;

assign_stm		: var ASGN exp SEMIC
			  {
			    $$ = newAssignStm($2.file, $2.line, $1, $3);
			  }
			;

if_stm			: IF LPAREN exp RPAREN stm
			  {
			    $$ = newIfStm1($1.file, $1.line, $3, $5);
			  }
			| IF LPAREN exp RPAREN stm ELSE stm
			  {
			    $$ = newIfStm2($1.file, $1.line, $3, $5, $7);
			  }
			;

while_stm		: WHILE LPAREN exp RPAREN stm
			  {
			    $$ = newWhileStm($1.file, $1.line, $3, $5);
			  }
			;

do_stm			: DO stm WHILE LPAREN exp RPAREN SEMIC
			  {
			    $$ = newDoStm($1.file, $1.line, $5, $2);
			  }
			;

break_stm		: BREAK SEMIC
			  {
			    $$ = newBreakStm($1.file, $1.line);
			  }
			;

return_stm		: RETURN SEMIC
			  {
			    $$ = newRetStm($1.file, $1.line);
			  }
			| RETURN exp SEMIC
			  {
			    $$ = newRetExpStm($1.file, $1.line, $2);
			  }
			;

call_stm		: IDENT LPAREN arg_list RPAREN SEMIC
			  {
			    $$ = newCallStm($1.file, $1.line,
			                    newSym($1.val),
			                    newSelfExp($1.file, $1.line),
			                    $3);
			  }
			| SUPER DOT IDENT LPAREN arg_list RPAREN SEMIC
			  {
			    $$ = newCallStm($3.file, $3.line,
			                    newSym($3.val),
			                    newSuperExp($1.file, $1.line),
			                    $5);
			  }
			| primary_exp DOT IDENT LPAREN arg_list RPAREN SEMIC
			  {
			    $$ = newCallStm($3.file, $3.line,
			                    newSym($3.val),
			                    $1,
			                    $5);
			  }
			;

asm_stm                 : ASM LCURL ASMCODE RCURL
                          {
                            $$ = newAsmStm($1.file, $1.line, $3.val);
                          }

arg_list		: /* empty */
			  {
			    $$ = emptyExpList();
			  }
			| non_empty_arg_list
			  {
			    $$ = $1;
			  }
			;

non_empty_arg_list	: exp
			  {
			    $$ = newExpList($1, emptyExpList());
			  }
			| exp COMMA non_empty_arg_list
			  {
			    $$ = newExpList($1, $3);
			  }
			;

exp			: or_exp
			  {
			    $$ = $1;
			  }
			;

or_exp			: and_exp
			  {
			    $$ = $1;
			  }
			| or_exp LOGOR and_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_LOR, $1, $3);
			  }
			;

and_exp			: equ_exp
			  {
			    $$ = $1;
			  }
			| and_exp LOGAND equ_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_LAND, $1, $3);
			  }
			;

equ_exp			: rel_exp
			  {
			    $$ = $1;
			  }
			| equ_exp EQ rel_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_EQ, $1, $3);
			  }
			| equ_exp NE rel_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_NE, $1, $3);
			  }
                        | equ_exp REQ rel_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_REQ, $1, $3);
			  }
			| equ_exp RNE rel_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_RNE, $1, $3);
			  }
			;

rel_exp			: add_exp
			  {
			    $$ = $1;
			  }
			| rel_exp LT add_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_LT, $1, $3);
			  }
			| rel_exp LE add_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_LE, $1, $3);
			  }
			| rel_exp GT add_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_GT, $1, $3);
			  }
			| rel_exp GE add_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_GE, $1, $3);
			  }
			| rel_exp INSTANCEOF type
			  {
			    $$ = newInstofExp($2.file, $2.line, $1, $3);
			  }
			;

add_exp			: mul_exp
			  {
			    $$ = $1;
			  }
			| add_exp PLUS mul_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_ADD, $1, $3);
			  }
			| add_exp MINUS mul_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_SUB, $1, $3);
			  }
			;

mul_exp			: unary_exp
			  {
			    $$ = $1;
			  }
			| mul_exp STAR unary_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_MUL, $1, $3);
			  }
			| mul_exp SLASH unary_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_DIV, $1, $3);
			  }
			| mul_exp PERCENT unary_exp
			  {
			    $$ = newBinopExp($2.file, $2.line,
			                     ABSYN_BINOP_MOD, $1, $3);
			  }
			;

unary_exp		: primary_exp
			  {
			    $$ = $1;
			  }
			| PLUS unary_exp
			  {
			    $$ = newUnopExp($1.file, $1.line,
			                    ABSYN_UNOP_PLUS, $2);
			  }
			| MINUS unary_exp
			  {
			    $$ = newUnopExp($1.file, $1.line,
			                    ABSYN_UNOP_MINUS, $2);
			  }
			| LOGNOT unary_exp
			  {
			    $$ = newUnopExp($1.file, $1.line,
			                    ABSYN_UNOP_LNOT, $2);
			  }
			| LPAREN CASTTO type RPAREN unary_exp
			  {
			    $$ = newCastExp($1.file, $1.line, $5, $3);
			  }
			;

primary_exp		: NIL
			  {
			    $$ = newNilExp($1.file, $1.line);
			  }
			| INTLIT
			  {
			    $$ = newIntExp($1.file, $1.line, $1.val);
			  }
			| BOOLEANLIT
			  {
			    $$ = newBoolExp($1.file, $1.line, $1.val);
			  }
			| CHARLIT
			  {
			    $$ = newCharExp($1.file, $1.line, $1.val);
			  }
			| STRINGLIT
			  {
			    $$ = newStringExp($1.file, $1.line, $1.val);
			  }
			| LPAREN exp RPAREN
			  {
			    $$ = $2;
			  }
			| SELF
			  {
			    $$ = newSelfExp($1.file, $1.line);
			  }
			| var
			  {
			    $$ = newVarExp($1->file, $1->line, $1);
			  }
			| IDENT LPAREN arg_list RPAREN
			  {
			    $$ = newCallExp($1.file, $1.line,
			                    newSym($1.val),
			                    newSelfExp($1.file, $1.line),
			                    $3);
			  }
			| SUPER DOT IDENT LPAREN arg_list RPAREN
			  {
			    $$ = newCallExp($3.file, $3.line,
			                    newSym($3.val),
			                    newSuperExp($1.file, $1.line),
			                    $5);
			  }
			| primary_exp DOT IDENT LPAREN arg_list RPAREN
			  {
			    $$ = newCallExp($3.file, $3.line,
			                    newSym($3.val),
			                    $1,
			                    $5);
			  }
			| NEW new_obj_spec
			  {			    
                            $$ = $2
			  }
			;

new_obj_spec		: IDENT LPAREN arg_list RPAREN
			  {
			    $$ = newNewExp($1.file, $1.line, newSym($1.val), $3)
			  }
			| IDENT LBRACK exp RBRACK more_dims
			  {
                            $$ = newNewArrayExp($2.file, $2.line,
                                    newSym($1.val), $3, arrayDims);
                            arrayDims = 1; /* reset array dimension counter */
			  }
			;

var			: IDENT
			  {
			    $$ = newSimpleVar($1.file, $1.line,
			                      newSym($1.val));
			  }
			| primary_exp LBRACK exp RBRACK
			  {
			    $$ = newArrayVar($2.file, $2.line,
			                     $1, $3);
			  }
			| primary_exp DOT IDENT
			  {
			    $$ = newMemberVar($3.file, $3.line,
			                      newSym($3.val), $1);
			  }
			;


%%


void yyerror(char *msg) {
  error("%s in line %d", msg, yylval.noVal.line);
}
