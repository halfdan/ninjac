#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "vmt.h"
#include "absyn.h"
#include "types.h"
#include "instance.h"
#include "table.h"
#include "codegen.h"

static FILE *asmFile;

/* Function decs */
static void generateCodeNode(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel);

/* Function impl */

static char* newMethodLabel(char* filepath, char* classname, char* methodname, boolean isStatic) {
    unsigned long hash;
    char *label;
    int length;

    hash = djb2(filepath);

    /* Label: ClassName_MethodName (+2 for underlines, +8 for hash) */
    length = strlen(classname) + strlen(methodname) + 2 + 8;
    if (isStatic) {
        /* If the method is static we prepend $ to the label */
        length += strlen("$");
    }

    label = (char *) allocate((length + 1) * sizeof (char *));

    if (isStatic) {
        sprintf(label, "$%s_%s_%lx", classname, methodname, hash);
    } else {
        sprintf(label, "%s_%s_%lx", classname, methodname, hash);
    }

    return label;
}

static int newLabel(void) {
    static int numLabels = 0;

    return numLabels++;
}

static void shouldNotReach(char *nodeName) {
    error("code generation should not reach '%s' node", nodeName);
}

static void generateCodeFile(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    fprintf(asmFile, "// File \"%s\"\n", node->file);
    generateCodeNode(node->u.file.classes, table, currentMethod, returnLabel, breakLabel);
}

static void generateCodeClsList(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Absyn *classList;
    Absyn *classDec;

    classList = node;

    if (!classList->u.clsList.isEmpty) {
        for (classDec = classList->u.clsList.head;
                classList->u.clsList.isEmpty == FALSE;
                classList = classList->u.clsList.tail,
                classDec = classList->u.clsList.head) {

            generateCodeNode(classDec, table, currentMethod, returnLabel, breakLabel);
        }
    }
}

static void generateCodeClassDec(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Entry *classEntry = lookupClass(&table, (table)->outerScope, node->u.classDec.name);
    fprintf(asmFile, "// Class \"%s\"\n", node->u.classDec.name->string);
    fprintf(asmFile, "%s_%lx:\n", node->u.classDec.name->string, djb2(node->file));
    printVMT(asmFile, classEntry->u.classEntry.class->vmt);
    generateCodeNode(node->u.classDec.members, classEntry->u.classEntry.class->mbrTable, currentMethod, returnLabel, breakLabel);
}

static void generateCodeMbrList(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Absyn *memberList;
    Absyn *memberDec;

    memberList = node;

    if (!memberList->u.mbrList.isEmpty) {
        for (memberDec = memberList->u.mbrList.head;
                memberList->u.mbrList.isEmpty == FALSE;
                memberList = memberList->u.mbrList.tail,
                memberDec = memberList->u.mbrList.head) {

            generateCodeNode(memberDec, table, currentMethod, returnLabel, breakLabel);
        }
    }
}

static void generateCodeFieldDec(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    /* nothing to generate here */
}

static void generateCodeMethodDec(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Entry *methodEntry;
    char* methodLabel;
    int newRetLabel;

    methodEntry = lookupMember(node->u.methodDec.class, node->u.methodDec.name, ENTRY_KIND_METHOD);
    methodLabel = newMethodLabel(node->file, methodEntry->u.methodEntry.class->name->string, node->u.methodDec.name->string, methodEntry->u.methodEntry.isStatic);

    fprintf(asmFile, "%s:\n", methodLabel);
    fprintf(asmFile, "\tasf\t%d\n", methodEntry->u.methodEntry.numLocals);

    newRetLabel = newLabel();
    generateCodeNode(node->u.methodDec.stms, methodEntry->u.methodEntry.localTable, methodEntry, newRetLabel, breakLabel);

    /* generate function epilog */
    fprintf(asmFile, "_L%d:\n", newRetLabel);
    fprintf(asmFile, "\trsf\n");
    fprintf(asmFile, "\tret\n");
    fprintf(asmFile, "\n");

    free(methodLabel);
}

static void generateCodeStmsList(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Absyn *stmsList;
    Absyn *stmsDec;

    stmsList = node;

    if (!stmsList->u.stmList.isEmpty) {
        for (stmsDec = stmsList->u.stmList.head;
                stmsList->u.stmList.isEmpty == FALSE;
                stmsList = stmsList->u.stmList.tail,
                stmsDec = stmsList->u.stmList.head) {

            generateCodeNode(stmsDec, table, currentMethod, returnLabel, breakLabel);
        }
    }
}

