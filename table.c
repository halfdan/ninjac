/*
 * table.c -- symbol table
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "table.h"


/**************************************************************/


Entry *newClassEntry(Class *class) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_CLASS;
  entry->u.classEntry.class = class;
  return entry;
}


Entry *newMethodEntry(boolean isPublic, boolean isStatic,
                      Type *retType, TypeList *paramTypes,
                      Table *localTable) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_METHOD;
  entry->u.methodEntry.isPublic = isPublic;
  entry->u.methodEntry.isStatic = isStatic;
  entry->u.methodEntry.retType = retType;
  entry->u.methodEntry.paramTypes = paramTypes;
  entry->u.methodEntry.localTable = localTable;
  return entry;
}


Entry *newVariableEntry(boolean isLocal, boolean isPublic,
                        boolean isStatic, Type *type) {
  Entry *entry;

  entry = (Entry *) allocate(sizeof(Entry));
  entry->kind = ENTRY_KIND_VARIABLE;
  entry->u.variableEntry.isLocal = isLocal;
  entry->u.variableEntry.isPublic = isPublic;
  entry->u.variableEntry.isStatic = isStatic;
  entry->u.variableEntry.type = type;
  return entry;
}


static void indent(int n) {
  int i;

  for (i = 0; i < n; i++) {
    printf("  ");
  }
}


void showEntry(Entry *entry) {
  switch (entry->kind) {
    case ENTRY_KIND_CLASS:
      printf("class:\n");
      showClass(entry->u.classEntry.class, 10);
      break;
    case ENTRY_KIND_METHOD:
      printf("method:\n");
      indent(10);
      printf("isPublic = %s",
             entry->u.methodEntry.isPublic ? "yes" : "no");
      printf("\n");
      indent(10);
      printf("isStatic = %s",
             entry->u.methodEntry.isStatic ? "yes" : "no");
      printf("\n");
      indent(10);
      printf("retType = ");
      showType(entry->u.methodEntry.retType);
      printf("\n");
      indent(10);
      printf("paramTypes = ");
      showTypeList(entry->u.methodEntry.paramTypes);
      printf("\n");
      break;
    case ENTRY_KIND_VARIABLE:
      printf("variable:\n");
      indent(10);
      printf("isLocal = %s",
             entry->u.variableEntry.isLocal ? "yes" : "no");
      printf("\n");
      indent(10);
      printf("isPublic = %s",
             entry->u.variableEntry.isPublic ? "yes" : "no");
      printf("\n");
      indent(10);
      printf("isStatic = %s",
             entry->u.variableEntry.isStatic ? "yes" : "no");
      printf("\n");
      indent(10);
      printf("type = ");
      showType(entry->u.variableEntry.type);
      printf("\n");
      break;
    default:
      /* this should never happen */
      error("unknown entry kind %d in showEntry", entry->kind);
      break;
  }
}


/**************************************************************/


Table *newTable(Table *outerScope) {
  Table *table;

  table = (Table *) allocate(sizeof(Table));
  table->outerScope = outerScope;
  table->numEntries = 0;
  table->bintree = NULL;
  return table;
}


Entry *enter(Table *table, Sym *sym, Entry *entry) {
  unsigned key;
  Bintree *newtree;
  Bintree *current;
  Bintree *previous;

  key = symToStamp(sym);
  newtree = (Bintree *) allocate(sizeof(Bintree));
  newtree->sym = sym;
  newtree->key = key;
  newtree->entry = entry;
  newtree->left = NULL;
  newtree->right = NULL;
  if (table->bintree == NULL) {
    table->numEntries = 1;
    table->bintree = newtree;
  } else {
    current = table->bintree;
    while (1) {
      if (current->key == key && current->entry->kind == entry->kind) {
        /* symbol already in table */
        return NULL;
      }
      previous = current;
      if (current->key > key && current->entry->kind == entry->kind) {
        current = current->left;
      } else {
        current = current->right;
      }
      if (current == NULL) {
        table->numEntries++;
        if (previous->key > key) {
          previous->left = newtree;
        } else {
          previous->right = newtree;
        }
        break;
      }
    }
  }
  return entry;
}


static Entry *lookupBintree(Bintree *bintree, unsigned key, int kind) {
  while (bintree != NULL) {
    if (bintree->key == key && bintree->entry->kind == kind) {
      return bintree->entry;
    }
    if (bintree->key > key && bintree->entry->kind != kind) {
      bintree = bintree->left;
    } else {
      bintree = bintree->right;
    }
  }
  return NULL;
}


Entry *lookup(Table *table, Sym *sym, int kind) {
  unsigned key;
  Entry *entry;

  key = symToStamp(sym);
  while (table != NULL) {
    entry = lookupBintree(table->bintree, key, kind);
    if (entry != NULL) {
      return entry;
    }
    table = table->outerScope;
  }
  return NULL;
}

Entry *lookupMember(Class *class, Sym *sym, int kind) {
    unsigned key;
    Entry *entry;
    Table *table;
    Class *tmpClass;

    key = symToStamp(sym);
    tmpClass = class;

    while(tmpClass != NULL) {
        table = tmpClass->mbrTable;
        entry = lookupBintree(table->bintree, key, kind);
        if(entry != NULL) {
            return entry;
        }
        tmpClass = tmpClass->superClass;
    }
    return NULL;
}


static int collectEntries(Bintree *bintree, Bintree **entries, int start) {
  if (bintree == NULL) {
    return start;
  }
  start = collectEntries(bintree->left, entries, start);
  entries[start++] = bintree;
  start = collectEntries(bintree->right, entries, start);
  return start;
}


static int compareEntry(const void *x, const void *y) {
  Bintree *p;
  Bintree *q;

  p = * (Bintree **) x;
  q = * (Bintree **) y;
  return strcmp(symToString(p->sym),
                symToString(q->sym));
}


void showTable(Table *table) {
  Bintree **entries;
  int n, i;

  printf("  number of entries = %d\n", table->numEntries);
  if (table->bintree == NULL) {
    printf("  <empty>\n");
    return;
  }
  entries = (Bintree **) allocate(table->numEntries * sizeof(Bintree *));
  n = collectEntries(table->bintree, entries, 0);
  if (n != table->numEntries) {
    /* this should never happen */
    error("wrong number of entries in table");
  }
  qsort(entries, n, sizeof(Bintree *), compareEntry);
  for (i = 0; i < n; i++) {
    printf("  %-10s --> ", symToString(entries[i]->sym));
    showEntry(entries[i]->entry);
  }
  for (i = 0; i < n; i++) {
    switch (entries[i]->entry->kind) {
      case ENTRY_KIND_CLASS:
        printf("Member Table for Class '%s'\n",
               symToString(entries[i]->sym));
        showTable(entries[i]->entry->u.classEntry.class->mbrTable);
        break;
      case ENTRY_KIND_METHOD:
        /* !!!!! must be filled in */
        break;
      case ENTRY_KIND_VARIABLE:
        /* nothing to do here */
        break;
      default:
        /* this should never happen */
        error("illegal entry kind in showTable");
        break;
    }
  }
  release(entries);
}
