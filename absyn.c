/*
 * absyn.c -- abstract syntax
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"


/**************************************************************/


Absyn *newFile(char *file, int line,
               Absyn *classes) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_FILE;
  node->file = file;
  node->line = line;
  node->u.file.classes = classes;
  return node;
}


Absyn *newClassDec(char *file, int line,
                   boolean publ, Sym *name,
                   Sym *superClass, Absyn *members) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CLASSDEC;
  node->file = file;
  node->line = line;
  node->u.classDec.publ = publ;
  node->u.classDec.name = name;
  node->u.classDec.superClass = superClass;
  node->u.classDec.members = members;
  return node;
}


Absyn *newFieldDec(char *file, int line,
                   boolean publ, boolean stat, Sym *name, Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_FIELDDEC;
  node->file = file;
  node->line = line;
  node->u.fieldDec.publ = publ;
  node->u.fieldDec.stat = stat;
  node->u.fieldDec.name = name;
  node->u.fieldDec.type = type;
  return node;
}


Absyn *newMethodDec(char *file, int line,
                    boolean publ, boolean stat, Sym *name, Absyn *retType,
                    Absyn *params, Absyn *locals, Absyn *stms) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_METHODDEC;
  node->file = file;
  node->line = line;
  node->u.methodDec.publ = publ;
  node->u.methodDec.stat = stat;
  node->u.methodDec.name = name;
  node->u.methodDec.retType = retType;
  node->u.methodDec.params = params;
  node->u.methodDec.locals = locals;
  node->u.methodDec.stms = stms;
  return node;
}


Absyn *newVoidTy(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VOIDTY;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newSimpleTy(char *file, int line,
                   Sym *name) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_SIMPLETY;
  node->file = file;
  node->line = line;
  node->u.simpleTy.name = name;
  return node;
}


Absyn *newArrayTy(char *file, int line,
                  Absyn *size, Absyn *baseType) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ARRAYTY;
  node->file = file;
  node->line = line;
  node->u.arrayTy.size = size;
  node->u.arrayTy.baseType = baseType;
  return node;
}


Absyn *newParDec(char *file, int line,
                 Sym *name, Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_PARDEC;
  node->file = file;
  node->line = line;
  node->u.parDec.name = name;
  node->u.parDec.type = type;
  return node;
}


Absyn *newVarDec(char *file, int line,
                 Sym *name, Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VARDEC;
  node->file = file;
  node->line = line;
  node->u.varDec.name = name;
  node->u.varDec.type = type;
  return node;
}


Absyn *newEmptyStm(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EMPTYSTM;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newCompStm(char *file, int line,
                  Absyn *stms) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_COMPSTM;
  node->file = file;
  node->line = line;
  node->u.compStm.stms = stms;
  return node;
}


Absyn *newAssignStm(char *file, int line,
                    Absyn *var, Absyn *exp) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ASSIGNSTM;
  node->file = file;
  node->line = line;
  node->u.assignStm.var = var;
  node->u.assignStm.exp = exp;
  return node;
}


Absyn *newIfStm1(char *file, int line,
                 Absyn *test, Absyn *thenPart) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_IFSTM1;
  node->file = file;
  node->line = line;
  node->u.ifStm1.test = test;
  node->u.ifStm1.thenPart = thenPart;
  return node;
}


Absyn *newIfStm2(char *file, int line,
                 Absyn *test, Absyn *thenPart, Absyn *elsePart) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_IFSTM2;
  node->file = file;
  node->line = line;
  node->u.ifStm2.test = test;
  node->u.ifStm2.thenPart = thenPart;
  node->u.ifStm2.elsePart = elsePart;
  return node;
}


Absyn *newWhileStm(char *file, int line,
                   Absyn *test, Absyn *body) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_WHILESTM;
  node->file = file;
  node->line = line;
  node->u.whileStm.test = test;
  node->u.whileStm.body = body;
  return node;
}


