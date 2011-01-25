#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "instance.h"
#include "absyn.h"
#include "table.h"


InstanceVar *newInstanceVar(Sym* name, InstanceVar *next,
        Sym *className, Sym *fileName, int offset) {

    InstanceVar *tmp = allocate(sizeof(InstanceVar));
    tmp->isEmpty = FALSE;
    tmp->className = className;
    tmp->fileName = fileName;
    tmp->name = name;
    tmp->next = next;
    tmp->offset = offset;
    return tmp;
}


InstanceVar *newEmptyInstanceVar(void) {
    InstanceVar *tmp;

    tmp = newInstanceVar(NULL,NULL,NULL,NULL,0);
    tmp->isEmpty = TRUE;
    
    return tmp;
}


InstanceVar *copyInstanceVar(InstanceVar *src) {
    InstanceVar *tmp;
    if ( src->isEmpty )
        tmp = newEmptyInstanceVar();
    else {
        tmp = newInstanceVar(
                src->name,
                copyInstanceVar(src->next),
                src->className,
                src->fileName,
                src->offset);
    }
    return tmp;
}


int findInstanceVar(InstanceVar* src, Sym *name) {
    while ( ! src->isEmpty ) {
        if ( strcmp(symToString(src->name), symToString(name)) == 0 ) {
            return src->offset;
        }
        src = src->next;
    }
    return -1;
}


void replaceInstanceVar(InstanceVar* src, Sym *name, Sym *className,
        Sym *fileName, int offset) {
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


void appendInstanceVar(InstanceVar* src, Sym *name, Sym *className, Sym *fileName) {
    if ( src->isEmpty ) {
        src->isEmpty = FALSE;
        src->name = name;
        src->className = className;
        src->fileName = fileName;
        src->offset = 0;
        src->next = newEmptyInstanceVar();
    } else {
        while ( ! src->next->isEmpty ) {
            src = src->next;
        }
        src->next = newInstanceVar(name, src->next, className, fileName, src->offset + 1);
    }
}

static void printIndent(int indent) {
    for (;indent > 0;indent--) {
        printf(" ");
    }
}


void showInstanceVar(InstanceVar* src, int indent) {
    printIndent(indent);
    if ( src->isEmpty ) {
        printf("===== END =====\n");
        return;
    }
    printf("%d:\t%s(%s)(%s)\n",
            src->offset,
            src->name->string,
            src->className->string,
            src->fileName->string);
    showInstanceVar(src->next, indent);
}

static void traverseBintree(Bintree *bintree, Class *class,
        Sym *fileName, InstanceVar *attList) {
    Entry *entry;
    Sym *name;
    int offset;

    if (bintree == NULL) {
        return;
    }

    traverseBintree(bintree->left, class, fileName, attList);

    entry = bintree->entry;
    name = bintree->sym;

    if ( entry->kind == ENTRY_KIND_VARIABLE ) {
        if ( ! entry->u.variableEntry.isStatic ) {
            offset = findInstanceVar(attList, name);
            if (offset >= 0) {
                replaceInstanceVar(attList, name, class->name, fileName, offset);
            } else {
                appendInstanceVar(attList, name, class->name, fileName);
            }
        }
    }

    traverseBintree(bintree->right, class, fileName, attList);
}

static void traverseTable(Class *class, Sym *className, Sym *fileName,
        InstanceVar* instancevar) {
    traverseBintree(class->mbrTable->bintree, class, fileName, instancevar);
}

void makeInstanceVariableOffsets(Class *class, char *fileName) {
    InstanceVar *tmp;

    /* nothing to do if instance variable offsets already calculated */
    if ( class->attibuteList != NULL ) {
        return;
    }

    /* calculate offsets of superclass if it is not done by now */
    if ( class->superClass->attibuteList == NULL ) {
        makeInstanceVariableOffsets(class->superClass, fileName);
    }

    tmp = copyInstanceVar(class->superClass->attibuteList);

    traverseTable(class, class->name, newSym(fileName), tmp);
    class->attibuteList = tmp;
}


int countFields(InstanceVar *attList) {
    int count = 0;

    while(!attList->isEmpty) {
        count++;
        attList = attList->next;
    }

    return count;
}
