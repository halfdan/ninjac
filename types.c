/*
 * types.c -- type representation
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"


/**************************************************************/


Class *newClass(boolean isPublic, Sym *name,
                Class *superClass, struct table *mbrTable) {
  Class *class;

  class = (Class *) allocate(sizeof(Class));
  class->isPublic = isPublic;
  class->name = name;
  class->superClass = superClass;
  class->mbrTable = mbrTable;
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
  return type;
}


Type *newNilType(void) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_NIL;
  return type;
}


Type *newSimpleType(Class *class) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_SIMPLE;
  type->u.simpleType.class = class;
  return type;
}


Type *newArrayType(Class *base, int dims) {
  Type *type;

  type = (Type *) allocate(sizeof(Type));
  type->kind = TYPE_KIND_ARRAY;
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

  switch (type->kind) {
    case TYPE_KIND_VOID:
      printf("void");
      break;
    case TYPE_KIND_NIL:
      printf("nil");
      break;
    case TYPE_KIND_SIMPLE:
      printf("%s", symToString(type->u.simpleType.class->name));
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