Absyn *newDoStm(char *file, int line,
                Absyn *test, Absyn *body) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_DOSTM;
  node->file = file;
  node->line = line;
  node->u.doStm.test = test;
  node->u.doStm.body = body;
  return node;
}


Absyn *newBreakStm(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_BREAKSTM;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newRetStm(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_RETSTM;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newRetExpStm(char *file, int line,
                    Absyn *exp) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_RETEXPSTM;
  node->file = file;
  node->line = line;
  node->u.retExpStm.exp = exp;
  return node;
}


Absyn *newCallStm(char *file, int line,
                  Sym *name, Absyn *rcvr, Absyn *args) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CALLSTM;
  node->file = file;
  node->line = line;
  node->u.callStm.name = name;
  node->u.callStm.rcvr = rcvr;
  node->u.callStm.args = args;
  return node;
}


Absyn *newBinopExp(char *file, int line,
                   int op, Absyn *left, Absyn *right) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_BINOPEXP;
  node->file = file;
  node->line = line;
  node->u.binopExp.op = op;
  node->u.binopExp.left = left;
  node->u.binopExp.right = right;
  return node;
}


Absyn *newUnopExp(char *file, int line,
                  int op, Absyn *right) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_UNOPEXP;
  node->file = file;
  node->line = line;
  node->u.unopExp.op = op;
  node->u.unopExp.right = right;
  return node;
}


Absyn *newInstofExp(char *file, int line,
                    Absyn *exp, Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_INSTOFEXP;
  node->file = file;
  node->line = line;
  node->u.instofExp.exp = exp;
  node->u.instofExp.type = type;
  return node;
}


Absyn *newCastExp(char *file, int line,
                  Absyn *exp, Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CASTEXP;
  node->file = file;
  node->line = line;
  node->u.castExp.exp = exp;
  node->u.castExp.type = type;
  return node;
}


Absyn *newNilExp(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NILEXP;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newIntExp(char *file, int line,
                 int value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_INTEXP;
  node->file = file;
  node->line = line;
  node->u.intExp.value = value;
  return node;
}


Absyn *newBoolExp(char *file, int line,
                  int value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_BOOLEXP;
  node->file = file;
  node->line = line;
  node->u.boolExp.value = value;
  return node;
}


Absyn *newCharExp(char *file, int line,
                  char value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CHAREXP;
  node->file = file;
  node->line = line;
  node->u.charExp.value = value;
  return node;
}


Absyn *newStringExp(char *file, int line,
                    char *value) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STRINGEXP;
  node->file = file;
  node->line = line;
  node->u.stringExp.value = value;
  return node;
}


Absyn *newSelfExp(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_SELFEXP;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newSuperExp(char *file, int line) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_SUPEREXP;
  node->file = file;
  node->line = line;
  return node;
}


Absyn *newVarExp(char *file, int line,
                 Absyn *var) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VAREXP;
  node->file = file;
  node->line = line;
  node->u.varExp.var = var;
  return node;
}


Absyn *newCallExp(char *file, int line,
                  Sym *name, Absyn *rcvr, Absyn *args) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CALLEXP;
  node->file = file;
  node->line = line;
  node->u.callExp.name = name;
  node->u.callExp.rcvr = rcvr;
  node->u.callExp.args = args;
  return node;
}


Absyn *newNewExp(char *file, int line,
                 Sym *name, Absyn *args) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NEWEXP;
  node->file = file;
  node->line = line;
  node->u.newExp.name = name;
  node->u.newExp.args = args;
  return node;
}

Absyn *newNewArrayExp(char *file, int line,
                 Absyn *type) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_NEWARRAYEXP;
  node->file = file;
  node->line = line;
  node->u.newArrayExp.type = type;
  return node;
}


Absyn *newSimpleVar(char *file, int line,
                    Sym *name) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_SIMPLEVAR;
  node->file = file;
  node->line = line;
  node->u.simpleVar.name = name;
  return node;
}


Absyn *newArrayVar(char *file, int line,
                   Absyn *var, Absyn *index) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_ARRAYVAR;
  node->file = file;
  node->line = line;
  node->u.arrayVar.var = var;
  node->u.arrayVar.index = index;
  return node;
}


