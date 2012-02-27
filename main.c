/*
 * main.c -- Ninja Compiler
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "utils.h"
#include "sym.h"
#include "types.h"
#include "absyn.h"
#include "vmt.h"
#include "scanner.h"
#include "instance.h"
#include "table.h"
#include "parser.h"
#include "semant.h"
#include "codegen.h"

#define VERSION		7

char *mainClass = "Main";

static void version(char *myself) {
  /* show version and compilation date */
  printf("%s version %d (compiled %s, %s)\n",
         myself, VERSION, __DATE__, __TIME__);
}


static void help(char *myself) {
  /* show some help how to use the program */
  printf("Usage: %s [options] <input file> [...]\n", myself);
  printf("Options:\n");
  printf("  --output <file>     specify output file\n");
  printf("  --mainclass <class> specify main class\n");
  printf("  --njlib <dir>       specify location of library\n");
  printf("  --tokens            show stream of tokens (no parsing)\n");
  printf("  --absyn             show abstract syntax\n");
  printf("  --tables            show symbol tables\n");
  printf("  --version           show version and exit\n");
  printf("  --help              show this help and exit\n");
}


int main(int argc, char *argv[]) {
  int i;
  char *inFileName[MAX_INFILES];
  int numInFiles;
  char *outFileName;
  char *ninjaLibrary;
  boolean optionTokens;
  boolean optionAbsyn;
  boolean optionTables;
  int token;
  Absyn *fileTrees[MAX_INFILES];
  FILE *outFile;
  Table **fileTables;

  /* analyze command line */
  numInFiles = 0;
  outFileName = NULL;
  optionTokens = FALSE;
  optionAbsyn = FALSE;
  optionTables = FALSE;
  ninjaLibrary = NULL;
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      /* option */
      if (strcmp(argv[i], "--output") == 0) {
        if (++i == argc) {
          error("output file name missing");
        }
        outFileName = argv[i];
      } else
      if (strcmp(argv[i], "--mainclass") == 0) {
        i++;
        if (argv[i][0] == '-' || i == argc) {
          error("main class missing or invalid");
        }
        mainClass = argv[i];
      } else
      if (strcmp(argv[i], "--njlib") == 0) {
        i++;
        if (argv[i][0] == '-' || i == argc) {
          error("Ninja library missing or invalid");
        }
        ninjaLibrary = argv[i];
      } else
      if (strcmp(argv[i], "--tokens") == 0) {
        optionTokens = TRUE;
      } else
      if (strcmp(argv[i], "--absyn") == 0) {
        optionAbsyn = TRUE;
      } else
      if (strcmp(argv[i], "--tables") == 0) {
        optionTables = TRUE;
      } else
      if (strcmp(argv[i], "--version") == 0) {
        version(argv[0]);
        exit(0);
      } else
      if (strcmp(argv[i], "--help") == 0) {
        help(argv[0]);
        exit(0);
      } else {
        error("unrecognized option '%s'; try '%s --help'",
              argv[i], argv[0]);
      }
    } else {
      /* file */
      if (numInFiles == MAX_INFILES) {
        error("too many input files");
      }
      inFileName[numInFiles++] = argv[i];
    }
  }
  if (numInFiles == 0) {
    error("no input file");
  }

  /* scan & parse source */
  for (i = 0; i < numInFiles; i++) {
    printf("Compiling file '%s'...\n", inFileName[i]);
    if (!yyInit(inFileName[i])) {
      error("cannot open input file '%s'", inFileName[i]);
    }
    if (optionTokens) {
      do {
        token = yylex();
        showToken(token);
      } while (token != 0);
    } else {
      yyparse();
      /* Set file name in Absyn Tree */
      fileTree->file = inFileName[i];
      fileTrees[i] = fileTree;
    }
  }
  if (optionTokens) {
    exit(0);
  }
  if (optionAbsyn) {
    for (i = 0; i < numInFiles; i++) {
      printf("Abstract syntax for file \"%s\":\n", inFileName[i]);
      showAbsyn(fileTrees[i]);
    }
  }
  /* do semantic analysis */
  fileTables = check(fileTrees, numInFiles, optionTables);

  
  if (optionAbsyn) {
    for (i = 0; i < numInFiles; i++) {
      printf("Abstract syntax for file AFTER SEMANT \"%s\":\n", inFileName[i]);
      showAbsyn(fileTrees[i]);
    }
  }
  /* If there is a filename open it, else use STDOUT */
  if(NULL != outFileName)
      outFile = fopen(outFileName, "w");
  else
      outFile = (FILE*)stdout;
  /* generate code */
  generateCode(fileTrees, numInFiles, fileTables, outFile);

  if(NULL != outFileName)
      fclose(outFile);

  /* done */
  return 0;
}
