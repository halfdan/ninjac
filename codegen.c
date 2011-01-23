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
static void generateCodeNode(Absyn *node, Table *table, int returnLabel, int breakLabel);

/* Function impl */

static char* newMethodLabel(char* filepath, char* classname, char* methodname, boolean isStatic) {
    unsigned long hash;
    char *label;
    int length;

    hash = djb2(filepath);

    /* Label: ClassName_MethodName (+2 for underlines, +8 for hash) */
    length = strlen(classname) + strlen(methodname) + 2 + 8;
    if (isStatic) {
        /* If the method is static we prepend CLASS_ to the label */
        length += strlen("CLASS_");
    }

    label = (char *) allocate((length + 1) * sizeof (char *));

    if (isStatic) {
        sprintf(label, "CLASS_%s_%s_%lx", classname, methodname, hash);
    } else {
        sprintf(label, "%s_%s_%lx", classname, methodname, hash);
    }

    return label;
}

static int newReturnLabel(void) {
    static int numLabels = 0;

    return numLabels++;
}

static void shouldNotReach(char *nodeName) {
  error("code generation should not reach '%s' node", nodeName);
}


static void generateCodeFile(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    fprintf(asmFile, "// File \"%s\"\n", node->file);
    generateCodeNode(node->u.file.classes, table, returnLabel, breakLabel);
}

static void generateCodeClsList(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Absyn *classList;
    Absyn *classDec;

    classList = node;

    if (!classList->u.clsList.isEmpty) {
        for (classDec = classList->u.clsList.head;
                classList->u.clsList.isEmpty == FALSE;
                classList = classList->u.clsList.tail,
                classDec = classList->u.clsList.head) {

            generateCodeNode(classDec, table, returnLabel, breakLabel);
        }
    }
}

static void generateCodeClassDec(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Entry *classEntry = lookupClass(&table, (table)->outerScope, node->u.classDec.name);
    fprintf(asmFile, "// Class \"%s\"\n", node->u.classDec.name->string);
    fprintf(asmFile, "%s_%lx:\n", node->u.classDec.name->string, djb2(node->file));
    printVMT(asmFile, classEntry->u.classEntry.class->vmt);
    generateCodeNode(node->u.classDec.members, classEntry->u.classEntry.class->mbrTable, returnLabel, breakLabel);
}

static void generateCodeMbrList(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Absyn *memberList;
    Absyn *memberDec;

    memberList = node;

    if (!memberList->u.mbrList.isEmpty) {
        for (memberDec = memberList->u.mbrList.head;
                memberList->u.mbrList.isEmpty == FALSE;
                memberList = memberList->u.mbrList.tail,
                memberDec = memberList->u.mbrList.head) {

            generateCodeNode(memberDec, table, returnLabel, breakLabel);
        }
    }
}

static void generateCodeMethodDec(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Entry *methodEntry;
    char* methodLabel;
    int newRetLabel;

    methodEntry = lookup(table, node->u.methodDec.name, ENTRY_KIND_METHOD);
    methodLabel = newMethodLabel(node->file, methodEntry->u.methodEntry.class->name->string, node->u.methodDec.name->string, methodEntry->u.methodEntry.isStatic);

    fprintf(asmFile, "%s:\n", methodLabel);
    fprintf(asmFile, "\tasf\t%d\n", methodEntry->u.methodEntry.numLocals);

    newRetLabel = newReturnLabel();
    generateCodeNode(node->u.methodDec.stms,methodEntry->u.methodEntry.localTable,newRetLabel,breakLabel);

    /* generate function epilog */
    fprintf(asmFile, "_L%d:\n", newRetLabel);
    fprintf(asmFile, "\trsf\n");
    fprintf(asmFile, "\tret\n");
    fprintf(asmFile, "\n");

    free(methodLabel);
}

static void generateCodeStmsList(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Absyn *stmsList;
    Absyn *stmsDec;

    stmsList = node;

    if (!stmsList->u.stmList.isEmpty) {
        for (stmsDec = stmsList->u.stmList.head;
                stmsList->u.stmList.isEmpty == FALSE;
                stmsList = stmsList->u.stmList.tail,
                stmsDec = stmsList->u.stmList.head) {

            generateCodeNode(stmsDec, table, returnLabel, breakLabel);
        }
    }
}

static void generateCodeAsmStmt(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    fprintf(asmFile, node->u.asmStm.code);
}

static void generateCodeParDec(Absyn *node, Table *table,
                          int returnLabel, int breakLabel) {
  shouldNotReach("ParDec");
}


static void generateCodeVarDec(Absyn *node, Table *table,
                          int returnLabel, int breakLabel) {
  shouldNotReach("VarDec");
}


static void generateCodeEmptyStm(Absyn *node, Table *table,
                            int returnLabel, int breakLabel) {
  /* nothing to generate here */
}

static void generateCodeNode(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    if (node == NULL) {
        error("generateCodeNode got NULL node pointer");
    }
    switch (node->type) {
        case ABSYN_FILE:
            generateCodeFile(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_CLSLIST:
            generateCodeClsList(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_CLASSDEC:
            generateCodeClassDec(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_MBRLIST:
            generateCodeMbrList(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_METHODDEC:
            generateCodeMethodDec(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_FIELDDEC:
            break;
        case ABSYN_STMLIST:
            generateCodeStmsList(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_ASMSTM:
            generateCodeAsmStmt(node, table, returnLabel, breakLabel);
            break;
        case ABSYN_EMPTYSTM:
            generateCodeEmptyStm(node,table,returnLabel,breakLabel);
            break;
        case ABSYN_VARDEC:
            generateCodeVarDec(node,table,returnLabel,breakLabel);
            break;
        case ABSYN_PARDEC:
            generateCodeParDec(node,table,returnLabel,breakLabel);
            break;
        default:
            /* this should never happen */
            error("unknown node type %d in generateCodeNode", node->type);
    }
}

void generateCode(Absyn *fileTrees[], int numInFiles, Table **fileTables, FILE *outFile) {
    int i;
    asmFile = outFile;
    for (i = 0; i < numInFiles; i++) {
        generateCodeNode(fileTrees[i], fileTables[i], -1, -1);
    }
}
