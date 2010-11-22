#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"
#include "types.h"
#include "table.h"
#include "absyn.h"

/*
 * Semantic Analysis
 * The semantic analysis consists of 4 passes.
 *
 * Pass 0:
 *  - collect all classes in file tables and global table
 *
 * Pass 1:
 *  - construct class hierarchy (add superclasses to classes)
 *
 * Pass 2:
 *  - collect methods and their parameters
 *
 * Pass 3 (Type checking):
 *  - Check if methods defined in subclasses correctly (!) override
 *    methods of the same name in superclasses
 *  -> params can be more generalized
 *  -> return type can be more specific
 *  - collect the local variables
 *  - collect statements
 *
 */

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
static void checkMethodDec(
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
static void checkParamDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkVarDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static Type *lookupTypeFromAbsyn(Absyn *node, Table **fileTable);
static boolean isParamTypeListEqual(TypeList *parList1, TypeList *parList2);

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
            checkMethodDec(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
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
            classEntry = lookup(*fileTable, node->u.classDec.name, ENTRY_KIND_CLASS);
            /* Lookup entry of the supposed superclass */
            superClassEntry = lookup(*fileTable, node->u.classDec.superClass, ENTRY_KIND_CLASS);
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
                classEntry->u.classEntry.class->superClass = superClassEntry->u.classEntry.class;
            }            
            break;
        case 2:
            /* Lookup current class entry */
            classEntry = lookup(*fileTable, node->u.classDec.name, ENTRY_KIND_CLASS);
            memberList = node->u.classDec.members;
            /* If memberlist isn't empty */
            if (!memberList->u.mbrList.isEmpty) {
                for(memberDec = memberList->u.mbrList.head;
                        memberList->u.mbrList.isEmpty == FALSE;
                        memberList = memberList->u.mbrList.tail,
                        memberDec = memberList->u.mbrList.head) {
                    /* Members can be methods or fields */
                    checkNode(memberDec, fileTable, localTable,
                            classEntry->u.classEntry.class,             /* Actual class */
                            classEntry->u.classEntry.class->mbrTable,   /* Member table */
                            globalTable, breakAllowed, returnType, pass);
                }
            }
            break;
        case 3:
            /* Lookup current class entry */
            classEntry = lookup(*fileTable, node->u.classDec.name, ENTRY_KIND_CLASS);
            memberList = node->u.classDec.members;
            /* If memberlist isn't empty */
            if (!memberList->u.mbrList.isEmpty) {
                for(memberDec = memberList->u.mbrList.head;
                        memberList->u.mbrList.isEmpty == FALSE;
                        memberList = memberList->u.mbrList.tail,
                        memberDec = memberList->u.mbrList.head) {
                    /* Members can be methods or fields */
                    checkNode(memberDec, fileTable, localTable,
                            classEntry->u.classEntry.class,             /* Actual class */
                            classEntry->u.classEntry.class->mbrTable,   /* Member table */
                            globalTable, breakAllowed, returnType, pass);
                }
            }
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

    Entry* fieldEntry;
    Type *fieldType;

    switch(pass) {
        case 0:
            break;
        case 1:           
            break;
        case 2:
            /* Determine the type of the field */
            fieldType = lookupTypeFromAbsyn(node->u.fieldDec.type, fileTable);
            /* Create new variable entry, not local */
            fieldEntry = newVariableEntry(FALSE, node->u.fieldDec.publ, node->u.fieldDec.stat, fieldType);
            /* Add the entry to the fileTable */
            if(NULL == enter(classTable, node->u.fieldDec.name, fieldEntry)) {
                printf("Field already exists in class '%s' in file '%s' on line '%d'.\n",
                        actClass->name->string,
                        node->file,
                        node->line);
                exit(1);
            }
            break;
        default: {
            error("This should never happen! You have found an invalid pass.");
        }
    }
}