Absyn *newMemberVar(char *file, int line,
                    Sym *name, Absyn *object) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_MEMBERVAR;
  node->file = file;
  node->line = line;
  node->u.memberVar.name = name;
  node->u.memberVar.object = object;
  return node;
}


Absyn *emptyClsList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CLSLIST;
  node->file = NULL;
  node->line = -1;
  node->u.clsList.isEmpty = TRUE;
  return node;
}


Absyn *newClsList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_CLSLIST;
  node->file = NULL;
  node->line = -1;
  node->u.clsList.isEmpty = FALSE;
  node->u.clsList.head = head;
  node->u.clsList.tail = tail;
  return node;
}


Absyn *emptyMbrList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_MBRLIST;
  node->file = NULL;
  node->line = -1;
  node->u.mbrList.isEmpty = TRUE;
  return node;
}


Absyn *newMbrList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_MBRLIST;
  node->file = NULL;
  node->line = -1;
  node->u.mbrList.isEmpty = FALSE;
  node->u.mbrList.head = head;
  node->u.mbrList.tail = tail;
  return node;
}


Absyn *emptyParList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_PARLIST;
  node->file = NULL;
  node->line = -1;
  node->u.parList.isEmpty = TRUE;
  return node;
}


Absyn *newParList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_PARLIST;
  node->file = NULL;
  node->line = -1;
  node->u.parList.isEmpty = FALSE;
  node->u.parList.head = head;
  node->u.parList.tail = tail;
  return node;
}


Absyn *emptyVarList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VARLIST;
  node->file = NULL;
  node->line = -1;
  node->u.varList.isEmpty = TRUE;
  return node;
}


Absyn *newVarList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_VARLIST;
  node->file = NULL;
  node->line = -1;
  node->u.varList.isEmpty = FALSE;
  node->u.varList.head = head;
  node->u.varList.tail = tail;
  return node;
}


Absyn *emptyStmList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STMLIST;
  node->file = NULL;
  node->line = -1;
  node->u.stmList.isEmpty = TRUE;
  return node;
}


Absyn *newStmList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_STMLIST;
  node->file = NULL;
  node->line = -1;
  node->u.stmList.isEmpty = FALSE;
  node->u.stmList.head = head;
  node->u.stmList.tail = tail;
  return node;
}


Absyn *emptyExpList(void) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EXPLIST;
  node->file = NULL;
  node->line = -1;
  node->u.expList.isEmpty = TRUE;
  return node;
}


Absyn *newExpList(Absyn *head, Absyn *tail) {
  Absyn *node;

  node = (Absyn *) allocate(sizeof(Absyn));
  node->type = ABSYN_EXPLIST;
  node->file = NULL;
  node->line = -1;
  node->u.expList.isEmpty = FALSE;
  node->u.expList.head = head;
  node->u.expList.tail = tail;
  return node;
}


/**************************************************************/


static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}


static void say(char *s) {
  printf("%s", s);
}


static void sayInt(int i) {
  printf("%d", i);
}


static void sayBool(int b) {
  if (b) {
    printf("true");
  } else {
    printf("false");
  }
}


static void sayChar(char c) {
  printf("'");
  switch (c) {
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
      printf("%c", c);
      break;
  }
  printf("'");
}


static void sayString(char *s) {
  printf("\"");
  while (*s != '\0') {
    switch (*s) {
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
        printf("%c", *s);
        break;
    }
    s++;
  }
  printf("\"");
}


static void showNode(Absyn *node, int indent);


static void showFile(Absyn *node, int n) {
  indent(n);
  say("File(\n");
  showNode(node->u.file.classes, n + 1);
  say(")");
}


