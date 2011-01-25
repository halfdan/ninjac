#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "vmt.h"
#include "types.h"
#include "instance.h"
#include "absyn.h"


VMT *newVMT(Sym* name, VMT *next, char *className, char *fileName, int offset) {
    VMT* vmt;

    vmt = (VMT *) allocate(sizeof (VMT));
    vmt->isEmpty = FALSE;
    vmt->name = name;
    vmt->next = next;
    vmt->className = className;
    vmt->fileName = fileName;
    vmt->offset = offset;

    return vmt;
}

VMT *newEmptyVMT() {
    VMT * vmt;

    vmt = newVMT(NULL, NULL, NULL, NULL, -1);
    vmt->isEmpty = TRUE;

    return vmt;
}

VMT *copyVMT(VMT *src) {
    VMT *vmt;
    if ( src->isEmpty )
        vmt = newEmptyVMT();
    else {
        vmt = newVMT(src->name,
                copyVMT(src->next),
                src->className,
                src->fileName,
                src->offset
                );
    }
    return vmt;
}

int findVMT(VMT* src, Sym *name) {
    while ( ! src->isEmpty ) {
        if ( strcmp(symToString(src->name), symToString(name)) == 0 ) {
            return src->offset;
        }
        src = src->next;
    }
    return -1;
}

void replaceVMT(VMT* src, Sym *name, char *className, char *fileName, int offset) {
    while( ! src->isEmpty ) {
        if ( src->offset == offset ) {
            src->name = name;
            src->className = className;
            src->fileName = fileName;
            return;
        }
        src = src->next;
    }
}

void appendVMT(VMT* src, Sym *name, char *className, char *fileName) {
    if ( src->isEmpty ) {
        src->isEmpty = FALSE;
        src->name = name;
        src->className = className;
        src->fileName = fileName;
        src->offset = 0;
        src->next = newEmptyVMT();
        return;
    }
    while ( ! src->next->isEmpty ) {
        src = src->next;
    }
    src->next = newVMT(name, src->next, className, fileName, src->offset + 1);
}

static void printIndent(int indent) {
    for (;indent > 0;indent--) {
        printf(" ");
    }
}

void showVMT(VMT* src, int indent) {
    printIndent(indent);
    if ( src->isEmpty ) {
        printf("===== END =====\n");
        return;
    }
    printf("%d:\t%s_%s\n", src->offset, src->className, symToString(src->name));
    showVMT(src->next, indent);
}

void printVMT(FILE* file, VMT* vmt) {
    while(!vmt->isEmpty) {
        fprintf(file, ".addr\t%s_%s_%lx\n", vmt->className, symToString(vmt->name), djb2(vmt->fileName));
        vmt = vmt->next;
    }
    fprintf(file, "\n");
}

int countMethods(VMT *vmt) {
    int count = 0;

    while(!vmt->isEmpty) {
        count++;
        vmt = vmt->next;
    }

    return count;
}
