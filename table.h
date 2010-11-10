/* 
 * File:   table.h
 * Author: fate
 *
 * Created on 8. November 2010, 20:14
 */

#ifndef TABLE_H
#define	TABLE_H

#ifdef	__cplusplus
extern "C" {
#endif

    typedef struct table {
        int isEmpty;
        Sym *sym;
        struct entry *entry;
        struct table *next;
        struct table *outerScope;
    } Table;

    #define LIST_KIND_ARG               0
    #define LIST_KIND_FIELD             1
    #define LIST_KIND_MEMBER            2
    #define LIST_KIND_METHOD            3
    #define LIST_KIND_PARAM             4

    typedef struct list {
        int isEmpty;
        int kind;
        union {
            struct {

            } argList;
            struct {

            } fieldList;
            struct {

            } memberList;
            struct {

            } methodList;
            struct {

            } paramList;
        } u;
    } List;

    #define ENTRY_KIND_CLASS            0
    #define ENTRY_KIND_FIELD            1
    #define ENTRY_KIND_LOCAL            2
    #define ENTRY_KIND_METHOD           3
    #define ENTRY_KIND_PACKAGE          4

    typedef struct entry {
        int kind;
        union {
            struct {
                char *name;
                Sym *sym;
                int isPublic;
                int nFields;
                int nMethods;
                List *fields;
                List *methods;
                List *members;
                struct entry *superClass;
                struct entry *package;
                struct type *type;
                struct table *symTab;
            } classEntry;
            struct {
                char *name;
                Sym *sym;
                int isPublic;
                int isStatic;
                struct type *type;
            } fieldEntry;
            struct {

            } localEntry;
            struct {

            } methodEntry;
            struct {

            } packageEntry;
        } u;
    } Entry;

    Table *newTable(Sym *sym, Entry *entry, Table *next, Table *outerScope);
    Table *newEmptyTable(void);

    Entry *newClassEntry(char *name, Sym *sym, int isPublic, int nFields, int nMethods, Type *type, 
             List *fields, List *methods, List *members,
             struct entry *superClass, struct entry *package, struct type *type, struct table *symTab
     );
/*    Entry *newFieldEntry(Type *type);
    Entry *newMethodEntry(Type *type);
    Entry *newLocalEntry(Type *type);*/
#ifdef	__cplusplus
}
#endif

#endif	/* TABLE_H */