static void showClassDec(Absyn *node, int n) {
  indent(n);
  say("ClassDec(\n");
  indent(n + 1);
  sayBool(node->u.classDec.publ);
  say(",\n");
  indent(n + 1);
  say(symToString(node->u.classDec.name));
  say(",\n");
  indent(n + 1);
  say(symToString(node->u.classDec.superClass));
  say(",\n");
  showNode(node->u.classDec.members, n + 1);
  say(")");
}


static void showFieldDec(Absyn *node, int n) {
  indent(n);
  say("FieldDec(\n");
  indent(n + 1);
  sayBool(node->u.fieldDec.publ);
  say(",\n");
  indent(n + 1);
  sayBool(node->u.fieldDec.stat);
  say(",\n");
  indent(n + 1);
  say(symToString(node->u.fieldDec.name));
  say(",\n");
  showNode(node->u.fieldDec.type, n + 1);
  say(")");
}


static void showMethodDec(Absyn *node, int n) {
  indent(n);
  say("MethodDec(\n");
  indent(n + 1);
  sayBool(node->u.methodDec.publ);
  say(",\n");
  indent(n + 1);
  sayBool(node->u.methodDec.stat);
  say(",\n");
  indent(n + 1);
  say(symToString(node->u.methodDec.name));
  say(",\n");
  showNode(node->u.methodDec.retType, n + 1);
  say(",\n");
  showNode(node->u.methodDec.params, n + 1);
  say(",\n");
  showNode(node->u.methodDec.locals, n + 1);
  say(",\n");
  showNode(node->u.methodDec.stms, n + 1);
  say(")");
}


static void showVoidTy(Absyn *node, int n) {
  indent(n);
  say("VoidTy()");
}


static void showSimpleTy(Absyn *node, int n) {
  indent(n);
  say("SimpleTy(");
  say(symToString(node->u.simpleTy.name));
  say(")");
}


static void showArrayTy(Absyn *node, int n) {
  indent(n);
  say("ArrayTy(\n");
  if (node->u.arrayTy.size == NULL) {
    /* this is the normal case */
    indent(n + 1);
    say("<no size>");
  } else {
    /* this can only happen in newExp */
    showNode(node->u.arrayTy.size, n + 1);
  }
  say(",\n");
  showNode(node->u.arrayTy.baseType, n + 1);
  say(")");
}


static void showParDec(Absyn *node, int n) {
  indent(n);
  say("ParDec(\n");
  indent(n + 1);
  say(symToString(node->u.parDec.name));
  say(",\n");
  showNode(node->u.parDec.type, n + 1);
  say(")");
}


static void showVarDec(Absyn *node, int n) {
  indent(n);
  say("VarDec(\n");
  indent(n + 1);
  say(symToString(node->u.varDec.name));
  say(",\n");
  showNode(node->u.varDec.type, n + 1);
  say(")");
}


static void showEmptyStm(Absyn *node, int n) {
  indent(n);
  say("EmptyStm()");
}


static void showCompStm(Absyn *node, int n) {
  indent(n);
  say("CompStm(\n");
  showNode(node->u.compStm.stms, n + 1);
  say(")");
}


static void showAssignStm(Absyn *node, int n) {
  indent(n);
  say("AssignStm(\n");
  showNode(node->u.assignStm.var, n + 1);
  say(",\n");
  showNode(node->u.assignStm.exp, n + 1);
  say(")");
}


static void showIfStm1(Absyn *node, int n) {
  indent(n);
  say("IfStm1(\n");
  showNode(node->u.ifStm1.test, n + 1);
  say(",\n");
  showNode(node->u.ifStm1.thenPart, n + 1);
  say(")");
}


static void showIfStm2(Absyn *node, int n) {
  indent(n);
  say("IfStm2(\n");
  showNode(node->u.ifStm2.test, n + 1);
  say(",\n");
  showNode(node->u.ifStm2.thenPart, n + 1);
  say(",\n");
  showNode(node->u.ifStm2.elsePart, n + 1);
  say(")");
}


static void showWhileStm(Absyn *node, int n) {
  indent(n);
  say("WhileStm(\n");
  showNode(node->u.whileStm.test, n + 1);
  say(",\n");
  showNode(node->u.whileStm.body, n + 1);
  say(")");
}


