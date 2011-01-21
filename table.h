/*
 * table.h -- symbol table
 */


#ifndef _TABLE_H_
#define _TABLE_H_


#define ENTRY_KIND_CLASS	0
#define ENTRY_KIND_METHOD	1
#define ENTRY_KIND_VARIABLE	2


typedef struct {
  int kind;
  union {
    struct {
      Class *class;		/* the class record */
    } classEntry;
    struct {
      boolean isPublic;		/* method visibility outside of class */
      boolean isStatic;		/* true iff this is a class method */
      Type *retType;		/* return type */
      TypeList *paramTypes;	/* parameter types */
      struct table *localTable;	/* symbol table for local variables */
    } methodEntry;
    struct {
      boolean isLocal;		/* true iff this is a local variable */
      boolean isPublic;		/* variable visibility outside of class */
				/* always false for local variables */
      boolean isStatic;		/* true iff this is a class variable */
				/* always false for local variables */
      Type *type;		/* the type of the variable */
    } variableEntry;
  } u;
} Entry;


Entry *newClassEntry(Class *class);
Entry *newMethodEntry(boolean isPublic, boolean isStatic,
                      Type *retType, TypeList *paramTypes,
                      struct table *localTable);
Entry *newVariableEntry(boolean isLocal, boolean isPublic,
                        boolean isStatic, Type *type);
void showEntry(Entry *entry);


typedef struct bintree {
  Sym *sym;
  unsigned key;
  Entry *entry;
  struct bintree *left;
  struct bintree *right;
} Bintree;


typedef struct table {
  struct table *outerScope;
  int numEntries;
  Bintree *bintree;
} Table;


Table *newTable(Table *outerScope);
Entry *enter(Table *table, Sym *sym, Entry *entry);
Entry *lookup(Table *table, Sym *sym, int kind);
Entry *lookupClass(Table **fileTable, Table *globalTable, Sym *sym);
Entry *lookupMember(Class *class, Sym *sym, int kind);
void showTable(Table *table);


#endif /* _TABLE_H_ */
