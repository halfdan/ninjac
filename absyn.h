/*
 * absyn.h -- abstract syntax
 */


#ifndef _ABSYN_H_
#define _ABSYN_H_


#define ABSYN_FILE		0
#define ABSYN_CLASSDEC		1
#define ABSYN_FIELDDEC		2
#define ABSYN_METHODDEC		3
#define ABSYN_VOIDTY		4
#define ABSYN_SIMPLETY		5
#define ABSYN_ARRAYTY		6
#define ABSYN_PARDEC		7
#define ABSYN_VARDEC		8
#define ABSYN_EMPTYSTM		9
#define ABSYN_COMPSTM		10
#define ABSYN_ASSIGNSTM		11
#define ABSYN_IFSTM1		12
#define ABSYN_IFSTM2		13
#define ABSYN_WHILESTM		14
#define ABSYN_DOSTM		15
#define ABSYN_BREAKSTM		16
#define ABSYN_RETSTM		17
#define ABSYN_RETEXPSTM		18
#define ABSYN_CALLSTM		19
#define ABSYN_BINOPEXP		20
#define ABSYN_UNOPEXP		21
#define ABSYN_INSTOFEXP		22
#define ABSYN_CASTEXP		23
#define ABSYN_NILEXP		24
#define ABSYN_INTEXP		25
#define ABSYN_BOOLEXP		26
#define ABSYN_CHAREXP		27
#define ABSYN_STRINGEXP		28
#define ABSYN_SELFEXP		29
#define ABSYN_SUPEREXP		30
#define ABSYN_VAREXP		31
#define ABSYN_CALLEXP		32
#define ABSYN_NEWEXP		33
#define ABSYN_NEWARRAYEXP       34
#define ABSYN_SIMPLEVAR		35
#define ABSYN_ARRAYVAR		36
#define ABSYN_MEMBERVAR		37
#define ABSYN_CLSLIST		38
#define ABSYN_MBRLIST		39
#define ABSYN_PARLIST		40
#define ABSYN_VARLIST		41
#define ABSYN_STMLIST		42
#define ABSYN_EXPLIST		43
#define ABSYN_ASMSTM            44
#define ABSYN_ASMINSTRLIST      45
#define ABSYN_ASMINSTR0         46
#define ABSYN_ASMINSTR1         47
#define ABSYN_ASMINSTR2         48
#define ABSYN_ASMINSTR3         49


#define ABSYN_BINOP_LOR		0
#define ABSYN_BINOP_LAND	1
#define ABSYN_BINOP_EQ		2
#define ABSYN_BINOP_NE		3
#define ABSYN_BINOP_REQ         4
#define ABSYN_BINOP_RNE         5
#define ABSYN_BINOP_LT		6
#define ABSYN_BINOP_LE		7
#define ABSYN_BINOP_GT		8
#define ABSYN_BINOP_GE		9
#define ABSYN_BINOP_ADD         10
#define ABSYN_BINOP_SUB		11
#define ABSYN_BINOP_MUL		12
#define ABSYN_BINOP_DIV		13
#define ABSYN_BINOP_MOD		14

#define ABSYN_UNOP_PLUS		0
#define ABSYN_UNOP_MINUS	1
#define ABSYN_UNOP_LNOT		2