static void showDoStm(Absyn *node, int n) {
  indent(n);
  say("DoStm(\n");
  showNode(node->u.doStm.test, n + 1);
  say(",\n");
  showNode(node->u.doStm.body, n + 1);
  say(")");
}


static void showBreakStm(Absyn *node, int n) {
  indent(n);
  say("BreakStm()");
}


static void showRetStm(Absyn *node, int n) {
  indent(n);
  say("RetStm()");
}


static void showRetExpStm(Absyn *node, int n) {
  indent(n);
  say("RetExpStm(\n");
  showNode(node->u.retExpStm.exp, n + 1);
  say(")");
}


static void showCallStm(Absyn *node, int n) {
  indent(n);
  say("CallStm(\n");
  indent(n + 1);
  say(symToString(node->u.callStm.name));
  say(",\n");
  showNode(node->u.callStm.rcvr, n + 1);
  say(",\n");
  showNode(node->u.callStm.args, n + 1);
  say(")");
}


static void showBinopExp(Absyn *node, int n) {
  indent(n);
  say("BinopExp(\n");
  indent(n + 1);
  switch (node->u.binopExp.op) {
    case ABSYN_BINOP_LOR:
      say("LOR");
      break;
    case ABSYN_BINOP_LAND:
      say("LAND");
      break;
    case ABSYN_BINOP_EQ:
      say("EQ");
      break;
    case ABSYN_BINOP_NE:
      say("NE");
      break;
    case ABSYN_BINOP_LT:
      say("LT");
      break;
    case ABSYN_BINOP_LE:
      say("LE");
      break;
    case ABSYN_BINOP_GT:
      say("GT");
      break;
    case ABSYN_BINOP_GE:
      say("GE");
      break;
    case ABSYN_BINOP_ADD:
      say("ADD");
      break;
    case ABSYN_BINOP_SUB:
      say("SUB");
      break;
    case ABSYN_BINOP_MUL:
      say("MUL");
      break;
    case ABSYN_BINOP_DIV:
      say("DIV");
      break;
    case ABSYN_BINOP_MOD:
      say("MOD");
      break;
    default:
      error("unknown operator %d in showBinopExp", node->u.binopExp.op);
  }
  say(",\n");
  showNode(node->u.binopExp.left, n + 1);
  say(",\n");
  showNode(node->u.binopExp.right, n + 1);
  say(")");
}


static void showUnopExp(Absyn *node, int n) {
  indent(n);
  say("UnopExp(\n");
  indent(n + 1);
  switch (node->u.unopExp.op) {
    case ABSYN_UNOP_PLUS:
      say("PLUS");
      break;
    case ABSYN_UNOP_MINUS:
      say("MINUS");
      break;
    case ABSYN_UNOP_LNOT:
      say("LNOT");
      break;
    default:
      error("unknown operator %d in showUnopExp", node->u.unopExp.op);
  }
  say(",\n");
  showNode(node->u.unopExp.right, n + 1);
  say(")");
}


static void showInstofExp(Absyn *node, int n) {
  indent(n);
  say("InstofExp(\n");
  showNode(node->u.instofExp.exp, n + 1);
  say(",\n");
  showNode(node->u.instofExp.type, n + 1);
  say(")");
}


static void showCastExp(Absyn *node, int n) {
  indent(n);
  say("CastExp(\n");
  showNode(node->u.castExp.exp, n + 1);
  say(",\n");
  showNode(node->u.castExp.type, n + 1);
  say(")");
}


static void showNilExp(Absyn *node, int n) {
  indent(n);
  say("NilExp()");
}


static void showIntExp(Absyn *node, int n) {
  indent(n);
  say("IntExp(");
  sayInt(node->u.intExp.value);
  say(")");
}


static void showBoolExp(Absyn *node, int n) {
  indent(n);
  say("BoolExp(");
  sayBool(node->u.boolExp.value);
  say(")");
}


