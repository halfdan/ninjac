#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "sym.h"
#include "absyn.h"
#include "vmt.h"
#include "types.h"
#include "instance.h"
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


/* i had to do this ugly hack because we have no possibility to save the actual
 * function like the actual class. This is not because we are too lazy to implement
 * a new parameter but because we have no 'Type Method' */
Entry *actMethod;
/* same with localOffset*/
int localOffset;

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
static void checkSimpleTy(
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
static void checkAssignStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkVarExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkSimpleVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkArrayVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkMemberVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkIfStm1(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkIfStm2(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkWhileStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkDoStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkBreakStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkRetStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkRetExpStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkCallStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkBinOpExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkUnOpExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkInstanceOfExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkCastExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkNilExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkIntExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkBoolExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkCharExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkSelfExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkSuperExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkNilExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkNewExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkNewArrayExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkCallExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass);
static void checkAsmStm(
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
        case ABSYN_SIMPLETY:
            checkSimpleTy(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_EMPTYSTM:
            break;
        case ABSYN_COMPSTM:
            checkCompStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_ASSIGNSTM:
            checkAssignStm(node, fileTable, localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_VAREXP:
            checkVarExp(node, fileTable, localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_SIMPLEVAR:
            checkSimpleVar(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_ARRAYVAR:
            checkArrayVar(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_MEMBERVAR:
            checkMemberVar(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_IFSTM1:
            checkIfStm1(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_IFSTM2:
            checkIfStm2(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_WHILESTM:
            checkWhileStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_DOSTM:
            checkDoStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_BREAKSTM:
            checkBreakStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_RETSTM:
            checkRetStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_RETEXPSTM:
            checkRetExpStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_CALLSTM:
            checkCallStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_BINOPEXP:
            checkBinOpExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_UNOPEXP:
            checkUnOpExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_INSTOFEXP:
            checkInstanceOfExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_CASTEXP:
            checkCastExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_NILEXP:
            checkNilExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_INTEXP:
            checkIntExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_BOOLEXP:
            checkBoolExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_CHAREXP:
            checkCharExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_SELFEXP:
            checkSelfExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_SUPEREXP:
            checkSuperExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_NEWEXP:
            checkNewExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_CALLEXP:
            checkCallExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_NEWARRAYEXP:
            checkNewArrayExp(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        case ABSYN_ASMSTM:
            checkAsmStm(node, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, returnType, pass);
            break;
        default: {
            error("Unknown node %d. Something went creepily wrong in the parser.", node->type);
        }
    }
}


static void traverseBintree(Bintree *bintree, Class *class, char *fileName, VMT *vmt) {
    Entry *entry;
    Sym *name;
    int offset;
    char *className = class->name->string;

    if (bintree == NULL) {
        return;
    }

    traverseBintree(bintree->left, class, fileName, vmt);

    entry = bintree->entry;
    name = bintree->sym;

    if ( entry->kind == ENTRY_KIND_METHOD ) {
        if ( ! entry->u.methodEntry.isStatic ) {
            offset = findVMT(vmt, name);
            if (offset >= 0) {
                replaceVMT(vmt, name, className, fileName, offset);
            } else {
                appendVMT(vmt, name, className, fileName);
            }
        }
    }
    
    traverseBintree(bintree->right, class, fileName, vmt);
}


static void traverseTable(Class *class, char *filename, VMT *vmt) {
    traverseBintree(class->mbrTable->bintree, class, filename, vmt);
}


static void makeVMT(Class *class, char *fileName) {
    VMT *vmt;

    /* nothing to do if vmt already exists */
    if ( class->vmt != NULL ) {
        return;
    }

    /* make VMT of superclass if it does not exist */
    if ( class->superClass->vmt == NULL ) {
        makeVMT(class->superClass, fileName);
    }

    vmt = copyVMT(class->superClass->vmt);

    traverseTable(class, fileName, vmt);
    class->vmt = vmt;
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
    Entry *classEntry;
    Entry *superClassEntry;
    Entry *metaClassEntry;
    Entry *metaSuperClassEntry;
    char *superClassName;
    Class *class;
    Class *metaClass;

    switch(pass) {
        case 0:
            /* Create new Table for class, fileTable as parent */
            classTable = newTable(*fileTable);
            /* Create new Class record for the meta class, too */
            metaClass = newClass(node->u.classDec.publ, metaClassName(node->u.classDec.name),
                    node->file, NULL, NULL, newTable( newTable(*fileTable)));
            /* Create new Class record, no superclass, new member table */
            class = newClass(node->u.classDec.publ, node->u.classDec.name,
                    node->file, NULL, metaClass, newTable(classTable));
            /* Create new entry for file/globaltable */
            metaClassEntry = newClassEntry(metaClass);
            /* Add the entry to the fileTable */
            classEntry = newClassEntry(class);
            /* Create new entry for the meta class */
            if ( NULL == enter(*fileTable, node->u.classDec.name, classEntry) ) {
                error("local redeclaration of class '%s' in '%s' on line %d",
                        symToString(node->u.classDec.name),
                        node->file,
                        node->line);
            }
            /* Add the meta class entry to the fileTable */
            if ( NULL == enter(*fileTable, metaClassEntry->u.classEntry.class->name, metaClassEntry) ) {
                error("local redeclaration of meta class '%s' in '%s' on line %d",
                        metaClassEntry->u.classEntry.class->name,
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
                /* and so goes the meta class */
                if ( NULL == enter(globalTable, metaClassEntry->u.classEntry.class->name, metaClassEntry) ) {
                    error("global redeclaration of class '%s' in '%s' on line %d",
                        metaClassEntry->u.classEntry.class->name->string,
                        node->file,
                        node->line);
                }
            }
            break;
        case 1:
            /* Lookup current class entry, should never be NULL */
            classEntry = lookupClass(fileTable, globalTable, node->u.classDec.name);
            /* Lookup meta class entry, should never be NULL */
            metaClassEntry = lookupClass(fileTable, globalTable, metaClassName(node->u.classDec.name));
            /* Lookup entry of the supposed superclass */
            superClassEntry = lookupClass(fileTable, globalTable, node->u.classDec.superClass);
            /* Lookup entry of the supposed superclass of meta class */
            metaSuperClassEntry = lookupClass(fileTable, globalTable, metaClassName(node->u.classDec.superClass));

            /* Did we find the superclass? */
            if(superClassEntry == NULL) {
                error("unknown superclass '%s' in '%s' on line %d",
                        node->u.classDec.superClass->string,
                        node->file,
                        node->line);
            }

            /* Did we find the meta superclass? */
            if(metaSuperClassEntry == NULL) {
                error("unknown superclass of metaclass '%s' in '%s' on line %d",
                        metaClassName(node->u.classDec.superClass)->string,
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
            /* Set the superclass of meta class */
            metaClassEntry->u.classEntry.class->superClass = metaSuperClassEntry->u.classEntry.class;

            break;
        case 2:
            /* Lookup current class entry */
            classEntry = lookupClass(fileTable, globalTable, node->u.classDec.name);
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
            classEntry = lookupClass(fileTable, globalTable, node->u.classDec.name);
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
        case 4:
            /* here will be dragons */
            /* here will be the creation of the virtual method table */
            /* here will be the evaluation of the instance variable offsets */
            /* Lookup current class entry */
            classEntry = lookupClass(fileTable, globalTable, node->u.classDec.name);
            makeVMT(classEntry->u.classEntry.class, node->file);
            makeInstanceVariableOffsets(classEntry->u.classEntry.class, node->file);
            makeVMT(classEntry->u.classEntry.class->metaClass, node->file);
            makeInstanceVariableOffsets(classEntry->u.classEntry.class->metaClass, node->file);
/*            printf("instVars of %s\n", classEntry->u.classEntry.class->name->string);
            showInstanceVar(classEntry->u.classEntry.class->attibuteList, 0);*/
            printf("instVars of %s\n", classEntry->u.classEntry.class->metaClass->name->string);
            showInstanceVar(classEntry->u.classEntry.class->metaClass->attibuteList, 0);
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
            if(varTmpEntry != NULL && !varTmpEntry->u.variableEntry.isStatic) {
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


            /* Add the entry to the member Table */
            /* if the member is static
             * then add it to the member table of the meta class */
            if( node->u.fieldDec.stat ) {
                fieldEntry->u.variableEntry.isStatic = FALSE;
                if(NULL == enter(actClass->metaClass->mbrTable, node->u.fieldDec.name, fieldEntry)) {
                    error("redeclaration of field '%s' (defined in class '%s') in file '%s' on line %d",
                            node->u.fieldDec.name->string,
                            actClass->name->string,
                            node->file,
                            node->line);
                }
            } else {
                if(NULL == enter(classTable, node->u.fieldDec.name, fieldEntry)) {
                    error("redeclaration of field '%s' (defined in class '%s') in file '%s' on line %d",
                            node->u.fieldDec.name->string,
                            actClass->name->string,
                            node->file,
                            node->line);
                }
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

    Class *metaClass = actClass->metaClass;
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
            if(NULL != node->u.methodDec.retType) {
                /* Determine the type of the field */
                returnType = lookupTypeFromAbsyn(node->u.methodDec.retType, fileTable);
            }
            else {
                /* A constructor method does not have a return type */
                returnType = NULL;
                /* Does the constructor have the same name as the class its in? */
                if(actClass->name != node->u.methodDec.name) {
                    error("constructor method must have the same name as the class in '%s' on line %d",
                            node->file,
                            node->line);
                }
            }
            

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
            /* initialize the local offset with 0 */
            localOffset = 0;
            if (!localList->u.varList.isEmpty) {
                /* Loop over all local variables in the list */
                for(localDec = localList->u.varList.head;
                        localList->u.varList.isEmpty == FALSE;
                        localList = localList->u.varList.tail,
                        localDec = localList->u.varList.head) {
                    /* Check the variable declaration */
                    checkVarDec(localDec, fileTable, localTable,
                            actClass, classTable, globalTable,
                            breakAllowed, returnType, pass);
                }
            }

            methodEntry = newMethodEntry(node->u.methodDec.publ,
                    node->u.methodDec.stat,
                    returnType /* Return type */,
                    paramTypes /* Param types*/,
                    localTable /* Local table*/,
                    actClass);

            /* Add the entry to the classTable if non-static
             * otherwise add the entry to the classTable of the meta class */
            if (node->u.methodDec.stat) {
                methodEntry->u.methodEntry.isStatic = FALSE;
                if(NULL == enter(metaClass->mbrTable, node->u.methodDec.name, methodEntry)) {
                    /* We don't allow method overloading at this point in Ninja */
                    error("Method already exists in class '%s' in file '%s' on line '%d'.",
                            actClass->name->string,
                            node->file,
                            node->line);
                }
            } else {
                if(NULL == enter(classTable, node->u.methodDec.name, methodEntry)) {
                    /* We don't allow method overloading at this point in Ninja */
                    error("Method already exists in class '%s' in file '%s' on line '%d'.",
                            actClass->name->string,
                            node->file,
                            node->line);
                }
            }
            break;
        case 3:
            /* Fetch method entry from class table */
            /*methodEntry = lookup(classTable, node->u.methodDec.name, ENTRY_KIND_METHOD);*/
            methodEntry = lookupMember(actClass, node->u.methodDec.name, ENTRY_KIND_METHOD);

            /* save a pointer of actual method */
            actMethod = methodEntry;

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
                error("redeclaration of parameter '%s' in '%s' on line %d",
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
                error("redeclaration of parameter or local variable '%s' in '%s' on line %d",
                        node->u.varDec.name->string,
                        node->file,
                        node->line);
            }
            variableEntry->u.variableEntry.offset = localOffset++;
            break;
        case 3:
            break;
        default: {
            error("Error: This should never happen! You have found an invalid pass.");
        }
    }
}

static void checkSimpleTy(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Sym *name = node->u.simpleTy.name;
    Entry *typeEntry = lookupClass(fileTable, globalTable, name);
    Type *typeType;

    if(typeEntry == NULL) {
        error("Simpletype is not a type");
    }

    typeType = newStaticSimpleType(typeEntry->u.classEntry.class);

    *returnType = *typeType;
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
        case 4:
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

static void checkAssignStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *lhs = node->u.assignStm.var;
    Absyn *rhs = node->u.assignStm.exp;
    Entry *var;
    Type *lhs_t, *rhs_t;

    lhs_t = allocate(sizeof(Type));
    rhs_t = allocate(sizeof(Type));

    switch(pass) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            /* Bsp.: lhs = rhs ; */
            /* lhs ist immer varExp */
            checkNode(lhs, fileTable, localTable, actClass,
                    classTable, globalTable, breakAllowed, lhs_t, pass);
            /* rhs ist irgendeine Exp */
            checkNode(rhs, fileTable, localTable, actClass,
                    classTable, globalTable, breakAllowed, rhs_t, pass);

            if ( ! isSameOrSubtypeOf(rhs_t, lhs_t) ) {
                error("assignment right-hand side cannot be converted to left-hand side in '%s' on line %d",
                        node->file,
                        node->line);
            }

            if (actMethod->u.methodEntry.isStatic) {
                switch(lhs->type) {
                    case ABSYN_SIMPLEVAR:
                        var = lookup(classTable, lhs->u.simpleVar.name, ENTRY_KIND_VARIABLE);

                        /* if variable is a local variable, everything is ok */
                        if ( var == NULL ) {
                            break;
                        }


                        /*
                         *  if it is a memberVar then it is fail in every possibility
                         * there are two ways how an assignment can fail in a static method:
                         * 1) assignment to a non-static field
                         * 2) assignment to a static field without the class name
                         */
                        if ( ! var->u.variableEntry.isStatic) {
                            error("non-static field '%s' cannot be accessed within a static method in '%s' on line %d",
                                    lhs->u.simpleVar.name->string,
                                    node->file,
                                    node->line);
                        } else {
                            error("static field '%s' cannot be accessed without a class name in '%s' on line %d",
                                    lhs->u.simpleVar.name->string,
                                    node->file,
                                    node->line);
                        }
                        break;
                    default:
                        break;
                }
            }

            free(lhs_t);
            free(rhs_t);
            /* ToDo */
            break;
        default:
            break;
    }
}

static void checkVarExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Type *tmpType = allocate(sizeof(Type));

    /* don't differenciate (Schlumpf) between passes because expressions
     * only matter in pass 4 */
    switch(node->u.varExp.var->type) {
        case ABSYN_SIMPLEVAR:
            /* in case the varExp is a simple var
             * check the var absyn node and hope checkSimpleVar returns the
             * correct type */
            checkSimpleVar(node->u.varExp.var, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, tmpType, pass);
            break;
        case ABSYN_ARRAYVAR:
            /* in case the varExp is an array var
             * check the  */
            checkArrayVar(node->u.varExp.var, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, tmpType, pass);
            break;
        case ABSYN_MEMBERVAR:
            checkMemberVar(node->u.varExp.var, fileTable,localTable, actClass, classTable,
                    globalTable, breakAllowed, tmpType, pass);
            break;
        default:
            error("You found a varExp that is not implemented! It's %d.",
                    node->type);
            break;
    }

    *returnType = *tmpType;
    free(tmpType);
}


static void checkSimpleVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Entry *varEntry;

    /* lookup the name of the simplevar and save its type */
    varEntry = lookup(localTable, node->u.simpleVar.name, ENTRY_KIND_VARIABLE);

    /* if no local variable is found
     * then check if there maybe a member variable */
    if ( varEntry == NULL ) {
        varEntry = lookupMember(actClass, node->u.simpleVar.name, ENTRY_KIND_VARIABLE);

        /* if it is no local and no member variable
         * check if is a class */
        if ( varEntry == NULL ) {
            varEntry = lookupClass(fileTable, globalTable, node->u.simpleVar.name);

            /* if it is still not found
             * then it really is not defined at */
            if (varEntry == NULL) {
                error("undefined variable '%s' in '%s' on line %d",
                        symToString(node->u.simpleVar.name),
                        node->file,
                        node->line);
            } else {
                *returnType = *(newStaticSimpleType(varEntry->u.classEntry.class));
            }
        } else {
            *returnType = *(varEntry->u.variableEntry.type);
        }
    } else {
        *returnType = *(varEntry->u.variableEntry.type);
    }
}


static void checkArrayVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Entry *varEntry;
    Type *indexType = allocate(sizeof(Type));
    Type *varType = allocate(sizeof(Type));
    Type *tmpType;

    Absyn *index = node->u.arrayVar.index;
    Absyn *varExp = node->u.arrayVar.var;
    Absyn *var = varExp->u.varExp.var;
    int dims;

    Entry *integerEntry = lookupClass(fileTable, globalTable, newSym("Integer"));
    Type *integerType = newSimpleType(integerEntry->u.classEntry.class);


    checkNode(index, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, indexType, pass);
    checkNode(var, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, varType, pass);

    if ( ! isSameOrSubtypeOf(indexType, integerType) ) {
        error("index must be an Integer in '%s' on line %d",
                node->file,
                node->line);
    }


    for(    dims = 1;
            var->type != ABSYN_SIMPLEVAR;
            varExp = var->u.arrayVar.var,
            var = varExp->u.varExp.var,
            dims++) {}

    varEntry = lookup(localTable, var->u.simpleVar.name, ENTRY_KIND_VARIABLE);

    if (varEntry->u.variableEntry.type->kind == TYPE_KIND_SIMPLE) {
        error("variable cannot be indexed in '%s' on line %d",
                node->file,
                node->line);
    }
    if (varEntry->u.variableEntry.type->kind == TYPE_KIND_ARRAY) {
        if (varEntry->u.variableEntry.type->u.arrayType.dims != dims ) {
            error("variable cannot be indexed in '%s' on line %d",
                    node->file,
                    node->line);
        }
    }

    free(integerType);
    free(indexType);
    free(varType);
}


static void checkMemberVar(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *object = node->u.memberVar.object;
    Sym *name = node->u.memberVar.name;
    Entry *varEntry;
    Type *objectType = allocate(sizeof(Type));
    Type *actClassType;


    /* determine type of object */
    checkNode(object, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, objectType, pass);

    if (objectType->kind == TYPE_KIND_VOID) {
        error("'void' does not have any fields in '%s' on line %d",
                node->file,
                node->line);
    }

    if (objectType->kind == TYPE_KIND_NIL) {
        error("'nil' does not have any fields in '%s' on line %d",
                node->file,
                node->line);
    }

    /* lookup the name of the membervar in object's class table */
    varEntry = lookupMember(objectType->u.simpleType.class, node->u.simpleVar.name, ENTRY_KIND_VARIABLE);

    if ( varEntry == NULL ) {
        error("field '%s' not found in '%s' on line %d",
                symToString(node->u.simpleVar.name),
                node->file,
                node->line);
    }


    if ( ! varEntry->u.variableEntry.isPublic ) {
        actClassType = newSimpleType(actClass);
/*        if ( ! isSameOrSubtypeOf(actClassType, varEntry->u.variableEntry.type) ) {*/
        if ( ! isSameOrSubtypeOf(actClassType, objectType) ) {
            error("field '%s' is not public in '%s' on line %d",
                    symToString(node->u.simpleVar.name),
                    node->file,
                    node->line);
        }
        free(actClassType);
    }

    if ( objectType->kind == TYPE_KIND_ARRAY ) {
        error("arrays do not have any fields in '%s' on line %d",
                node->file,
                node->line);
    }

    if ( varEntry->u.variableEntry.isStatic ) {
        /* if field is static
         * then the object type must be a static type */
        if ( ! objectType->isStatic ) {
            error("static field '%s' can only be used with a class as object in '%s' on line %d",
                    symToString(node->u.simpleVar.name),
                    node->file,
                    node->line);
        }
    } else {
        /* if field is not static 
         * then the objectType must be a non-static type */
        if ( objectType->isStatic ) {
/*        if ( ! isStaticTypeOf(objectType, varEntry->u.variableEntry.type ) ) {*/
            error("non-static field '%s' cannot be used with a class as object in '%s' on line %d",
                symToString(node->u.simpleVar.name),
                node->file,
                node->line);
        }

    }

    *returnType = *(varEntry->u.variableEntry.type);

    free(objectType);
}


static void checkIfStm1(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *test = node->u.ifStm1.test;
    Absyn *thenPart = node->u.ifStm1.thenPart;

    Type *testType = allocate(sizeof(Type));

    checkNode(test, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, testType, pass);

    if ( strcmp(testType->u.simpleType.class->name->string, "Boolean" ) ) {
        error("'if' test expression must be a Boolean in '%s' on line %d",
                node->file,
                node->line);
    }
    
    checkNode(thenPart, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, returnType, pass);

    free(testType);
}


static void checkIfStm2(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *test = node->u.ifStm1.test;
    Absyn *thenPart = node->u.ifStm2.thenPart;
    Absyn *elsePart = node->u.ifStm2.elsePart;

    Type *testType = allocate(sizeof(Type));

    checkNode(test, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, testType, pass);

    if ( strcmp(testType->u.simpleType.class->name->string, "Boolean" ) ) {
        error("'if' test expression must be a Boolean in '%s' on line %d",
                node->file,
                node->line);
    }

    checkNode(thenPart, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, returnType, pass);
    checkNode(elsePart, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, returnType, pass);

    free(testType);
}


static void checkWhileStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    Absyn *test = node->u.whileStm.test;
    Absyn *body = node->u.whileStm.body;

    Type *testType = allocate(sizeof(Type));

    checkNode(test, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, testType, pass);

    if ( strcmp(testType->u.simpleType.class->name->string, "Boolean" ) ) {
        error("'while' test expression must be a Boolean in '%s' on line %d",
                node->file,
                node->line);
    }

    checkNode(body, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, returnType, pass);

    free(testType);
}


static void checkDoStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    Absyn *test = node->u.doStm.test;
    Absyn *body = node->u.doStm.body;

    Type *testType = allocate(sizeof(Type));

    checkNode(test, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, testType, pass);

    if ( strcmp(testType->u.simpleType.class->name->string, "Boolean" ) ) {
        error("'do' test expression must be a Boolean in '%s' on line %d",
                node->file,
                node->u.doStm.test->line);
    }

    checkNode(body, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, returnType, pass);

    free(testType);
}


static void checkBreakStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    if ( ! breakAllowed ) {
        error("misplaced 'break' in '%s' on line %d",
                node->file,
                node->line);
    }
}


static void checkRetStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass){

    if (actMethod->u.methodEntry.retType->kind != TYPE_KIND_VOID) {
        error("return statement must return a value in '%s' on line %d",
                node->file,
                node->line);
    }
}


static void checkRetExpStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass){

    Type *actMethodRetType = actMethod->u.methodEntry.retType;
    Type *retExpStmRetType = allocate(sizeof(Type));

    checkNode(node->u.retExpStm.exp, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, retExpStmRetType, pass);

    
    if (actMethod->u.methodEntry.retType->kind == TYPE_KIND_VOID) {
        error("return statement must not return a value in '%s' on line %d",
                node->file,
                node->line);
    }

    /* return expression must be same or subtype of function return type
     * because expressions expecting an object A can only be served with an A
     * or a Subclass of A */
    if ( ! isSameOrSubtypeOf( retExpStmRetType, actMethodRetType)) {
        error("return type does not match method declaration in '%s' on line %d",
                node->file,
                node->line);
    }

    free(retExpStmRetType);
}


static void checkCallStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Type *rcvrType = allocate(sizeof(Type));
    Absyn *rcvrNode = node->u.callStm.rcvr;
    Entry *methodEntry;
    Entry *tmpEntry;
    int i;
    TypeList *paramList;
    TypeList *argList;
    Absyn *args;
    Type *argType = allocate(sizeof(Type));
    Type *paramType;

    checkNode(rcvrNode, fileTable,localTable, actClass, classTable,
            globalTable, breakAllowed, rcvrType, pass);

    if (rcvrType->kind == TYPE_KIND_VOID) {
        error("'void' does not have any methods in '%s' on line %d",
                node->file,
                node->line);
    }

    if (rcvrType->kind == TYPE_KIND_NIL) {
        error("'nil' does not have any methods in '%s' on line %d",
                node->file,
                node->line);
    }

    if ( actMethod->u.methodEntry.isStatic ) {
        if ( rcvrNode->type == ABSYN_SELFEXP ) {
            error("the current receiver 'self' is not available in '%s' on line %d",
                node->file,
                node->line);
        }
        if ( rcvrNode->type == ABSYN_SUPEREXP ) {
            error("the current receiver 'super' is not available in '%s' on line %d",
                node->file,
                node->line);
        }
    }

    /* check if the receiver is an arrayVar
     * and if another method then 'length()' is called
     * then the method cannot be called */
    if ( rcvrNode->type == ABSYN_VAREXP ) {
        if (rcvrNode->u.varExp.var->type == ABSYN_SIMPLEVAR) {
            tmpEntry = lookup(localTable, rcvrNode->u.varExp.var->u.simpleVar.name, ENTRY_KIND_VARIABLE);

            if (tmpEntry != NULL) {
                if (tmpEntry->u.variableEntry.type->kind == TYPE_KIND_ARRAY) {
                    if (strcmp("length", node->u.callStm.name->string) != 0) {
                        error("arrays do not have any methods other than 'length' in '%s' on line %d",
                                node->file,
                                node->line);
                    } else {
                        if (!node->u.callStm.args->u.expList.isEmpty) {
                            error("call to method 'length' of arrays must not have any arguments in '%s' on line %d",
                                node->file,
                                node->line);
                        }
                    }
                }
            }
        }
    }

    /* lookup the method name in the receiver's member table */
    methodEntry = lookup(rcvrType->u.simpleType.class->mbrTable, node->u.callStm.name, ENTRY_KIND_METHOD);
    if (methodEntry == NULL) {
        error("method '%s' not found in '%s' on line %d",
                node->u.callStm.name->string,
                node->file,
                node->line);
    }
    
    /* if actual class is not same or subclass of receivers's class
     * then the method must be public
     * otherwise it cannot be seen in this context */
    if ( ! methodEntry->u.methodEntry.isPublic) {
        if ( ! isSameOrSubclassOf(actClass, rcvrType->u.simpleType.class) ) {
            error("method '%s' is not public in '%s' on line %d",
                    node->u.callStm.name->string,
                    node->file,
                    node->line);
        }
    }

    if ( methodEntry->u.methodEntry.isStatic ) {
        /* if the method is static and the receiver is not a class
         * then the method cannot be called */
        if ( rcvrNode->type == ABSYN_VAREXP ) {
            if (rcvrNode->u.varExp.var->type == ABSYN_SIMPLEVAR) {
                tmpEntry = lookupClass(fileTable,
                        globalTable,
                        rcvrNode->u.varExp.var->u.simpleVar.name);

                if (tmpEntry == NULL) {
                    error("static method '%s' can only be called with a class as receiver in '%s' on line %d",
                        node->u.callStm.name->string,
                        node->file,
                        node->line);
                }
            } else {
                error("error in CallStm: the receiver is no simpleVar");
            }
        } else {
            error("error in CallStm: the receiver is no varExp");
        }
    } else {
        /* if method is not static and the receiver is a class name
         * then the method cannot be called */
        if ( rcvrNode->type == ABSYN_VAREXP ) {
            if (rcvrNode->u.varExp.var->type == ABSYN_SIMPLEVAR) {
                tmpEntry = lookupClass(fileTable,
                        globalTable,
                        rcvrNode->u.varExp.var->u.simpleVar.name);

                if (tmpEntry != NULL) {
                    error("non-static method '%s' cannot be called with a class as receiver in '%s' on line %d",
                        node->u.callStm.name->string,
                        node->file,
                        node->line);
                }
            }
        }
    }

    /* check arguments */
    /* Loop over the args and paramtypes and compare their types */
    
    paramList = methodEntry->u.methodEntry.paramTypes;
    args = node->u.callStm.args;

    for( i = 0 ; !paramList->isEmpty &&  !args->u.expList.isEmpty ; i++ ) {
        /* body -> prolog */
        /* 1. Type: determine directly */
        paramType = paramList->type;

        /* 2. Type: determine with checkNode */
        checkNode(args->u.expList.head, fileTable, localTable, actClass, classTable,
                globalTable, breakAllowed, argType, pass);

        /* body -> main part */
        /* if one argument is not same or subtype of param then the method
         * cannot be called */
        if(!isSameOrSubtypeOf(argType, paramType)) {
            error("method '%s' called with incompatible argument type (argument #%d) in '%s' on line %d",
                    node->u.callStm.name->string,
                    getLength(methodEntry->u.methodEntry.paramTypes) - i,
                    node->file,
                    node->line);
        }

        /* body -> epilog*/
        paramList = paramList->next;
        args = args->u.expList.tail;
    }

    /* check if params and args count are the same */
    if ( ! (args->u.expList.isEmpty && paramList->isEmpty) ) {
        if ( args->u.expList.isEmpty ) {
            error("method '%s' called with too few arguments in '%s' on line %d",
                    node->u.callStm.name->string,
                    node->file,
                    node->line);
        }

        if ( paramList->isEmpty ) {
            error("method '%s' called with too many arguments in '%s' on line %d",
                    node->u.callStm.name->string,
                    node->file,
                    node->line);
        }
    }

    free(rcvrType);
    free(argType);
}


static void checkBinOpExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Entry *booleanEntry = lookupClass(fileTable, globalTable, newSym("Boolean"));
    Type *booleanType = newSimpleType(booleanEntry->u.classEntry.class);
    Entry *integerEntry = lookupClass(fileTable, globalTable, newSym("Integer"));
    Type *integerType = newSimpleType(integerEntry->u.classEntry.class);
    Entry *characterEntry = lookupClass(fileTable, globalTable, newSym("Character"));
    Type *characterType = newSimpleType(characterEntry->u.classEntry.class);

    int op = node->u.binopExp.op;
    Absyn *left  = node->u.binopExp.left;
    Absyn *right = node->u.binopExp.right;

    Type *leftType  = allocate(sizeof(Type));
    Type *rightType = allocate(sizeof(Type));

    /* determine type of left */
    checkNode(left, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, leftType, pass);
    
    /* determine type of right*/
    checkNode(right, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, rightType, pass);


    switch(op) {
        case ABSYN_BINOP_LOR:
        case ABSYN_BINOP_LAND:
            /* check if left operand is boolean*/
            if (leftType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(leftType, booleanType) ) {
                    error("left operand of boolean expression must be a Boolean in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("left operand of boolean expression must be a Boolean in '%s' on line %s",
                        node->file,
                        node->line);
            }

            /* check if right operand is boolean*/
            if (rightType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(rightType, booleanType) ) {
                    error("right operand of boolean expression must be a Boolean in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("right operand of boolean expression must be a Boolean in '%s' on line %s",
                        node->file,
                        node->line);
            }

            *returnType = *booleanType;

            break;
        case ABSYN_BINOP_EQ:
        case ABSYN_BINOP_NE:
        case ABSYN_BINOP_LT:
        case ABSYN_BINOP_LE:
        case ABSYN_BINOP_GT:
        case ABSYN_BINOP_GE:
            /* check if left operand is character or integer */
            if (leftType->kind == TYPE_KIND_SIMPLE) {
                if ( ! ( isSameOrSubtypeOf(leftType, integerType)
                         || isSameOrSubtypeOf(leftType, characterType)
                       ) ) {
                    error("left operand of comparison must be an Integer or a Character in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("left operand of comparison must be an Integer or a Character in '%s' on line %d",
                        node->file,
                        node->line);
            }

            /* if left operand is integer check if right operand is integer */
            if ( isSameOrSubtypeOf(leftType, integerType) ) {
                if (rightType->kind != TYPE_KIND_SIMPLE) {
                    error("right operand of comparison must be an Integer in '%s' on line %d",
                            node->file,
                            node->line);
                }
                
                if ( !isSameOrSubtypeOf(rightType, integerType) ) {
                    error("right operand of comparison must be an Integer in '%s' on line %d",
                            node->file,
                            node->line);
                }
            }

            /* if left operand is character check if right operand is character */
            if ( isSameOrSubtypeOf(leftType, characterType) ) {
                if (rightType->kind != TYPE_KIND_SIMPLE) {
                    error("right operand of comparison must be a Character in '%s' on line %d",
                            node->file,
                            node->line);
                }

                if ( !isSameOrSubtypeOf(rightType, characterType) ) {
                    error("right operand of comparison must be a Character in '%s' on line %d",
                            node->file,
                            node->line);
                }
            }

            *returnType = *booleanType;

            break;
        case ABSYN_BINOP_ADD:
        case ABSYN_BINOP_SUB:
        case ABSYN_BINOP_MUL:
        case ABSYN_BINOP_DIV:
        case ABSYN_BINOP_MOD:
            /* check if left operand is integer */
            if (leftType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(leftType, integerType) ) {
                    error("left operand of arithmetic expression must be an Integer in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("left operand of arithmetic expression must be an Integer in '%s' on line %d",
                        node->file,
                        node->line);
            }
            
            /* check if right operand is integer */
            if (rightType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(rightType, integerType) ) {
                    error("right operand of arithmetic expression must be an Integer in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("right operand of arithmetic expression must be an Integer in '%s' on line %d",
                        node->file,
                        node->line);
            }

            *returnType = *integerType;

            break;
        default:
            error("You found an unexspected BinOp: %d", op);
            break;
    }

    free(leftType);
    free(rightType);
}

static void checkUnOpExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    
    Entry *integerEntry = lookupClass(fileTable, globalTable, newSym("Integer"));
    Type *integerType = newSimpleType(integerEntry->u.classEntry.class);
    Entry *booleanEntry = lookupClass(fileTable, globalTable, newSym("Boolean"));
    Type *booleanType = newSimpleType(booleanEntry->u.classEntry.class);

    int op = node->u.unopExp.op;
    Absyn *right = node->u.unopExp.right;

    Type *rightType = allocate(sizeof(Type));

    switch(op) {
        case ABSYN_UNOP_LNOT:
            /* determine type of right*/
            checkNode(right, fileTable, localTable, actClass, classTable,
                    globalTable, breakAllowed, rightType, pass);

            /* check if right operand is integer */
            if (rightType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(rightType, booleanType) ) {
                    error("operand of unary '!' must be a Boolean in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("operand of unary '!' must be a Boolean in '%s' on line %d",
                        node->file,
                        node->line);
            }

            *returnType = *booleanType;
            break;
        case ABSYN_UNOP_MINUS:
        case ABSYN_UNOP_PLUS:
            /* determine type of right*/
            checkNode(right, fileTable, localTable, actClass, classTable,
                    globalTable, breakAllowed, rightType, pass);

            /* check if right operand is integer */
            if (rightType->kind == TYPE_KIND_SIMPLE) {
                if ( ! isSameOrSubtypeOf(rightType, integerType) ) {
                    error("operand of unary '+' or '-' must be an Integer in '%s' on line %d",
                            node->file,
                            node->line);
                }
            } else {
                error("operand of unary '+' or '-' must be an Integer in '%s' on line %d",
                        node->file,
                        node->line);
            }

            *returnType = *integerType;
            break;
        default:
            error("You found an unexspected UnOp: %d", op);
            break;
    }
    
    free(integerType);
    free(booleanType);
    free(rightType);
}

static void checkInstanceOfExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *exp = node->u.instofExp.exp;
    Absyn *type = node->u.instofExp.type;
    
    Type expType;
    Type typeType;

    /* determine type of exp */
    checkNode(exp, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, &expType, pass);
    /* determine type of type */
    checkNode(type, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, &typeType, pass);

    if( ! isStaticTypeOf(&typeType, &expType)) {
        error("'instanceof' has inconvertible types in '%s' on line %d",
                node->file,
                node->line);
    }
    
}


static void checkCastExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Absyn *exp = node->u.castExp.exp;
    Absyn *type = node->u.instofExp.type;

    Type expType;
    Type typeType;

    /* determine type of exp */
    checkNode(exp, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, &expType, pass);
    /* determine type of type */
    checkNode(type, fileTable, localTable, actClass, classTable,
            globalTable, breakAllowed, &typeType, pass);

    if( ! isStaticTypeOf(&typeType, &expType) ) {
        error("'castto' has inconvertible types in '%s' on line %d",
                node->file,
                node->line);
    }

}


static void checkNilExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    
    Type *nilType = newNilType();
    *returnType = *nilType;
}


static void checkIntExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    Entry *integerEntry = lookupClass(fileTable, globalTable, newSym("Integer"));
    Type *integerType = newSimpleType(integerEntry->u.classEntry.class);
    *returnType = *integerType;
}


