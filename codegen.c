#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "vmt.h"
#include "types.h"
#include "absyn.h"
#include "instance.h"
#include "table.h"
#include "codegen.h"

static FILE *asmFile;
static ClassList *metaClasses;

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


/*
 * Absyn *node: LHS of expression
 * Absyn *exp: RHS of expression
 * 
 */
static void handleVar(Absyn *node, Absyn *exp, Table *table, Entry *currentMethod, int returnLabel, int breakLabel, boolean write) {
  switch(node->type) {
    case ABSYN_SIMPLEVAR: {
      Entry *entry;
      entry = lookup(table, node->u.simpleVar.name, ENTRY_KIND_VARIABLE | ENTRY_KIND_CLASS);
      if (entry == NULL) {
        error("variable/class declaration of '%s' vanished from symbol table",
            symToString(node->u.simpleVar.name));
      }
      if(entry->kind == ENTRY_KIND_CLASS) {
        if(exp) {
          generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);
        }
        fprintf(asmFile, "\tpushg\t%d\n", entry->u.classEntry.class->globalIndex);
      }
      /* "self." is optional */
      else if(entry->u.variableEntry.isLocal) {
        if(exp) {
          generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);
        }
        fprintf(asmFile, "\t%s\t%d\n", write ? "popl" : "pushl", entry->u.variableEntry.offset);
      }
      else {
        /* push self */
        fprintf(asmFile, "\tpushl\t%d\n", -3 - currentMethod->u.methodEntry.numParams);
        /* push value */
        if(exp) {
          generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);
        }
        /* access field */
        fprintf(asmFile, "\t%s\t%d\n", write ? "putf" : "getf", entry->u.variableEntry.offset);
      }
    }
    break;
 
    case ABSYN_MEMBERVAR: {
      /* Need to show the class */
      Entry *fieldEntry = lookup(node->u.memberVar.objectClass->mbrTable,
          node->u.memberVar.name, ENTRY_KIND_VARIABLE);
      if (fieldEntry == NULL) {
        error("field declaration vanished from symbol table");
      }
      generateCodeNode(node->u.memberVar.object, table, currentMethod, returnLabel, breakLabel);
      if(exp) {
        generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);
      }
      fprintf(asmFile, "\t%s\t%d\n", write ? "putf" : "getf", fieldEntry->u.variableEntry.offset);
    }
    break;
 
    case ABSYN_ARRAYVAR: {
      generateCodeNode(node->u.arrayVar.var, table, currentMethod, returnLabel, breakLabel);
      generateCodeNode(node->u.arrayVar.index, table, currentMethod, returnLabel, breakLabel);
      if(exp) {
        generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);
      }
      fprintf(asmFile, "\t%s\n", write ? "putfa" : "getfa");
    }
    break;
 
    default:
      error("Invalid var-type");
      break;
  }
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
    Class *class;
    Class *metaClass;

    Entry *classEntry = lookupClass(&table, (table)->outerScope, node->u.classDec.name);
    class = classEntry->u.classEntry.class;
    metaClass = class->metaClass;

    /* Add meta class to the list of meta classes for _prolog generation */
    metaClasses = newClassList(metaClasses, metaClass);

    fprintf(asmFile, "// Metaclass \"%s\"\n", metaClass->name->string);
    fprintf(asmFile, "%s_%lx:\n", metaClass->name->string, djb2(node->file));
    if (strcmp(metaClass->name->string, "$Object") == 0) {
        fprintf(asmFile, ".addr\tnil\n");
    } else {
        fprintf(asmFile, ".addr\t%s_%lx\n", metaClass->superClass->name->string, djb2(metaClass->superClass->fileName));
    }
    printVMT(asmFile, metaClass->vmt);

    fprintf(asmFile, "// Class \"%s\"\n", node->u.classDec.name->string);
    fprintf(asmFile, "%s_%lx:\n", node->u.classDec.name->string, djb2(node->file));
    if (strcmp(node->u.classDec.name->string, "Object") == 0) {
        fprintf(asmFile, ".addr\tnil\n");
    } else {
        fprintf(asmFile, ".addr\t%s_%lx\n", classEntry->u.classEntry.class->superClass->name->string, djb2(classEntry->u.classEntry.class->superClass->fileName));
    }
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
    generateCodeNode(node->u.asmStm.instrList, table, currentMethod, returnLabel, breakLabel);
}

