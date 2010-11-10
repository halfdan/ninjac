#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "table.h"

    Table *newTable(Sym *sym, Entry *entry, Table *next, Table *outerScope) {
        Table *pt = (Table *) allocate(sizeof(Table));
        pt -> isEmpty = FALSE;
        pt -> sym = sym;
        pt -> entry = entry;
        pt -> next = next;
        pt -> outerScope = outerScope;
        return pt;
    }
    
    Table *newEmptyTable(void) {
        Table *pt = newTable(NULL, NULL, NULL, NULL);
        pt -> isEmpty = TRUE;
        return pt;
    }


    Entry *newClassEntry(char *name, Sym *sym, int isPublic, int nFields, int nMethods, Type *type,
             List *fields, List *methods, List *members,
             struct entry *superClass, struct entry *package, struct type *type, struct table *symTab) {
        Entry *pe = (Entry *) allocate(sizeof(Entry));
        pe -> kind = ENTRY_KIND_CLASS;
        pe -> u.classEntry.name = name;
        pe -> u.classEntry.sym = sym;
        pe -> u.classEntry.isPublic = TRUE;
        pe -> u.classEntry.nFields = nFields;
        pe -> u.classEntry.nMethods = nMethods;
        pe -> u.classEntry.type = type;
        pe -> u.classEntry.fields =  fields;
        pe -> u.classEntry.methods = methods;
        pe -> u.classEntry.members = members;
        pe -> u.classEntry.superClass = superClass;
        pe -> u.classEntry.package = package;
        pe -> u.classEntry.symTab = symTab;

        return pe;
    }
