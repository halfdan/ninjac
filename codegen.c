#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "vmt.h"
#include "absyn.h"
#include "types.h"
#include "table.h"
#include "codegen.h"

static FILE *asmFile;

/* Function decs */
static void generateCodeNode(Absyn *node, Table *table, int returnLabel, int breakLabel);

/* Function impl */

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
            
            generateCodeNode(classDec,table,returnLabel,breakLabel);
        }
    }
}

static void generateCodeClassDec(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Entry *classEntry = lookupClass(&table, (table)->outerScope, node->u.classDec.name);
    fprintf(asmFile, "// Class \"%s\"\n", node->u.classDec.name->string);
    showVMT(classEntry->u.classEntry.class->vmt,0);
    generateCodeNode(node->u.classDec.members, table, returnLabel, breakLabel);
}

static void generateCodeMbrList(Absyn *node, Table *table, int returnLabel, int breakLabel) {
    Absyn *memberList;
    Absyn *memberDec;

    memberList = node;

    if (!memberList->u.mbrList.isEmpty) {
        for (memberDec = memberList->u.mbrList.head;
                memberList->u.mbrList.isEmpty == FALSE;
                memberList = memberList->u.clsList.tail,
                memberDec = memberList->u.clsList.head) {

            generateCodeNode(memberDec,table,returnLabel,breakLabel);
        }
    }
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
            break;
        case ABSYN_FIELDDEC:
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