typedef struct absyn {
  int type;
  char *file;
  int line;
  union {
    struct {
      struct absyn *classes;	/* classes defined in this file */
    } file;
    struct {
      boolean publ;		/* class visibility outside of file */
      Sym *name;		/* name of class */
      Sym *superClass;		/* name of super class */
      struct absyn *members;	/* class members */
    } classDec;
    struct {
      boolean publ;		/* field visibility outside of class */
      boolean stat;		/* is this a class field? */
      Sym *name;		/* name of the field */
      struct absyn *type;	/* type of the field */
    } fieldDec;
    struct {
      boolean publ;		/* method visibility outside of class */
      boolean stat;		/* is this a class method? */
      boolean isConstructor;    /* is this a constructor? */
      Sym *name;		/* name of the method */
      struct class *class;             /* class record belonging to */
      struct absyn *retType;	/* return type */
      struct absyn *params;	/* parameter variables */
      struct absyn *locals;	/* local variables */
      struct absyn *stms;	/* statements */
    } methodDec;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } voidTy;
    struct {
      Sym *name;		/* the simple type's name */
    } simpleTy;
    struct {
      Sym *type;
      int dims;
    } arrayTy;
    struct {
      Sym *name;		/* the parameter's name */
      struct absyn *type;	/* the parameter's type */
    } parDec;
    struct {
      Sym *name;		/* the variable's name */
      struct absyn *type;	/* the variable's type */
    } varDec;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } emptyStm;
    struct {
      struct absyn *stms;	/* list of statements */
    } compStm;
    struct {
      struct absyn *var;	/* variable on LHS */
      struct absyn *exp;	/* expression on RHS */
    } assignStm;
    struct {
      struct absyn *test;	/* test expression */
      struct absyn *thenPart;	/* then-part statement */
    } ifStm1;
    struct {
      struct absyn *test;	/* test expression */
      struct absyn *thenPart;	/* then-part statement */
      struct absyn *elsePart;	/* else-part statement */
    } ifStm2;
    struct {
      struct absyn *test;	/* test expression */
      struct absyn *body;	/* body statement */
    } whileStm;
    struct {
      struct absyn *test;	/* test expression */
      struct absyn *body;	/* body statement */
    } doStm;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } breakStm;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } retStm;
    struct {                    /* asm-Stm */
      struct absyn* instrList;
    } asmStm;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* instruction */
      struct absyn *tail;	/* asm instruction list */
    } asmInstrList;
    struct {                    /* e.g. halt, eq, ne, ..*/
      char *instr;
    } asmInstr0;
    struct {                    /* e.g. drop, getf, putf, .. */
      char *instr;
      int immediate;
    } asmInstr1;
    struct {                    /* vmcall <numArgs>,<offset>*/
      char *instr;
      int numArgs;
      int offset;
    } asmInstr2;
    struct {                    /* e.g. .addr <label>, call <target>, .. */
      char *instr;
      char *label;
    } asmInstr3;
    struct {
      struct absyn *exp;	/* return expression */
    } retExpStm;
    struct {
      Sym *name;		/* method name */
      struct absyn *rcvr;	/* receiver expression */
      struct absyn *args;	/* argument expressions */
      Class *rcvrClass;         /* Class of rcvr */
    } callStm;
    struct {
      int op;			/* operation */
      struct absyn *left;	/* left operand expression */
      struct absyn *right;	/* right operand expression */
      struct type *expType;     /* type of expression */
    } binopExp;
    struct {
      int op;			/* operation */
      struct absyn *right;	/* (right) operand expression */
      struct type *expType;     /* type of expression */
    } unopExp;
    struct {
      struct absyn *exp;	/* expression */
      struct absyn *type;	/* type */
      struct type *expType;     /* type of expression */
    } instofExp;
    struct {
      struct absyn *exp;	/* expression */
      struct absyn *type;	/* type */
      struct type *expType;     /* type of expression */
    } castExp;
    struct {
      int dummy;		/* empty struct not allowed in C */
      struct type *expType;     /* type of expression */
    } nilExp;
    struct {
      int value;		/* the integer literal's value */
      struct type *expType;     /* type of expression */
    } intExp;
    struct {
      int value;		/* the boolean literal's value */
      struct type *expType;     /* type of expression */
    } boolExp;
    struct {
      char value;		/* the character literal's value */
      struct type *expType;     /* type of expression */
    } charExp;
    struct {
      char *value;		/* the string literal's value */
      struct type *expType;     /* type of expression */
    } stringExp;
    struct {
      int dummy;		/* empty struct not allowed in C */
      struct type *expType;     /* type of expression */
    } selfExp;
    struct {
      int dummy;		/* empty struct not allowed in C */
      struct type *expType;     /* type of expression */
    } superExp;
    struct {
      struct absyn *var;	/* variable */
      struct type *expType;     /* type of expression */
    } varExp;
    struct {
      Sym *name;		/* method name */
      struct absyn *rcvr;	/* receiver expression */
      struct absyn *args;	/* argument expressions */
      struct type *expType;     /* type of expression */
      Class *rcvrClass;         /* Class of rcvr */
    } callExp;
    struct {
      Sym *type;		/* class name */
      struct absyn *args;	/* args */
      struct type *expType;     /* type of expression */
    } newExp;
    struct {
      Sym *type;		/* type name */
      struct absyn *size;       /* size of first dimension */
      int dims;                 /* number of dimensions*/
      struct type *expType;     /* type of expression */
    } newArrayExp;
    struct {
      Sym *name;		/* the simple variable's name */
    } simpleVar;
    struct {
      struct absyn *var;	/* the variable which gets indexed */
      struct absyn *index;	/* the index */
    } arrayVar;
    struct {
      Sym *name;		/* the member variable's name */
      struct absyn *object;	/* the object of which it is a member */
      Class *objectClass;	/* the class of the member variable */
    } memberVar;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* class */
      struct absyn *tail;	/* class list */
    } clsList;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* member */
      struct absyn *tail;	/* member list */
    } mbrList;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* parameter */
      struct absyn *tail;	/* parameter list */
    } parList;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* variable */
      struct absyn *tail;	/* variable list */
    } varList;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* statement */
      struct absyn *tail;	/* statement list */
    } stmList;
    struct {
      boolean isEmpty;		/* true: last element, head/tail unused */
      struct absyn *head;	/* expression */
      struct absyn *tail;	/* expression list */
    } expList;
  } u;
} Absyn;