static void generateCodeAsmStmt(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    /* fprintf(asmFile, "%s\n", node->u.asmStm.code); */
}

static void generateCodeEmptyStm(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    /* nothing to generate here */
}

static void generateCodeCompStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    generateCodeNode(node->u.compStm.stms, table, currentMethod, returnLabel, breakLabel);
}

static void generateCodeAssignStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    /* ToDo: Assignment, field/member/local variable lookup */

}

static void generateCodeIfStmt1(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    int label1;

    label1 = newLabel();
    generateCodeNode(node->u.ifStm1.test, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "\tbrf\t_L%d\n", label1);
    generateCodeNode(node->u.ifStm1.thenPart, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "_L%d:\n", label1);
}

static void generateCodeIfStmt2(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    int label1, label2;

    label1 = newLabel();
    label2 = newLabel();
    generateCodeNode(node->u.ifStm2.test, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "\tbrf\t_L%d\n", label1);
    generateCodeNode(node->u.ifStm2.thenPart, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "\tjmp\t_L%d\n", label2);
    fprintf(asmFile, "_L%d:\n", label1);
    generateCodeNode(node->u.ifStm2.elsePart, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "_L%d:\n", label2);
}

static void generateCodeWhileStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    int label1, label2, newBreakLabel;

    label1 = newLabel();
    label2 = newLabel();
    newBreakLabel = newLabel();
    fprintf(asmFile, "\tjmp\t_L%d\n", label2);
    fprintf(asmFile, "_L%d:\n", label1);
    generateCodeNode(node->u.whileStm.body, table, currentMethod, returnLabel, newBreakLabel);
    fprintf(asmFile, "_L%d:\n", label2);
    generateCodeNode(node->u.whileStm.test, table, currentMethod, returnLabel, newBreakLabel);
    fprintf(asmFile, "\tbrt\t_L%d\n", label1);
    fprintf(asmFile, "_L%d:\n", newBreakLabel);
}

static void generateCodeDoStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    int label1, newBreakLabel;

    label1 = newLabel();
    newBreakLabel = newLabel();
    fprintf(asmFile, "_L%d:\n", label1);
    generateCodeNode(node->u.doStm.body, table, currentMethod, returnLabel, newBreakLabel);
    generateCodeNode(node->u.doStm.test, table, currentMethod, returnLabel, newBreakLabel);
    fprintf(asmFile, "\tbrt\t_L%d\n", label1);
    fprintf(asmFile, "_L%d:\n", newBreakLabel);
}

static void generateCodeBreakStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    if (breakLabel == -1) {
        error("no valid break label in genCodeBreakStm");
    }
    fprintf(asmFile, "\tjmp\t_L%d\n", breakLabel);
}

static void generateCodeRetStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    if (returnLabel == -1) {
        error("no valid return label in genCodeRetStm");
    }
    fprintf(asmFile, "\tjmp\t_L%d\n", returnLabel);
}

static void generateCodeRetExpStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    generateCodeNode(node->u.retExpStm.exp, table, currentMethod, returnLabel, breakLabel);
    fprintf(asmFile, "\tpopr\n");
    if (returnLabel == -1) {
        error("no valid return label in genCodeRetExpStm");
    }
    fprintf(asmFile, "\tjmp\t_L%d\n", returnLabel);
}

static void generateCodeSuperExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    shouldNotReach("SuperExp");
}

static void generateCodeCallExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {


    /* We need to switch over the receiver of the callExp */
    switch(node->u.callExp.rcvr->type) {
        case ABSYN_SUPEREXP:
            break;
        case ABSYN_SELFEXP:
            break;
        default:
            generateCodeNode(node->u.callExp.rcvr, table, currentMethod, returnLabel, breakLabel);
            break;
    }

    generateCodeNode(node->u.callExp.args, table, currentMethod, returnLabel, breakLabel);
}

static void generateCodeNewArrayExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Entry *classEntry = lookup(table, node->u.newArrayExp.type, ENTRY_KIND_CLASS);

    generateCodeNode(node->u.newArrayExp.size, table, currentMethod, returnLabel, breakLabel);

    fprintf(asmFile, "\tnewa\n");
    fprintf(asmFile, "\t.addr %s_%lx", node->u.newArrayExp.type->string, djb2(classEntry->u.classEntry.class->fileName));
}