static void checkMethodDec(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Entry* methodEntry;
    Entry* tmpEntry;
    Type *tmpType;
    TypeList *paramTypes;
    Absyn *localList;
    Absyn *localDec;
    Absyn *stmtList;
    Absyn *stmtDec;
    Absyn *paramList;
    Absyn *paramDec;

    switch(pass) {
        case 0:
            break;
        case 1:            
            break;
        case 2:
            /* Determine the type of the field */
            returnType = lookupTypeFromAbsyn(node->u.methodDec.retType, fileTable);

            /* Build up the TypeList of paramTypes starting with an emptyTypeList */
            paramTypes = emptyTypeList();

            /* Creating a localTable for the method entry*/
            localTable = newTable(classTable);

            /* Getting the list of params for looping over it */
            paramList = node->u.methodDec.params;

            /* Do we have params? */
            if(!paramList->u.parList.isEmpty) {
                /* Loop over all params in the list */
                for(paramDec = paramList->u.parList.head;
                        paramList->u.parList.isEmpty == FALSE;
                        paramList = paramList->u.clsList.tail,
                        paramDec = paramList->u.clsList.head) {

                    /* Get the type of the current param*/
                    tmpType = lookupTypeFromAbsyn(paramDec->u.parDec.type, fileTable);

                    /* Store the type in the paramTypes list */
                    paramTypes = newTypeList(tmpType, paramTypes);

                    /* Check the param declaration */
                    checkParamDec(paramDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }

            methodEntry = newMethodEntry(node->u.methodDec.publ,
                    node->u.methodDec.stat,
                    returnType /* Return type */,
                    paramTypes /* Param types*/,
                    localTable /* Local table*/);

            /* Add the entry to the classTable */
            if(NULL == enter(classTable, node->u.methodDec.name, methodEntry)) {
                /* We don't allow method overloading at this point in Ninja */
                printf("Method already exists in class '%s' in file '%s' on line '%d'.\n",
                        actClass->name->string,
                        node->file,
                        node->line);
                exit(1);
            }
            break;
        case 3:
            /* Fetch method entry from class table */
            methodEntry = lookup(classTable, node->u.methodDec.name, ENTRY_KIND_METHOD);
            /* Does the method already exist in the super class? */
            tmpEntry = lookupMember(actClass->superClass, node->u.methodDec.name, ENTRY_KIND_METHOD);
            if(tmpEntry != NULL) {
                /* Wenn die Methode public ist, nicht static und den gleichen returnType hat.. */
                if(!(tmpEntry->u.methodEntry.isPublic &&
                        /* Return type can be equal or more specific */
                        isSameOrSubtypeOf(methodEntry->u.methodEntry.retType, tmpEntry->u.methodEntry.retType) &&
                        /* Param types of current class can be equal or more generalized */
                        isParamTypeListEqual(tmpEntry->u.methodEntry.paramTypes, methodEntry->u.methodEntry.paramTypes))
                        ) {

                    error("Method already exists in superclass but declarations don't match.");
                }
            }

            /* Does the file contain any locals? */
            localList = node->u.methodDec.locals;
            if (!localList->u.varList.isEmpty) {
                /* Loop over all local variables in the list */
                for(localDec = localList->u.varList.head;
                        localList->u.varList.isEmpty == FALSE;
                        localList = localList->u.varList.tail,
                        localDec = localList->u.varList.head) {
                    /* Check the variable declaration */
                    checkNode(localDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }

            /* Does the file contain any statements? */
            stmtList = node->u.methodDec.stms;
            if (!stmtList->u.stmList.isEmpty) {
                /* Loop over all statements in the list */
                for(stmtDec = stmtList->u.stmList.head;
                        stmtList->u.stmList.isEmpty == FALSE;
                        stmtList = stmtList->u.stmList.tail,
                        stmtDec = stmtList->u.stmList.head) {
                    /* Check the statement declaration */
                    checkNode(stmtDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }
            break;
        default: {
            error("Error: This should never happen! You have found an invalid pass.");
        }
    }
}

static void checkParamDec(
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
        case 3:
            break;
        default: {
            error("Error: This should never happen! You have found an invalid pass.");
        }
    }
}

static void checkVarDec(
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
        case 3:
            break;
        default: {
            error("Error: This should never happen! You have found an invalid pass.");
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
            /* Create a new file table */
            *fileTable = newTable(globalTable);
            /* Does the file contain any classes? */
            if (!classList->u.clsList.isEmpty) {
                /* Loop over all classes in the list */
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    /* Check the class declaration */
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }                
            }
            break;
        case 1:
            /* Does the file contain any classes? */
            if (!classList->u.clsList.isEmpty) {
                /* Loop over all classes in the list */
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    /* Check the class declaration */
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }
            break;
        case 2:
            /* Does the file contain any classes? */
            if (!classList->u.clsList.isEmpty) {
                /* Loop over all classes in the list */
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    /* Check the class declaration */
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }
            break;
        case 3:
            /* Does the file contain any classes? */
            if (!classList->u.clsList.isEmpty) {
                /* Loop over all classes in the list */
                for(classDec = classList->u.clsList.head;
                        classList->u.clsList.isEmpty == FALSE;
                        classList = classList->u.clsList.tail,
                        classDec = classList->u.clsList.head) {
                    /* Check the class declaration */
                    checkClassDec(classDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
                }
            }
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
    }

    /* third pass: typechecking */
    for(i = 0; i < numInFiles; i++) {
         checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, NULL, 2);
    }

    if (showSymbolTables) {
        printf("## Global Symboltable ##\n");
        showTable(globalTable);

        /* Loop over all fileTables */
        for(i = 0; i < numInFiles; i++) {
            printf("## Symboltable for file '%s' ##\n", fileTrees[i]->file);
            showTable(fileTables[i]);
        }
        exit(0);
    }

    return fileTable;
}

static Type *lookupTypeFromAbsyn(Absyn *node, Table **fileTable) {
    Entry *classEntry;
    Absyn *arrayNode;
    int i;

    switch(node->type) {
        case ABSYN_SIMPLETY:
            classEntry = lookup(*fileTable, node->u.simpleTy.name, ENTRY_KIND_CLASS);

            if (classEntry == NULL) {
                error("Unknown identifier '%s' in file '%s' on line '%d'",
                        node->u.simpleTy.name->string,
                        node->file,
                        node->line);
            }

            if (classEntry->kind != ENTRY_KIND_CLASS) {
                error("Identifier '%s' is not a Class in file '%s' on line '%d'",
                        node->u.simpleTy.name->string,
                        node->file,
                        node->line);
            }

            return newSimpleType(classEntry->u.classEntry.class);
            break;
        case ABSYN_ARRAYTY:

            /* This loops through the whole array type syntax tree and counts
             * its dimensions. At the end of the loop in
             *  - i stands for the counted dimensions.
             *  - arrayNode is the abstract syntax node of the simple type. */
            for (   i = 1, arrayNode = node->u.arrayTy.baseType;
                    arrayNode->type != ABSYN_SIMPLETY;
                    i++, arrayNode = arrayNode->u.arrayTy.baseType
                    )
                ;

            /* Lookup the class entry of the type */
            classEntry = lookup(*fileTable, arrayNode->u.simpleTy.name, ENTRY_KIND_CLASS);

            /* Class does not exist */
            if (classEntry == NULL) {
                error("Unknown identifier '%s' in file '%s' on line '%d'",
                        arrayNode->u.simpleTy.name->string,
                        arrayNode->file,
                        arrayNode->line);
            }

            if (classEntry->kind != ENTRY_KIND_CLASS) {
                error("Identifier '%s' is not a Class in file '%s' on line '%d'",
                        arrayNode->u.simpleTy.name->string,
                        arrayNode->file,
                        arrayNode->line);
            }

            return newArrayType(classEntry->u.classEntry.class, i);
            break;

        case ABSYN_VOIDTY:
            return newVoidType();
            break;
        default:
            return NULL;
    }
}

static boolean isParamTypeListEqual(TypeList *parList1, TypeList *parList2) {

    TypeList *tmpList1, *tmpList2;

    /* Loop over the two lists and compare their types*/
    for(
            tmpList1 = parList1,
            tmpList2 = parList2;
            !tmpList1->isEmpty && !tmpList2->isEmpty;
            tmpList1 = tmpList1->next,
            tmpList2 = tmpList2->next
        ) {
        if(!isSameOrSubtypeOf(tmpList1->type, tmpList2->type)) {
            return FALSE;
        }
    }

    /* If one of the elements is not empty we have
     * parameter lists of different length
     */
    if(!tmpList1->isEmpty || !tmpList2->isEmpty) {
        return FALSE;
    }

    return TRUE;
}
