/*
 * types.c -- type representation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "vmt.h"
#include "types.h"
#include "instance.h"


/**************************************************************/


Class *newClass(boolean isPublic, Sym *name, char *fileName,
                Class *superClass, Class *metaClass, struct table *mbrTable) {
  Class *class;

  class = (Class *) allocate(sizeof(Class));
  class->isPublic = isPublic;
  class->name = name;
  class->fileName = fileName;
  class->superClass = superClass;
  class->metaClass = metaClass;
  class->mbrTable = mbrTable;
  class->vmt = NULL;
  class->attibuteList = NULL;
  return class;
}


boolean isSameOrSubclassOf(Class *class1, Class *class2) {
  while (class1 != NULL) {
    if (class1 == class2) {
      return TRUE;
    }
    class1 = class1->superClass;
  }
  return FALSE;
}


static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}


void showClass(Class *class, int pos) {
  indent(pos);
  printf("isPublic = %s", class->isPublic ? "yes" : "no");
  printf("\n");
  indent(pos);
  printf("name = %s", symToString(class->name));
  printf("\n");
  indent(pos);
  if (class->superClass == NULL) {
    printf("superClass = <none>");
  } else {
    printf("superClass = %s", symToString(class->superClass->name));
  }
  printf("\n");
}


/**************************************************************/


Type *newVoidType(void) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_VOID;
  type->isStatic = -1;
  return type;
}


Type *newNilType(void) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_NIL;
  type->isStatic = -1;
  return type;
}


Type *newSimpleType(Class *class) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_SIMPLE;
  type->isStatic = FALSE;
  type->u.simpleType.class = class;
  return type;
}


Type *newStaticSimpleType(Class *class) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_SIMPLE;
  type->isStatic = TRUE;
  type->u.simpleType.class = class;
  return type;
}


Type *newArrayType(Class *base, int dims) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_ARRAY;
  type->isStatic = FALSE;
  type->u.arrayType.base = base;
  type->u.arrayType.dims = dims;
  return type;
}


boolean isSameOrSubtypeOf(Type *type1, Type *type2) {
  switch (type1->kind) {
    case TYPE_KIND_VOID:
      return type2->kind == TYPE_KIND_VOID;
    case TYPE_KIND_NIL:
      return type2->kind != TYPE_KIND_VOID;
    case TYPE_KIND_SIMPLE:
      return (type2->kind == TYPE_KIND_SIMPLE) &&
             isSameOrSubclassOf(type1->u.simpleType.class,
                                type2->u.simpleType.class) &&
              type1->isStatic == type2->isStatic;
    case TYPE_KIND_ARRAY:
      return (type2->kind == TYPE_KIND_ARRAY) &&
             isSameOrSubclassOf(type1->u.arrayType.base,
                                type2->u.arrayType.base) &&
             (type1->u.arrayType.dims == type2->u.arrayType.dims);
    default:
      /* this should never happen */
      error("unknown type kind %d in isSameOrSubtypeOf", type1->kind);
      break;
  }
  return FALSE;
}


boolean isStaticTypeOf(Type *type1, Type *type2) {
  if (type1->isStatic == type2->isStatic)
    return FALSE;
    
  switch (type1->kind) {
    case TYPE_KIND_VOID:
      return type2->kind == TYPE_KIND_VOID;
    case TYPE_KIND_NIL:
      return type2->kind != TYPE_KIND_VOID;
    case TYPE_KIND_SIMPLE:
      return (type2->kind == TYPE_KIND_SIMPLE) &&
             isSameOrSubclassOf(type1->u.simpleType.class,
                                type2->u.simpleType.class);
    case TYPE_KIND_ARRAY:
      return (type2->kind == TYPE_KIND_ARRAY) &&
             isSameOrSubclassOf(type1->u.arrayType.base,
                                type2->u.arrayType.base) &&
             (type1->u.arrayType.dims == type2->u.arrayType.dims);
    default:
      /* this should never happen */
      error("unknown type kind %d in isSameOrSubtypeOf", type1->kind);
      break;
  }
  return FALSE;
}

void showType(Type *type) {
  int i;

  if (type == NULL) {
      printf("<constructor>");
      return;
  }

  switch (type->kind) {
    case TYPE_KIND_VOID:
      printf("void");
      break;
    case TYPE_KIND_NIL:
      printf("nil");
      break;
    case TYPE_KIND_SIMPLE:
      printf("%s", symToString(type->u.simpleType.class->name));
      if (type->isStatic) printf(" (static)");
      break;
    case TYPE_KIND_ARRAY:
      printf("%s", symToString(type->u.arrayType.base->name));
      for (i = 0; i < type->u.arrayType.dims; i++) {
        printf("[]");
      }
      break;
    default:
      /* this should never happen */
      error("unknown type kind %d in showType", type->kind);
      break;
  }
}


/**************************************************************/


TypeList *emptyTypeList(void) {
  TypeList *typeList;

  typeList = (TypeList *) allocate(sizeof(TypeList));
  typeList->isEmpty = TRUE;
  return typeList;
}


TypeList *newTypeList(Type *type, TypeList *next) {
  TypeList *typeList;

  typeList = (TypeList *) allocate(sizeof(TypeList));
  typeList->isEmpty = FALSE;
  typeList->type = type;
  typeList->next = next;
  return typeList;
}


void showTypeList(TypeList *typeList) {
  printf("(");
  while (!typeList->isEmpty) {
    showType(typeList->type);
    typeList = typeList->next;
    if (!typeList->isEmpty) {
      printf(", ");
    }
  }
  printf(")");
}

boolean isParamTypeListLengthEqual(TypeList *parList1, TypeList *parList2) {

    TypeList *tmpList1, *tmpList2;

    /* Loop over the two lists and compare their types*/
    for(
            tmpList1 = parList1,
            tmpList2 = parList2;
            !tmpList1->isEmpty && !tmpList2->isEmpty;
            tmpList1 = tmpList1->next,
            tmpList2 = tmpList2->next
        ) {
    }; /* don't actually do anything */

    /* If one of the elements is not empty we have
     * parameter lists of different length
     */
    if(!tmpList1->isEmpty || !tmpList2->isEmpty) {
        return FALSE;
    }
    /* ToDo: Could check which list is longer/shorter */

    return TRUE;
}

int getLength(TypeList* List) {
    TypeList *tmp;
    int i;
    
    for (i = 0,tmp = List;!tmp -> isEmpty;tmp = tmp->next,i++)
        ;

    return i;
}
