/* 
 * File:   types.h
 * Author: fate
 *
 * Created on 8. November 2010, 22:27
 */

#ifndef TYPES_H
#define	TYPES_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define TYPE_KIND_ARRAY         0
    #define TYPE_KIND_CLASS         1
    #define TYPE_KIND_VOID          2

    typedef struct type {
        int kind;
        union {
            struct {
                int dummy;
            } voidType;
            struct {
                char *name;
                int nMember;
                int nMethods;
                int nFields;
                struct type *superType;
            } classType;
            struct {
                struct type *type;
                int number;
            } arrayType;
        } u;
    } Type;


    Type *newArrayType(Type *type, int number);
    Type *newClassType(char *name);
    Type *newVoidType(void);

    void showType(FILE *stream, Type *type);

#ifdef	__cplusplus
}
#endif

#endif	/* TYPES_H */