static void generateCodeAsmInstrList(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Absyn *instrList;
    Absyn *instrDec;

    instrList = node;

    if (!instrList->u.asmInstrList.isEmpty) {
        for (instrDec = instrList->u.asmInstrList.head;
                instrList->u.asmInstrList.isEmpty == FALSE;
                instrList = instrList->u.asmInstrList.tail,
                instrDec = instrList->u.asmInstrList.head) {

            generateCodeNode(instrDec, table, currentMethod, returnLabel, breakLabel);
        }
    }
}

static void generateCodeAsmInstr0(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    fprintf(asmFile, "\t%s\n", node->u.asmInstr0.instr);
}

static void generateCodeAsmInstr1(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    fprintf(asmFile, "\t%s\t%d\n", node->u.asmInstr1.instr, node->u.asmInstr1.immediate);
}

static void generateCodeAsmInstr2(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    fprintf(asmFile, "\t%s\t%d,%d\n", node->u.asmInstr2.instr, node->u.asmInstr2.numArgs, node->u.asmInstr2.offset);
}

static void generateCodeAsmInstr3(Absyn *node, Table *table, Entry *currentMethod, int returnLabel, int breakLabel) {
    Entry *classEntry;
    Class *class;
    if (strcmp(node->u.asmInstr3.instr, ".addr") == 0) {
        classEntry = lookup(table, newSym(node->u.asmInstr3.label), ENTRY_KIND_CLASS);
        class = classEntry->u.classEntry.class;
        fprintf(asmFile, "\t%s\t%s_%lx\n", node->u.asmInstr3.instr, node->u.asmInstr3.label, djb2(class->fileName));
    } else {
        fprintf(asmFile, "\t%s\t%s\n", node->u.asmInstr3.instr, node->u.asmInstr3.label);
    }

}

static void generateCodeEmptyStm(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    /* nothing to generate here */
}

static void generateCodeCompStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    generateCodeNode(node->u.compStm.stms, table, currentMethod, returnLabel, breakLabel);
}

static Sym *getVarName(Absyn *varNode) {
    while (varNode->type != ABSYN_SIMPLEVAR) {
        switch (varNode->type) {
            case ABSYN_ARRAYVAR:
            {
                varNode = varNode->u.arrayVar.var->u.varExp.var;
                break;
            }
            case ABSYN_VAREXP:
            {
                varNode = varNode->u.varExp.var;
                break;
            }
            case ABSYN_MEMBERVAR:
            {
                return varNode->u.memberVar.name;
                break;
            }
            default:
            {
                printf("Error Type %d\n", varNode->type);
                shouldNotReach("getVarName got some unknown Var-Type");
            }
        }
    }
    return varNode->u.simpleVar.name;
}

static void generateCodeAssignStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    handleVar(node->u.assignStm.var, node->u.assignStm.exp, table, currentMethod, returnLabel, breakLabel, TRUE);
/*
    Absyn *varNode = node->u.assignStm.var;
    Absyn *exp = node->u.assignStm.exp;
    Sym *name = getVarName(varNode);

    Entry *entry = lookup(currentMethod->u.methodEntry.localTable, name, ENTRY_KIND_VARIABLE);

    if (entry != NULL) {
        generateCodeNode(exp, table, currentMethod, returnLabel, breakLabel);        
    }
*/
    /* Depending on the receiver of the value we need
     * to do a different evaluation.
     */
/*    fprintf(asmFile, "\tpopl\t%d\n", entry->u.variableEntry.offset);*/
            
    /*    Absyn *varVar = var->u.varExp.var;*/
    /*    Type *varType = var->u.varExp.expType;*/
    /*    Entry *varEntry;*/


    /*    ->u.varExp.var;
        node->u.assignStm.exp;node->u.assignStm.var->u.varExp.var
        ->type;
     */ /* ToDo: Assignment, field/member/local variable lookup */

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