Absyn *newFile(char *file, int line,
               Absyn *classes);
Absyn *newClassDec(char *file, int line,
                   boolean publ, Sym *name,
                   Sym *superClass, Absyn *members);
Absyn *newFieldDec(char *file, int line,
                   boolean publ, boolean stat, Sym *name, Absyn *type);
Absyn *newMethodDec(char *file, int line,
                    boolean publ, boolean stat, boolean isConstructor, Sym *name,
                    Absyn *retType, Absyn *params, Absyn *locals, Absyn *stms);
Absyn *newVoidTy(char *file, int line);
Absyn *newSimpleTy(char *file, int line,
                   Sym *name);
Absyn *newArrayTy(char *file, int line,
                  Sym *type, int dims);
Absyn *newParDec(char *file, int line,
                 Sym *name, Absyn *type);
Absyn *newVarDec(char *file, int line,
                 Sym *name, Absyn *type);
Absyn *newEmptyStm(char *file, int line);
Absyn *newCompStm(char *file, int line,
                  Absyn *stms);
Absyn *newAssignStm(char *file, int line,
                    Absyn *var, Absyn *exp);
Absyn *newIfStm1(char *file, int line,
                 Absyn *test, Absyn *thenPart);
Absyn *newIfStm2(char *file, int line,
                 Absyn *test, Absyn *thenPart, Absyn *elsePart);
Absyn *newWhileStm(char *file, int line,
                   Absyn *test, Absyn *body);
Absyn *newDoStm(char *file, int line,
                Absyn *test, Absyn *body);
Absyn *newBreakStm(char *file, int line);
Absyn *newRetStm(char *file, int line);
Absyn *newRetExpStm(char *file, int line,
                    Absyn *exp);
Absyn *newCallStm(char *file, int line,
                  Sym *name, Absyn *rcvr, Absyn *args);
Absyn *newBinopExp(char *file, int line,
                   int op, Absyn *left, Absyn *right);
Absyn *newUnopExp(char *file, int line,
                  int op, Absyn *right);
Absyn *newInstofExp(char *file, int line,
                    Absyn *exp, Absyn *type);
Absyn *newCastExp(char *file, int line,
                  Absyn *exp, Absyn *type);
Absyn *newNilExp(char *file, int line);
Absyn *newIntExp(char *file, int line,
                 int value);
Absyn *newBoolExp(char *file, int line,
                  int value);
Absyn *newCharExp(char *file, int line,
                  char value);
Absyn *newStringExp(char *file, int line,
                    char *value);
Absyn *newSelfExp(char *file, int line);
Absyn *newSuperExp(char *file, int line);
Absyn *newVarExp(char *file, int line,
                 Absyn *var);
Absyn *newCallExp(char *file, int line,
                  Sym *name, Absyn *rcvr, Absyn *args);
Absyn *newAsmStm(char *file, int line, Absyn *instrList);
Absyn *newNewExp(char *file, int line,
                 Sym *type, Absyn *args);
Absyn *newNewArrayExp(char *file, int line,
                 Sym *type, Absyn *size, int dims);
Absyn *newSimpleVar(char *file, int line,
                    Sym *name);
Absyn *newArrayVar(char *file, int line,
                   Absyn *var, Absyn *index);
Absyn *newMemberVar(char *file, int line,
                    Sym *name, Absyn *object);
Absyn *emptyClsList(void);
Absyn *newClsList(Absyn *head, Absyn *tail);
Absyn *emptyMbrList(void);
Absyn *newMbrList(Absyn *head, Absyn *tail);
Absyn *emptyParList(void);
Absyn *newParList(Absyn *head, Absyn *tail);
Absyn *emptyVarList(void);
Absyn *newVarList(Absyn *head, Absyn *tail);
Absyn *emptyStmList(void);
Absyn *newStmList(Absyn *head, Absyn *tail);
Absyn *emptyExpList(void);
Absyn *newExpList(Absyn *head, Absyn *tail);
Absyn *emptyAsmInstrList(void);
Absyn *newAsmInstrList(Absyn *head, Absyn *tail);
Absyn *newAsmInstr0(char *file, int line, char* instr);
Absyn *newAsmInstr1(char *file, int line, char* instr, int immediate);
Absyn *newAsmInstr2(char *file, int line, char* instr, int numArgs, int offset);
Absyn *newAsmInstr3(char *file, int line, char* instr, char* label);

void showAbsyn(Absyn *node);


#endif /* _ABSYN_H_ */