static void showCharExp(Absyn *node, int n) {
  indent(n);
  say("CharExp(");
  sayChar(node->u.charExp.value);
  say(")");
}


static void showStringExp(Absyn *node, int n) {
  indent(n);
  say("StringExp(");
  sayString(node->u.stringExp.value);
  say(")");
}


static void showSelfExp(Absyn *node, int n) {
  indent(n);
  say("SelfExp()");
}


static void showSuperExp(Absyn *node, int n) {
  indent(n);
  say("SuperExp()");
}


static void showVarExp(Absyn *node, int n) {
  indent(n);
  say("VarExp(\n");
  showNode(node->u.varExp.var, n + 1);
  say(")");
}


static void showCallExp(Absyn *node, int n) {
  indent(n);
  say("CallExp(\n");
  indent(n + 1);
  say(symToString(node->u.callExp.name));
  say(",\n");
  showNode(node->u.callExp.rcvr, n + 1);
  say(",\n");
  showNode(node->u.callExp.args, n + 1);
  say(")");
}


static void showNewExp(Absyn *node, int n) {
  indent(n);
  say("NewExp(\n");
  indent(n + 1);
  say(symToString(node->u.newExp.name));
  say(",\n");
  showNode(node->u.newExp.args, n + 1);
  say(")");
}

static void showNewArrayExp(Absyn *node, int n) {
  indent(n);
  say("NewArrayExp(\n");
  showNode(node->u.newArrayExp.type, n + 1);
  say(")");
}

static void showSimpleVar(Absyn *node, int n) {
  indent(n);
  say("SimpleVar(");
  say(symToString(node->u.simpleVar.name));
  say(")");
}


static void showArrayVar(Absyn *node, int n) {
  indent(n);
  say("ArrayVar(\n");
  showNode(node->u.arrayVar.var, n + 1);
  say(",\n");
  showNode(node->u.arrayVar.index, n + 1);
  say(")");
}


static void showMemberVar(Absyn *node, int n) {
  indent(n);
  say("MemberVar(\n");
  indent(n + 1);
  say(symToString(node->u.memberVar.name));
  say(",\n");
  showNode(node->u.memberVar.object, n + 1);
  say(")");
}