static void generateProlog(Table* table) {
    Entry* mainClass = lookup(table, newSym("$Main"), ENTRY_KIND_CLASS);

    /* execution framework */
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "// execution framework\n");
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "_start:\n");
    fprintf(asmFile, "\tcall\t$Main_main_%lx\n", djb2(mainClass->u.classEntry.class->fileName));
    fprintf(asmFile, "\tcall\t_exit\n");
    /* void exit() */
    fprintf(asmFile, "\n");
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "// _exit()\n");
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "_exit:\n");
    fprintf(asmFile, "\tasf\t0\n");
    fprintf(asmFile, "\thalt\n");
    fprintf(asmFile, "\trsf\n");
    fprintf(asmFile, "\tret\n");
}

static void generateCodeNode(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    if (node == NULL) {
        error("generateCodeNode got NULL node pointer");
    }
    switch (node->type) {
        case ABSYN_FILE: /* 0 */
            generateCodeFile(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_CLASSDEC: /* 1 */
            generateCodeClassDec(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_FIELDDEC: /* 2 */
            generateCodeFieldDec(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_METHODDEC: /* 3 */
            generateCodeMethodDec(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_VOIDTY: /* 4 */
            shouldNotReach("VoidTy");
            break;
        case ABSYN_SIMPLETY: /* 5 */
            shouldNotReach("SimpleTy");
            break;
        case ABSYN_ARRAYTY: /* 6 */
            shouldNotReach("ArrayTy");
            break;
        case ABSYN_PARDEC: /* 7 */
            shouldNotReach("ParDec");
            break;
        case ABSYN_VARDEC: /* 8 */
            shouldNotReach("VarDec");
            break;
        case ABSYN_EMPTYSTM: /* 9 */
            generateCodeEmptyStm(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_COMPSTM: /* 10 */
            generateCodeCompStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASSIGNSTM: /* 11 */
            generateCodeAssignStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_IFSTM1: /* 12 */
            generateCodeIfStmt1(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_IFSTM2: /* 13 */
            generateCodeIfStmt2(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_WHILESTM: /* 14 */
            generateCodeWhileStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_DOSTM: /* 15 */
            generateCodeDoStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_BREAKSTM: /* 16 */
            generateCodeBreakStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_RETSTM: /* 17 */
            generateCodeRetStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_RETEXPSTM: /* 18 */
            generateCodeRetExpStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_CALLSTM: /* 19 */
            break;
        case ABSYN_BINOPEXP: /* 20 */
            break;
        case ABSYN_UNOPEXP: /* 21 */
            break;
        case ABSYN_INSTOFEXP: /* 22 */
            break;
        case ABSYN_CASTEXP: /* 23 */
            break;
        case ABSYN_NILEXP: /* 24 */
            break;
        case ABSYN_INTEXP: /* 25 */
            break;
        case ABSYN_BOOLEXP: /* 26 */
            break;
        case ABSYN_CHAREXP: /* 27 */
            break;
        case ABSYN_STRINGEXP: /* 28 */
            break;
        case ABSYN_SELFEXP: /* 29 */
            break;
        case ABSYN_SUPEREXP: /* 30 */
            generateCodeSuperExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_VAREXP: /* 31 */
            break;
        case ABSYN_CALLEXP: /* 32 */
            generateCodeCallExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_NEWEXP: /* 33 */
            break;
        case ABSYN_NEWARRAYEXP: /* 34 */
            generateCodeNewArrayExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_SIMPLEVAR: /* 35 */
            break;
        case ABSYN_ARRAYVAR: /* 36 */
            break;
        case ABSYN_MEMBERVAR: /* 37 */
            break;
        case ABSYN_CLSLIST: /* 38 */
            generateCodeClsList(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_MBRLIST: /* 39 */
            generateCodeMbrList(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_PARLIST: /* 40 */
            break;
        case ABSYN_VARLIST: /* 41 */
            break;
        case ABSYN_STMLIST: /* 42 */
            generateCodeStmsList(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_EXPLIST: /* 43 */
            break;
        case ABSYN_ASMSTM: /* 44 */
            generateCodeAsmStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        default:
            /* this should never happen */
            error("unknown node type %d in generateCodeNode", node->type);
    }
}

void generateCode(Absyn *fileTrees[], int numInFiles, Table **fileTables, FILE *outFile) {
    int i;
    asmFile = outFile;

    /* fileTables[0]->outerScope is the global table! */
    generateProlog(fileTables[0]->outerScope);

    for (i = 0; i < numInFiles; i++) {
        generateCodeNode(fileTrees[i], fileTables[i], NULL, -1, -1);
    }
}
