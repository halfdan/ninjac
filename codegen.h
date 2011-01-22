/*
 * codegen.h -- Code generation
 */

#ifndef CODEGEN_H
#define	CODEGEN_H

void generateCode(Absyn *fileTrees[], int numInFiles, Table **fileTables, FILE *outFile);

#endif	/* CODEGEN_H */