static void checkBoolExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    Entry *booleanEntry = lookupClass(fileTable, globalTable, newSym("Boolean"));
    Type *booleanType = newSimpleType(booleanEntry->u.classEntry.class);
    *returnType = *booleanType;
}


static void checkCharExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    Entry *characterEntry = lookupClass(fileTable, globalTable, newSym("Character"));
    Type *characterType = newSimpleType(characterEntry->u.classEntry.class);
    *returnType = *characterType;
}


static void checkSelfExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    /* lookup and return type of selfExp */
    Entry *tmpEntry = lookupClass(fileTable, globalTable, actClass->name);
    Type *tmpType = newSimpleType(tmpEntry->u.classEntry.class);
    *returnType = *tmpType;
}


static void checkSuperExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    /* lookup and return type of superExp */
    Entry *tmpEntry = lookupClass(fileTable, globalTable, actClass->superClass->name);
    Type *tmpType = newSimpleType(tmpEntry->u.classEntry.class);
    *returnType = *tmpType;
    free(tmpType);
}


static void checkNewExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    /* ToDo: here could be checks for checking the arguments of the newExp matches
     * the constructor of the class, but I'm to tired right now.
     * so just return the type of the class... */
    Entry *tmpEntry = lookupClass(fileTable, globalTable, node->u.newExp.type);
    Type *tmpType = newSimpleType(tmpEntry->u.classEntry.class);
    *returnType = *tmpType;
}

