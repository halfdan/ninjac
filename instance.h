/* 
 * File:   instance.h
 * Author: fate
 *
 * Created on 23. Januar 2011, 19:10
 */

#ifndef INSTANCE_H
#define	INSTANCE_H

typedef struct instancevar {
    boolean isEmpty;        /* determines wether the end of vmt is reached or not */
    struct instancevar *next;       /* pointer to next VMT */
    struct sym *name;       /* pointer to method name symbol */
    struct sym *className;
    struct sym *fileName;
    int offset;             /* offset of the virtual method */
} InstanceVar;

InstanceVar *newInstanceVar(Sym* name, InstanceVar *next,
        Sym *className, Sym *fileName, int offset);
InstanceVar *newEmptyInstanceVar(void);
InstanceVar *copyInstanceVar(InstanceVar *src);

int findInstanceVar(InstanceVar* src, Sym *name);
void replaceInstanceVar(InstanceVar* src, Sym *name, Sym *className, Sym *fileName, int offset);
void appendInstanceVar(InstanceVar* src, Sym *name, Sym *className, Sym *fileName);

void showInstanceVar(InstanceVar* src, int indent);

void makeInstanceVariableOffsets(Class *class, char *fileName);

int countFields(InstanceVar *attList);

#endif	/* INSTANCE_H */
