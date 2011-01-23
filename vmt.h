/* 
 * File:   vmt.h
 * Author: fate
 *
 * Created on 23. Januar 2011, 01:19
 */

#ifndef VMT_H
#define	VMT_H

typedef struct vmt {
    boolean isEmpty;        /* determines wether the end of vmt is reached or not */
    struct vmt *next;       /* pointer to next VMT */
    struct sym *name;       /* pointer to method name symbol */
    char *className;
    char *fileName;
    int offset;             /* offset of the virtual method */
} VMT;

VMT *newVMT(Sym* name, VMT *next, char *className, char *fileName, int offset);
VMT *newEmptyVMT(void);
VMT *copyVMT(VMT *src);

int findVMT(VMT* src, Sym *name);
void replaceVMT(VMT* src, Sym *name, char *className, char *fileName, int offset);
void appendVMT(VMT* src, Sym *name, char *className, char *fileName);

void showVMT(VMT* src, int indent);

#endif	/* VMT_H */

