#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static void checkCompStm(
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
    Absyn *lhs, *rhs;
    Type *lhs_t, *rhs_t;


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
        case ABSYN_EMPTYSTM:
            break;
        case ABSYN_COMPSTM:
            checkCompStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_ASSIGNSTM:
            /* Bsp.: lhs = rhs */
            lhs = node->u.assignStm.var;
            rhs = node->u.assignStm.exp;
            lhs_t = lookupTypeFromAbsyn(lhs, fileTable);
            rhs_t = lookupTypeFromAbsyn(rhs, fileTable);
            if ( ! isSameOrSubtypeOf(lhs_t, rhs_t) ) {
                printf("assignment right-hand side cannot be converted to left-hand side in '%s' on line %d.\n",
                        node->file,
                        node->line);
            }
            break;
        default: {
            error("Unknown node %d. Something went creepily wrong in the parser.", node->type);
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
    char * superClassName;
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
            if ( NULL == enter(*fileTable, node->u.classDec.name, classEntry) ) {
                error("local redeclaration of class '%s' in '%s' on line %d",
                        symToString(node->u.classDec.name),
                        node->file,
                        node->line);
            }
            /* ..and if it's public to the globalTable aswell */
            if(node->u.classDec.publ) {
                if ( NULL == enter(globalTable, node->u.classDec.name, classEntry) ) {
                    error("global redeclaration of class '%s' in '%s' on line %d",
                        symToString(node->u.classDec.name),
                        node->file,
                        node->line);
                }
            }
            break;
        case 1:
            /* Lookup current class entry, should never be NULL */
            classEntry = lookup(*fileTable, node->u.classDec.name, ENTRY_KIND_CLASS);
            /* Lookup entry of the supposed superclass */
            superClassEntry = lookup(*fileTable, node->u.classDec.superClass, ENTRY_KIND_CLASS);

            /* Did we find the superclass? */
            if(superClassEntry == NULL) {
                error("unknown superclass '%s' in '%s' on line %d",
                        node->u.classDec.superClass->string,
                        node->file,
                        node->line);
            }

            /* if the Superclass is a primitive class */
            superClassName = symToString(superClassEntry->u.classEntry.class->name);
            if (strcmp(superClassName, "Integer") == 0 ) {
                error("primitive class '%s' cannot be extended in '%s' on '%d",
                        superClassName,
                        node->file,
                        node->line);
            }

            /* Set the superclass */
            classEntry->u.classEntry.class->superClass = superClassEntry->u.classEntry.class;

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
            error("This should never happen! You have found an invalid pass.");
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
    Entry* varTmpEntry;
    Type *fieldType;

    switch(pass) {
        case 0:
            break;
        case 1:           
            break;
        case 2:
            /* if defined in one of superclasses either as method or field */
            varTmpEntry = lookupMember(actClass->superClass, node->u.fieldDec.name, ENTRY_KIND_VARIABLE);
            if(varTmpEntry != NULL) {
                error("redeclaration of field '%s' (defined in a superclass of '%s') in file '%s' on line %d",
                        node->u.fieldDec.name->string,
                        actClass->name->string,
                        node->file,
                        node->line);
            }

            /* Determine the type of the field */
            fieldType = lookupTypeFromAbsyn(node->u.fieldDec.type, fileTable);
            /* Create new variable entry, not local */
            fieldEntry = newVariableEntry(FALSE, node->u.fieldDec.publ, node->u.fieldDec.stat, fieldType);


            /* Add the entry to the fileTable */
            if(NULL == enter(classTable, node->u.fieldDec.name, fieldEntry)) {
                error("redeclaration of field '%s' (defined in class '%s') in file '%s' on line %d",
                        node->u.fieldDec.name->string,
                        actClass->name->string,
                        node->file,
                        node->line);
            }

            break;
        case 3:
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
    Entry* superClassMethodEntry;
    Entry* tmpEntry;
    Type *tmpType;
    TypeList *paramTypes;
    TypeList *methodParamsList, *superClassParamsList;
    Absyn *localList;
    Absyn *localDec;
    Absyn *stmtList;
    Absyn *stmtDec;
    Absyn *paramList;
    Absyn *paramDec;
    int i;
    
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

            /* Does the method contain any locals? */
            localList = node->u.methodDec.locals;
            if (!localList->u.varList.isEmpty) {
                /* Loop over all local variables in the list */
                for(localDec = localList->u.varList.head;
                        localList->u.varList.isEmpty == FALSE;
                        localList = localList->u.varList.tail,
                        localDec = localList->u.varList.head) {
                    /* Check the variable declaration */
                    checkVarDec(localDec, fileTable, localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
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
                error("Method already exists in class '%s' in file '%s' on line '%d'.",
                        actClass->name->string,
                        node->file,
                        node->line);
            }
            break;
        case 3:
            /* Fetch method entry from class table */
            methodEntry = lookup(classTable, node->u.methodDec.name, ENTRY_KIND_METHOD);
            /* Does the method already exist in the super class? */
            superClassMethodEntry = lookupMember(actClass->superClass, node->u.methodDec.name, ENTRY_KIND_METHOD);
            if(superClassMethodEntry != NULL) {
                /* Redeclaration errors */
                if (superClassMethodEntry->u.methodEntry.isStatic) {
                    if (superClassMethodEntry->u.methodEntry.isPublic != node->u.methodDec.publ) {
                        error("shadowing method '%s' changes visibility in '%s' on line %d",
                                node->u.methodDec.name->string,
                                node->file,
                                node->line);
                    }

                    if (!methodEntry->u.methodEntry.isStatic) {
                        error("redeclaration of static superclass method '%s' (defined in a superclass of '%s') as non-static in '%s' on line %d",
                                node->u.methodDec.name->string,
                                actClass->name->string,
                                node->file,
                                node->line);
                    }
                } else {
                    if (superClassMethodEntry->u.methodEntry.isPublic != node->u.methodDec.publ) {
                        error("overriding method '%s' changes visibility in '%s' on line %d",
                                node->u.methodDec.name->string,
                                node->file,
                                node->line);
                    }
                    if (methodEntry->u.methodEntry.isStatic) {
                        error("redeclaration of non-static superclass method '%s' (defined in a superclass of '%s') as static in '%s' on line %d",
                                node->u.methodDec.name->string,
                                actClass->name->string,
                                node->file,
                                node->line);
                    }
                    if ( ! isSameOrSubtypeOf(methodEntry->u.methodEntry.retType, superClassMethodEntry->u.methodEntry.retType)) {
                        error("return type of overriding method '%s' cannot be converted in '%s' on line %d",
                                node->u.methodDec.name->string,
                                node->file,
                                node->line);
                    }


                    /* check for ParamLists */
                    methodParamsList = methodEntry->u.methodEntry.paramTypes;
                    superClassParamsList = superClassMethodEntry->u.methodEntry.paramTypes;
                    
                    if ( ! isParamTypeListLengthEqual(methodParamsList, superClassParamsList) ) {
                        error("overriding method '%s' has different number of parameters in '%s' on line %d",
                                node->u.methodDec.name->string,
                                node->file,
                                node->line);
                    } else {

                        /* Loop over the two lists and compare their types*/
                        for(
                                i=0,
                                methodParamsList = methodEntry->u.methodEntry.paramTypes,
                                superClassParamsList = superClassMethodEntry->u.methodEntry.paramTypes;
                                !methodParamsList->isEmpty && !superClassParamsList->isEmpty;
                                i++,
                                methodParamsList = methodParamsList->next,
                                superClassParamsList = superClassParamsList->next
                            ) {
                            if(!isSameOrSubtypeOf(superClassParamsList->type, methodParamsList->type)) {
                                error("type of parameter #%d of overriding method '%s' cannot be converted in '%s' on line %d",
                                        getLength(methodEntry->u.methodEntry.paramTypes) - i,
                                        node->u.methodDec.name->string,
                                        node->file,
                                        node->line);
                            }
                        }
                    }
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
                    checkNode(stmtDec, fileTable, methodEntry->u.methodEntry.localTable, actClass, classTable, globalTable, breakAllowed, returnType, pass);
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

    Type *variableType;
    Entry* variableEntry;

    switch(pass) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            /* Lookup the type of the parameter */
            variableType = lookupTypeFromAbsyn(node->u.parDec.type, fileTable);
            /* Create new variable entry (a param is a local variable after all) */
            variableEntry = newVariableEntry(TRUE, FALSE, FALSE, variableType);
            /* Add the entry to the localTable */
            if(NULL == enter(localTable, node->u.parDec.name, variableEntry)) {
                /* Multiple definitions of variables are not allowed  */
                error("Variable '%s' already exists in method in file '%s' on line %d.",
                        node->u.parDec.name->string,
                        node->file,
                        node->line);
            }
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

    Type *variableType;
    Entry* variableEntry;

    switch(pass) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            /* Lookup the type of the local variable */
            variableType = lookupTypeFromAbsyn(node->u.varDec.type, fileTable);
            /* Create new local variable entry */
            variableEntry = newVariableEntry(TRUE, FALSE, FALSE, variableType);
            /* Add the entry to the localTable */
            if(NULL == enter(localTable, node->u.varDec.name, variableEntry)) {
                /* Multiple definitions of variables are not allowed  */
                error("Variable '%s' already exists in method in file '%s' on line %d.",
                        node->u.varDec.name->string,
                        node->file,
                        node->line);
            }
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

static void checkCompStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *stmtList;
    Absyn *stmtDec;

    /* Does this compound statement contain any statements? */
    stmtList = node->u.compStm.stms;
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
}

Table **check(Absyn *fileTrees[], int numInFiles, boolean showSymbolTables) {
    /* initialize tables and foobars */
    Table *globalTable;
    Table *fileTable;
    Table *classTable;
    Table **fileTables;
    Class *objectClass;
    Class *integerClass;
    Class *booleanClass;
    Entry *objectEntry;
    Entry *integerEntry;
    Entry *booleanEntry;
    Entry *mainClassEntry;
    Entry *mainMethodEntry;
    
    int i;

    /* Initialize trivial Classes */
    globalTable = newTable(NULL);

    objectClass = newClass(TRUE, newSym("Object"), NULL, newTable(globalTable));
    objectEntry = newClassEntry(objectClass);
    enter(globalTable, objectClass->name, objectEntry);

    integerClass = newClass(TRUE, newSym("Integer"), NULL, newTable(globalTable));
    integerEntry = newClassEntry(integerClass);
    enter(globalTable, integerClass->name, integerEntry);

    booleanClass = newClass(TRUE, newSym("Boolean"), NULL, newTable(globalTable));
    booleanEntry = newClassEntry(booleanClass);
    enter(globalTable, booleanClass->name, booleanEntry);

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

    /* third pass: stuff */
    for(i = 0; i < numInFiles; i++) {
         checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, NULL, 2);
    }

    /* fourth pass: typechecking */
    for(i = 0; i < numInFiles; i++) {
         checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, NULL, 3);
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

    mainClassEntry = lookup(globalTable, newSym("Main"), ENTRY_KIND_CLASS);

    if ( NULL == mainClassEntry ) {
        error("public class '%s' is missing.", mainClass);
    } else {
        mainMethodEntry = lookup(mainClassEntry->u.classEntry.class->mbrTable, newSym("main"), ENTRY_KIND_METHOD);
        if ( NULL == mainMethodEntry ) {
            error("public class '%s' does not contain a method 'main'.", mainClass);
        } else {
            if ( ! mainMethodEntry->u.methodEntry.isPublic ) {
                error("method 'main' of public class '%s' is not public", mainClass);
            }
            if ( ! mainMethodEntry->u.methodEntry.isStatic ) {
                error("method 'main' of public class '%s' is not static", mainClass);
            }
            if ( mainMethodEntry->u.methodEntry.retType->kind != TYPE_KIND_VOID ) {
                error("method 'main' of public class '%s' must have return type 'void", mainClass);
            }
            if ( ! mainMethodEntry->u.methodEntry.paramTypes->isEmpty ) {
                error("method 'main' of public class '%s' must not have any arguments", mainClass);
            }
        }
    }

    return fileTables;
}

static Type *lookupTypeFromAbsyn(Absyn *node, Table **fileTable) {
    Entry *classEntry;

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
            /* Lookup the class entry of the type */
            classEntry = lookup(*fileTable, node->u.arrayTy.type, ENTRY_KIND_CLASS);

            /* Class does not exist */
            if (classEntry == NULL) {
                error("Unknown identifier '%s' in file '%s' on line '%d'",
                        node->u.arrayTy.type->string,
                        node->file,
                        node->line);
            }

            if (classEntry->kind != ENTRY_KIND_CLASS) {
                error("Identifier '%s' is not a Class in file '%s' on line '%d'",
                        node->u.arrayTy.type->string,
                        node->file,
                        node->line);
            }

            return newArrayType(classEntry->u.classEntry.class, node->u.arrayTy.dims);
            break;

        case ABSYN_VOIDTY:
            return newVoidType();
            break;
        default:
            return NULL;
    }
}