static void showClsList(Absyn *node, int n) {
  indent(n);
  say("ClsList(");
  while (!node->u.clsList.isEmpty) {
    say("\n");
    showNode(node->u.clsList.head, n + 1);
    node = node->u.clsList.tail;
    if (!node->u.clsList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showMbrList(Absyn *node, int n) {
  indent(n);
  say("MbrList(");
  while (!node->u.mbrList.isEmpty) {
    say("\n");
    showNode(node->u.mbrList.head, n + 1);
    node = node->u.mbrList.tail;
    if (!node->u.mbrList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showParList(Absyn *node, int n) {
  indent(n);
  say("ParList(");
  while (!node->u.parList.isEmpty) {
    say("\n");
    showNode(node->u.parList.head, n + 1);
    node = node->u.parList.tail;
    if (!node->u.parList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showVarList(Absyn *node, int n) {
  indent(n);
  say("VarList(");
  while (!node->u.varList.isEmpty) {
    say("\n");
    showNode(node->u.varList.head, n + 1);
    node = node->u.varList.tail;
    if (!node->u.varList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showStmList(Absyn *node, int n) {
  indent(n);
  say("StmList(");
  while (!node->u.stmList.isEmpty) {
    say("\n");
    showNode(node->u.stmList.head, n + 1);
    node = node->u.stmList.tail;
    if (!node->u.stmList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showExpList(Absyn *node, int n) {
  indent(n);
  say("ExpList(");
  while (!node->u.expList.isEmpty) {
    say("\n");
    showNode(node->u.expList.head, n + 1);
    node = node->u.expList.tail;
    if (!node->u.expList.isEmpty) {
      say(",");
    }
  }
  say(")");
}


static void showNode(Absyn *node, int indent) {
  if (node == NULL) {
    error("showNode got NULL node pointer");
  }
  switch (node->type) {
    case ABSYN_FILE:
      showFile(node, indent);
      break;
    case ABSYN_CLASSDEC:
      showClassDec(node, indent);
      break;
    case ABSYN_FIELDDEC:
      showFieldDec(node, indent);
      break;
    case ABSYN_METHODDEC:
      showMethodDec(node, indent);
      break;
    case ABSYN_VOIDTY:
      showVoidTy(node, indent);
      break;
    case ABSYN_SIMPLETY:
      showSimpleTy(node, indent);
      break;
    case ABSYN_ARRAYTY:
      showArrayTy(node, indent);
      break;
    case ABSYN_PARDEC:
      showParDec(node, indent);
      break;
    case ABSYN_VARDEC:
      showVarDec(node, indent);
      break;
    case ABSYN_EMPTYSTM:
      showEmptyStm(node, indent);
      break;
    case ABSYN_COMPSTM:
      showCompStm(node, indent);
      break;
    case ABSYN_ASSIGNSTM:
      showAssignStm(node, indent);
      break;
    case ABSYN_IFSTM1:
      showIfStm1(node, indent);
      break;
    case ABSYN_IFSTM2:
      showIfStm2(node, indent);
      break;
    case ABSYN_WHILESTM:
      showWhileStm(node, indent);
      break;
    case ABSYN_DOSTM:
      showDoStm(node, indent);
      break;
    case ABSYN_BREAKSTM:
      showBreakStm(node, indent);
      break;
    case ABSYN_RETSTM:
      showRetStm(node, indent);
      break;
    case ABSYN_RETEXPSTM:
      showRetExpStm(node, indent);
      break;
    case ABSYN_CALLSTM:
      showCallStm(node, indent);
      break;
    case ABSYN_BINOPEXP:
      showBinopExp(node, indent);
      break;
    case ABSYN_UNOPEXP:
      showUnopExp(node, indent);
      break;
    case ABSYN_INSTOFEXP:
      showInstofExp(node, indent);
      break;
    case ABSYN_CASTEXP:
      showCastExp(node, indent);
      break;
    case ABSYN_NILEXP:
      showNilExp(node, indent);
      break;
    case ABSYN_INTEXP:
      showIntExp(node, indent);
      break;
    case ABSYN_BOOLEXP:
      showBoolExp(node, indent);
      break;
    case ABSYN_CHAREXP:
      showCharExp(node, indent);
      break;
    case ABSYN_STRINGEXP:
      showStringExp(node, indent);
      break;
    case ABSYN_SELFEXP:
      showSelfExp(node, indent);
      break;
    case ABSYN_SUPEREXP:
      showSuperExp(node, indent);
      break;
    case ABSYN_VAREXP:
      showVarExp(node, indent);
      break;
    case ABSYN_CALLEXP:
      showCallExp(node, indent);
      break;
    case ABSYN_NEWEXP:
      showNewExp(node, indent);
      break;
    case ABSYN_NEWARRAYEXP:
      showNewArrayExp(node, indent);
      break;
    case ABSYN_SIMPLEVAR:
      showSimpleVar(node, indent);
      break;
    case ABSYN_ARRAYVAR:
      showArrayVar(node, indent);
      break;
    case ABSYN_MEMBERVAR:
      showMemberVar(node, indent);
      break;
    case ABSYN_CLSLIST:
      showClsList(node, indent);
      break;
    case ABSYN_MBRLIST:
      showMbrList(node, indent);
      break;
    case ABSYN_PARLIST:
      showParList(node, indent);
      break;
    case ABSYN_VARLIST:
      showVarList(node, indent);
      break;
    case ABSYN_STMLIST:
      showStmList(node, indent);
      break;
    case ABSYN_EXPLIST:
      showExpList(node, indent);
      break;
    default:
      /* this should never happen */
      error("unknown node type %d in showNode", node->type);
      break;
  }
}


void showAbsyn(Absyn *node) {
  showNode(node, 0);
  printf("\n");
}
