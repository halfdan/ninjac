#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"
#include "types.h"
#include "table.h"
#include "absyn.h"

static void checkNode(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkClassDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkFieldDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkFile(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);

static void checkNode(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {


    switch(node->type) {
        case ABSYN_FILE:
            checkFile(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_FIELDDEC:
            checkFieldDec(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_METHODDEC:

            break;
        default: {
            printf("Unknown node. Something went creepily wrong in the parser.\n");
        }
    }
}

static void checkClassDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *memberList;
    Absyn *memberDec;
    Entry* classEntry;
    Entry* superClassEntry;
    Class* class;

    switch(pass) {
        case 0:
            /* Create new Table for class, fileTable as parent */
            classTable = newTable(*fileTable);
            /* Create new Class record, no superclass, new member table */
            class = newClass(node->u.classDec.publ, node->u.classDec.name, NULL, newTable(classTable));
            /* Create new entry for file/globaltable */
            classEntry = newClassEntry(class);
            /* Add the entry to the fileTable */
            enter(*fileTable, node->u.classDec.name, classEntry);
            /* ..and if it's public to the globalTable aswell */
            if(node->u.classDec.publ) {
                enter(globalTable, node->u.classDec.name, classEntry);
            }            
            break;
        case 1:
            /* Lookup current class entry */
            classEntry = lookup(*fileTable, node->u.classDec.name);
            /* Lookup entry of the supposed superclass */
            superClassEntry = lookup(*fileTable, node->u.classDec.superClass);
            /* Did we find the superclass? */
            if(superClassEntry == NULL) {
                printf("Superclass '%s' not found in '%s' on line %d.\n",
                        node->u.classDec.superClass->string,
                        node->file,
                        node->line);
            }
            /* Is it really a class? */
            else if(superClassEntry->kind != ENTRY_KIND_CLASS) {
                printf("Symbol '%s' is not a class in '%s' on line %d.\n",
                        node->u.classDec.superClass->string,
                        node->file,
                        node->line);
            }
            /* Set the superclass */
            else {
                classEntry->u.classEntry.class->superClass = (Class *)superClassEntry->u.classEntry.class;
            }

            memberList = node->u.classDec.members;
            /* If memberlist isn't empty */
            if (!memberList->u.mbrList.isEmpty) {
                for(memberDec = memberList->u.mbrList.head;
                        memberList->u.mbrList.isEmpty == FALSE;
                        memberList = memberList->u.mbrList.tail,
                        memberDec = memberList->u.mbrList.head) {
                    checkNode(memberDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }            
            break;
        case 2:
            break;
        default: {
            printf("Error: This should never happen! You have found an invalid pass.\n");
            exit(1);
        }
    }
}


static void checkFieldDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    switch(pass) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        default: {
            printf("Error: This should never happen! You have found an invalid pass.\n");
            exit(1);
        }
    }
}


static void checkFOO(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    switch(pass) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        default: {
            printf("Error: This should never happen! You have found an invalid pass.\n");
            exit(1);
        }
    }
}

static void checkFile(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *classList = node->u.file.classes;
    Absyn *classDec;
    Table *tmp;
    
    switch(pass) {
        case 0:
            
            *fileTable = newTable(globalTable);
            if (!classList->u.clsList.isEmpty) {
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }                
            }
            break;
        case 1:
            if (!classList->u.clsList.isEmpty) {
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }
            break;
        case 2:
            break;
        default: {
            printf("Error: This should never happen! You have found an invalid pass.\n");
            exit(1);
        }
    }
}

Table *check(Absyn *fileTrees[], int numInFiles, boolean showSymbolTables) {
    /* initialize tables and foobars */
    Table *globalTable;
    Table *fileTable;
    Table *classTable;
    Table **fileTables;
    Class *objectClass;
    Entry *objectEntry;
    
    int i;

    /* Initialize trivial Classes */
    globalTable = newTable(NULL);
    objectClass = newClass(TRUE, newSym("Object"), NULL, newTable(globalTable));
    objectEntry = newClassEntry(objectClass);
    enter(globalTable, objectClass->name, objectEntry);

    /* Allocate needed Table pointer space */
    fileTables = (Table **)allocate(numInFiles * sizeof(Table *));
    
    /* first pass: collecting classes and other identifiers */
    for(i = 0; i < numInFiles; i++) {
        checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, NULL, 0);
    }    

    /* second pass: build class hierarchy */    
    for(i = 0; i < numInFiles; i++) {
        checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, NULL, 1);
        /*checkNode(fileTrees[i], &(fileTables[i]), globalTable, 1);*/
    }

    if (showSymbolTables) {
        printf("## Global Symboltable ##\n");
        showTable(globalTable);
        for(i = 0; i < numInFiles; i++) {
            printf("## Symboltable for file '%s' ##\n", fileTrees[i]->file);
            showTable(fileTables[i]);
        }
        exit(0);
    }
    /* thid pass: typechecking */
    /*for(i = 0; i < numInFiles; i++) {
        check(fileTrees[i], optionTables, globalTable, 2);
    }*/

    return fileTable;
}
