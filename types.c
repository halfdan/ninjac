#inculde "types.h"
#include <string.h>


#include "types.h"

Type *newArrayType(Type *type, int number) {
    Type *pt;
    pt = (Type *) allocate(sizeof(Type));
    pt -> kind = TYPE_KIND_ARRAY;
    pt -> u . arrayType . number = number;
    pt -> u . arrayType . type = type;
    return pt;
}

Type *newClassType(char *name, int number) {
    Type *pt;
    pt = (Type *) allocate(sizeof(Type));
    pt -> kind = TYPE_KIND_CLASS;
    pt -> u . classType . member = number;
    strcpy(pt -> u . classType . name, name);
    return pt;
}

Type *newVoidType(void) {
    Type *pt;
    pt = (Type *) allocate(sizeof(Type));
    pt -> kind = TYPE_KIND_VOID;
    pt -> u . voidType . dummy = 0;
    return pt;
}

void showType(FILE *stream, Type *type) {
  switch (type->kind) {
    case TYPE_KIND_ARRAY:
      fprintf(stream, "ARRAY[%i] of ", type->u.arrayType.number);
      showType(stream, type->u.arrayType.type);
      break;
    case TYPE_KIND_CLASS:
      fprintf(stream, "%s", type->u.classType.name);
      break;
    case TYPE_KIND_VOID:
      fprintf(stream, "%s", "VOID");
      break;
    default:
      error("unknown type kind %d in showType", type->kind);
      break;
  }
}