static Type *getExpType(Absyn *expNode) {
    switch (expNode->type) {
        case ABSYN_VAREXP:
            return expNode->u.varExp.expType;
        default:
            error("you found an invalid node in getExpType in types.c!");
            /* Won't return but GCC complains otherwise */
            return NULL;
            break;
    }
}

static void generateCodeCallStmt(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    Absyn *rcvr = node->u.callStm.rcvr;
    Absyn *args = node->u.callStm.args;
    Sym *name = node->u.callStm.name;
    Type *rcvrType = getExpType(rcvr);
    Class *rcvrClass = node->u.callStm.rcvrClass;
    Entry *methodEntry;
    int offset;
    int numParams;
    
    methodEntry = lookupMember(rcvrClass, name, ENTRY_KIND_METHOD);
    
    if (methodEntry->u.methodEntry.isStatic) {
        /* rcvrClass == rcvrMetaclass */
        fprintf(asmFile, "\tpushg\t%d\n", rcvrClass->globalIndex);
        
        offset = findVMT(rcvrClass->vmt, name);
        numParams = methodEntry->u.methodEntry.numParams;

        generateCodeNode(args, table, currentMethod, returnLabel, breakLabel);

        fprintf(asmFile, "\tvmcall\t%d,%d\n",
                numParams + 1,
                offset + 1);
    } else {
        generateCodeNode(rcvr, table, currentMethod, returnLabel, breakLabel);
    }

    /* ToDo */

}

static void generateCodeSuperExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    shouldNotReach("SuperExp");
}

static void generateVarExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {
    Sym *varName = getVarName(node);
    Entry *varEntry = lookup(currentMethod->u.methodEntry.localTable, varName, ENTRY_KIND_VARIABLE);
    if (varEntry != NULL) {
        fprintf(asmFile, "\tpushl\t%d\n", varEntry->u.variableEntry.offset);
    }
}

static void generateCodeCallExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Entry *methodEntry; 
    int offset;

    /* We need to switch over the receiver of the callExp */
    switch (node->u.callExp.rcvr->type) {
        case ABSYN_SUPEREXP:
            break;
        case ABSYN_SELFEXP:
            break;
        default:
            generateCodeNode(node->u.callExp.rcvr, table, currentMethod, returnLabel, breakLabel);
            break;
    }
    methodEntry = lookup(node->u.callExp.rcvrClass->mbrTable, node->u.callExp.name, ENTRY_KIND_METHOD);
    if(methodEntry == NULL) {
        methodEntry = lookup(node->u.callExp.rcvrClass->metaClass->mbrTable, node->u.callExp.name, ENTRY_KIND_METHOD);
    }
    
    generateCodeNode(node->u.callExp.args, table, currentMethod, returnLabel, breakLabel);
    offset = findVMT(node->u.callExp.rcvrClass->vmt, node->u.callExp.name);
    fprintf(asmFile, "\tvmcall\t%d,%d\n", methodEntry->u.methodEntry.numParams + 1, offset + 1);
    fprintf(asmFile, "\tpushr\n");
}

static void generateCodeNewExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    int numFields;
    int offset;
    Class *class;
    Type *type;
    Entry *entry;

    /* Fetch type and number of fields*/
    type = node->u.newExp.expType;
    numFields = type->u.simpleType.class->numFields;
    class = type->u.simpleType.class;

    /* new Object with numFields fields */
    fprintf(asmFile, "\tnew\t%d\n", numFields);
    fprintf(asmFile, "\t.addr %s_%lx\n", class->name->string, djb2(class->fileName));

    /* Look for a method with the same name of the class in the metaclass => constructor */
    entry = lookupMember(class->metaClass, class->name, ENTRY_KIND_METHOD);

    /* Did we find a constructor? */
    if (entry != NULL) {
        /* Generate code for arguments */
        generateCodeNode(node->u.newExp.args, table, currentMethod, returnLabel, breakLabel);
        offset = findVMT(class->metaClass->vmt, class->name);
        fprintf(asmFile, "\tpushg\t%d\n", class->metaClass->globalIndex);
        fprintf(asmFile, "\tvmcall\t%d,%d\n", entry->u.methodEntry.numParams + 1, offset + 1);
    }
}

