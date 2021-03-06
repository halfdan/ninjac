/*
 * types.h -- type representation
 */


#ifndef _TYPES_H_
#define _TYPES_H_


typedef struct class {
  boolean isPublic;		/* class visibility outside of package */
  Sym *name;			/* name of the class */
  struct class *superClass;	/* its superclass */
  struct class *metaClass;      /* its meta class */
  struct table *mbrTable;	/* symbol table for class members */
  struct vmt *vmt;                     /* virtual method table as linked list */
  struct instancevar *attibuteList;       /* instance variables as linked list */
  char *fileName;               /* File where the class is defined */
  int numFields;                /* number of instance variables */
  int numMethods;               /* number of object methods */
  int globalIndex;              /* index for meta classes */
} Class;

typedef struct classList {
  boolean isEmpty;            /* Last element */
  Class *head;
  struct classList *tail;
} ClassList;


Class *newClass(boolean isPublic, Sym *name, char *fileName,
                Class *superClass, Class *metaClass, struct table *mbrTable);

ClassList *newClassList(ClassList *classList, Class *class1);
ClassList *emptyClassList(void);

boolean isSameOrSubclassOf(Class *class1, Class *class2);
void showClass(Class *class, int pos);
void makeMetaClass(Class * class);


#define TYPE_KIND_VOID		0
#define TYPE_KIND_NIL		1
#define TYPE_KIND_SIMPLE	2
#define TYPE_KIND_ARRAY		3


typedef struct type {
  int kind;
  int isStatic;                 /* for static type evaluations */
  union {
    struct {
      int dummy;		/* empty struct not allowed in C */
    } voidType;
    struct {
      int dummy;		/* empty struct not allowed in C */
    } nilType;
    struct {
      Class *class;		/* the class */
    } simpleType;
    struct {
      Class *base;		/* the base class */
      int dims;			/* number of dimensions */
    } arrayType;
  } u;
} Type;


Type *newVoidType(void);
Type *newNilType(void);
Type *newSimpleType(Class *class);
Type *newStaticSimpleType(Class *class);
Type *newArrayType(Class *base, int dims);
boolean isSameOrSubtypeOf(Type *type1, Type *type2);
boolean isStaticTypeOf(Type *type1, Type *type2);
void showType(Type *type);


typedef struct typeList {
  boolean isEmpty;		/* true: last element, type/next unused */
  Type *type;			/* head of the list */
  struct typeList *next;	/* tail of the list */
} TypeList;


TypeList *emptyTypeList(void);
TypeList *newTypeList(Type *type, TypeList *next);
void showTypeList(TypeList *typeList);
boolean isParamTypeListLengthEqual(TypeList *parList1, TypeList *parList2);
int getLength(TypeList* List);


#endif /* _TYPES_H_ */
