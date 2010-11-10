/*
 * scanner.h -- scanner interface
 */


#ifndef _SCANNER_H_
#define _SCANNER_H_


typedef struct {
  char *file;
  int line;
} NoVal;

typedef struct {
  char *file;
  int line;
  int val;
} IntVal;

typedef struct {
  char *file;
  int line;
  char *val;
} StringVal;


boolean yyInit(char *srcFileName);
int yylex(void);
void showToken(int token);


#endif /* _SCANNER_H_ */