static void generateCodeNewArrayExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Entry *classEntry = lookup(table, node->u.newArrayExp.type, ENTRY_KIND_CLASS);

    generateCodeNode(node->u.newArrayExp.size, table, currentMethod, returnLabel, breakLabel);

    fprintf(asmFile, "\tnewa\n");
    fprintf(asmFile, "\t.addr %s_%lx\n", node->u.newArrayExp.type->string, djb2(classEntry->u.classEntry.class->fileName));
}

static void generateCodeUnopExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Entry *entry;
    Class *class;
    switch (node->u.unopExp.op) {
        case ABSYN_UNOP_PLUS:
            generateCodeNode(node->u.unopExp.right, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_UNOP_MINUS:
            /* First we need to create the target object */
            entry = lookup(table, newSym("Integer"), ENTRY_KIND_CLASS);
            class = entry->u.classEntry.class;

            /* Generate new Integer object and duplicate it */
            fprintf(asmFile, "\tnew\t%d\n", 2);
            fprintf(asmFile, "\t.addr\t%s_%lx\n", class->name->string, djb2(class->fileName));
            fprintf(asmFile, "\tdup\n");

            /* Put the constant 0 on the stack */
            fprintf(asmFile, "\tpushc\t0\n");

            /* After this expression we should have an Integer object on the stack */
            generateCodeNode(node->u.unopExp.right, table, currentMethod, returnLabel, breakLabel);

            /* Get the first field containing the Integer */
            fprintf(asmFile, "\tgetf\t%d\n", 1);

            /* Substract value on stack */
            fprintf(asmFile, "\tsub\n");

            /* put the value on the stack into the first field */
            fprintf(asmFile, "\tputf\t%d\n", 1);
            break;
        case ABSYN_UNOP_LNOT:
            /* First we need to create the target object */
            entry = lookup(table, newSym("Boolean"), ENTRY_KIND_CLASS);
            class = entry->u.classEntry.class;

            /* Generate new Boolean object and duplicate it */
            fprintf(asmFile, "\tnew\t%d\n", 2);
            fprintf(asmFile, "\t.addr\t%s_%lx\n", class->name->string, djb2(class->fileName));
            fprintf(asmFile, "\tdup\n");

            /* Put the constant 0 on the stack */
            fprintf(asmFile, "\tpushc\t1\n");

            /* After this expression we should have an Integer object on the stack */
            generateCodeNode(node->u.unopExp.right, table, currentMethod, returnLabel, breakLabel);

            /* Get the first field containing the Integer */
            fprintf(asmFile, "\tgetf\t%d\n", 1);

            /* Substract value on stack */
            fprintf(asmFile, "\tsub\n");

            /* put the value on the stack into the first field */
            fprintf(asmFile, "\tputf\t%d\n", 1);
            break;
        default:
            error("unknown unary operator %d in generateCodeUnopExp",
                    node->u.unopExp.op);
    }
}

static void generateCodeIntExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Entry *integerEntry;
    Class *integerClass;

    integerEntry = lookup(table, newSym("Integer"), ENTRY_KIND_CLASS);
    integerClass = integerEntry->u.classEntry.class;

    /* Generate new Integer object and duplicate it */
    fprintf(asmFile, "\tnew\t%d\n", 2);
    fprintf(asmFile, "\t.addr\t%s_%lx\n", integerClass->name->string, djb2(integerClass->fileName));
    fprintf(asmFile, "\tdup\n");

    /* Push the value of the intExp */
    fprintf(asmFile, "\tpushc\t%d\n", node->u.intExp.value);

    /* put the value on the stack into the first field */
    fprintf(asmFile, "\tputf\t%d\n", 1);
}