static void checkNewArrayExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    /* ToDo: here could be checks for checking the arguments of the newExp matches
     * the constructor of the class, but I'm to tired right now.
     * so just return the type of the class... */
    Entry *tmpEntry = lookupClass(fileTable, globalTable, node->u.newArrayExp.type);
    Type *tmpType = newArrayType(tmpEntry->u.classEntry.class, node->u.newArrayExp.dims);
    *returnType = *tmpType;
}

static void checkCallExp(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {

    Entry *callExpMethodEntry = lookup(classTable, node->u.callExp.name, ENTRY_KIND_METHOD);
    Type *callExpType = callExpMethodEntry->u.methodEntry.retType;

    *returnType = *callExpType;

    /*
     * node->u.callExp.args
     * node->u.callExp.name
     * node->u.callExp.rcvr
     */
}

static void checkAsmStm(
        Absyn *node,
        Table **fileTable,
        Table *localTable,
        Class *actClass,
        Table *classTable,
        Table *globalTable,
        boolean breakAllowed,
        Type *returnType,
        int pass) {
    /* we just trust it */
}


Table **check(Absyn *fileTrees[], int numInFiles, boolean showSymbolTables) {
    /* initialize tables and foobars */
    Table *globalTable;
    Table *fileTable;
    Table *classTable;
    Table **fileTables;
    Class *objectClass;
    Class *objectMetaClass;
    Class *integerClass;
    Class *integerMetaClass;
    Class *booleanClass;
    Class *booleanMetaClass;
    Class *characterClass;
    Class *characterMetaClass;
    Entry *objectEntry;
    Entry *objectMetaEntry;
    Entry *integerEntry;
    Entry *integerMetaEntry;
    Entry *booleanEntry;
    Entry *booleanMetaEntry;
    Entry *characterEntry;
    Entry *characterMetaEntry;
    Entry *mainClassEntry;
    Entry *mainClassMetaEntry;
    Entry *mainMethodEntry;

    Type *returnType = allocate(sizeof(Type));

    int i;

    /* Initialize trivial Classes */
    globalTable = newTable(NULL);

    objectMetaClass = newClass(TRUE, newSym("$Object"), NULL, NULL, NULL, newTable(globalTable));
    objectClass = newClass(TRUE, newSym("Object"), NULL, NULL, objectMetaClass, newTable(globalTable));
    objectMetaClass->vmt = newEmptyVMT();
    objectClass->vmt = newEmptyVMT();
    objectMetaClass->attibuteList = newEmptyInstanceVar();
    objectClass->attibuteList = newEmptyInstanceVar();
    objectMetaEntry = newClassEntry(objectMetaClass);
    objectEntry = newClassEntry(objectClass);
    enter(globalTable, objectMetaClass->name, objectMetaEntry);
    enter(globalTable, objectClass->name, objectEntry);

    integerMetaClass = newClass(TRUE, newSym("$Integer"), NULL, NULL, NULL, newTable(globalTable));
    integerClass = newClass(TRUE, newSym("Integer"), NULL, NULL, integerMetaClass, newTable(globalTable));
    integerMetaClass->vmt = newEmptyVMT();
    integerClass->vmt = newEmptyVMT();
    integerMetaClass->attibuteList = newEmptyInstanceVar();
    integerClass->attibuteList = newEmptyInstanceVar();
    integerEntry = newClassEntry(integerClass);
    integerMetaEntry = newClassEntry(integerMetaClass);
    enter(globalTable, integerMetaClass->name, integerMetaEntry);
    enter(globalTable, integerClass->name, integerEntry);

    booleanMetaClass = newClass(TRUE, newSym("$Boolean"), NULL, NULL, NULL, newTable(globalTable));
    booleanClass = newClass(TRUE, newSym("Boolean"), NULL, NULL, booleanMetaClass, newTable(globalTable));
    booleanMetaClass->vmt = newEmptyVMT();
    booleanClass->vmt = newEmptyVMT();
    booleanMetaClass->attibuteList = newEmptyInstanceVar();
    booleanClass->attibuteList = newEmptyInstanceVar();
    booleanMetaEntry = newClassEntry(booleanMetaClass);
    booleanEntry = newClassEntry(booleanClass);
    enter(globalTable, booleanMetaClass->name, booleanMetaEntry);
    enter(globalTable, booleanClass->name, booleanEntry);

    characterMetaClass = newClass(TRUE, newSym("$Character"), NULL, NULL, NULL, newTable(globalTable));
    characterClass = newClass(TRUE, newSym("Character"), NULL, NULL, characterMetaClass, newTable(globalTable));
    characterMetaClass->vmt = newEmptyVMT();
    characterClass->vmt = newEmptyVMT();
    characterMetaClass->attibuteList = newEmptyInstanceVar();
    characterClass->attibuteList = newEmptyInstanceVar();
    characterMetaEntry = newClassEntry(characterMetaClass);
    characterEntry = newClassEntry(characterClass);
    enter(globalTable, characterMetaClass->name, characterMetaEntry);
    enter(globalTable, characterClass->name, characterEntry);

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
                globalTable, FALSE, returnType, 3);
    }

    /* fifth pass: make virtual method tables */
    for(i = 0; i < numInFiles; i++) {
         checkNode(fileTrees[i], &(fileTables[i]), NULL, NULL, NULL,
                globalTable, FALSE, returnType, 4);
    }

    /* sixth pass: generate offsets for instance variables */

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
        mainMethodEntry = lookupMember(mainClassEntry->u.classEntry.class,
                newSym("main"), ENTRY_KIND_METHOD);
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
                error("method 'main' of public class '%s' must have return type 'void'", mainClass);
            }
            if ( ! mainMethodEntry->u.methodEntry.paramTypes->isEmpty ) {
                error("method 'main' of public class '%s' must not have any arguments", mainClass);
            }
        }
    }

    free(returnType);
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