static void generateCodeInstofExp(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Type *type;

    /* Generate code for the left side expression (e.g. exp instanceof Type) */
    generateCodeNode(node->u.instofExp.exp, table, currentMethod, returnLabel, breakLabel);

    type = node->u.instofExp.expType;
    /* ToDO */
    /* fprintf(asmFile, "\tpushg\t%d\n", <position of type->metaclass>)); */
    switch (type->kind) {
        case TYPE_KIND_SIMPLE:

            break;
        case TYPE_KIND_ARRAY:
            break;
        default:
            error("Wrong kind in generateCodeInstofExp!");
            break;
    }
    fprintf(asmFile, "\tinstof\n");
}

static void generateCodeExpList(Absyn *node, Table *table, Entry *currentMethod,
        int returnLabel, int breakLabel) {

    Absyn *expList;

    expList = node;

    while (!expList->u.expList.isEmpty) {
        generateCodeNode(expList->u.expList.head, table, currentMethod, returnLabel, breakLabel);
        expList = expList->u.expList.tail;
    }
}

static void generateProlog(Table* table) {
    Entry* mainClass = lookup(table, newSym("$Main"), ENTRY_KIND_CLASS);

    /* execution framework */
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "// execution framework\n");
    fprintf(asmFile, "//\n");
    fprintf(asmFile, "_start:\n");
    fprintf(asmFile, "\tcall\t_init\n");
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

static void generateCodeMetaClasses(void) {
    ClassList* currentClassList;
    Class* currentClass;

    /* Generate init */
    fprintf(asmFile, "_init:\n");
    currentClassList = metaClasses;
    while (!currentClassList->isEmpty) {
        currentClass = currentClassList->head;
        fprintf(asmFile, "\t// Generate Metaclass object \"%s\"\n", currentClass->name->string);
        fprintf(asmFile, "\tnew\t%d\n", currentClass->numFields);
        fprintf(asmFile, "\t.addr\t%s_%lx\n", currentClass->name->string, djb2(currentClass->fileName));
        fprintf(asmFile, "\tpopg\t%d\n", currentClass->globalIndex);
        currentClassList = currentClassList->tail;
    }
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
            generateCodeCallStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_BINOPEXP: /* 20 */
            break;
        case ABSYN_UNOPEXP: /* 21 */
            generateCodeUnopExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_INSTOFEXP: /* 22 */
            generateCodeInstofExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_CASTEXP: /* 23 */
            break;
        case ABSYN_NILEXP: /* 24 */
            break;
        case ABSYN_INTEXP: /* 25 */
            generateCodeIntExp(node, table, currentMethod, returnLabel, breakLabel);
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
            generateVarExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_CALLEXP: /* 32 */
            generateCodeCallExp(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_NEWEXP: /* 33 */
            generateCodeNewExp(node, table, currentMethod, returnLabel, breakLabel);
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
            generateCodeExpList(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMSTM: /* 44 */
            generateCodeAsmStmt(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMINSTRLIST: /* 45 */
            generateCodeAsmInstrList(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMINSTR0: /* 46 */
            generateCodeAsmInstr0(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMINSTR1: /* 47 */
            generateCodeAsmInstr1(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMINSTR2: /* 48 */
            generateCodeAsmInstr2(node, table, currentMethod, returnLabel, breakLabel);
            break;
        case ABSYN_ASMINSTR3: /* 49 */
            generateCodeAsmInstr3(node, table, currentMethod, returnLabel, breakLabel);
            break;
        default:
            /* this should never happen */
            error("unknown node type %d in generateCodeNode", node->type);
    }
}

void generateCode(Absyn *fileTrees[], int numInFiles, Table **fileTables, FILE *outFile) {
    int i;
    asmFile = outFile;

    metaClasses = emptyClassList();

    /* fileTables[0]->outerScope is the global table! */
    generateProlog(fileTables[0]->outerScope);

    for (i = 0; i < numInFiles; i++) {
        generateCodeNode(fileTrees[i], fileTables[i], NULL, -1, -1);
    }

    generateCodeMetaClasses();
}
