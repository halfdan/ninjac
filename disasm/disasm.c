#include <stdio.h>
#include <stdlib.h>

#define LINE_SIZE	200

#define HALT		0
#define PUSHC		1
#define ADD		2
#define SUB		3
#define MUL		4
#define DIV		5
#define MOD		6
#define RDINT		7
#define WRINT		8
#define ASF		9
#define RSF		10
#define PUSHL		11
#define POPL		12
#define EQ		13
#define NE		14
#define LT		15
#define LE		16
#define GT		17
#define GE		18
#define JMP		19
#define BRF		20
#define BRT		21
#define CALL		22
#define RET		23
#define DROP		24
#define PUSHR		25
#define POPR		26
#define DUP		27
#define NEW		28
#define GETF		29
#define PUTF		30
#define NEWA		31
#define GETLA		32
#define GETFA		33
#define PUTFA		34
#define PUSHN		35
#define REFEQ		36
#define REFNE		37
#define VMCALL		38
#define PUSHG           39
#define POPG            40

#define IMMEDIATE(x)	((x) & 0x00FFFFFF)
#define SIGN_EXTEND(i)	((i) & 0x00800000 ? (i) | 0xFF000000 : (i))

unsigned int *code;		/* program memory */
int codeSize;			/* size in instructions */

void loadCode(char *codeFileName) {
  FILE *codeFile;
  long size;

  codeFile = fopen(codeFileName, "r");
  if (codeFile == NULL) {
    fprintf(stderr, "cannot open code file '%s'", codeFileName);
  }
  fseek(codeFile, 0, SEEK_END);
  size = ftell(codeFile);
  fseek(codeFile, 0, SEEK_SET);
  code = malloc(size);
  if (code == NULL) {
    fclose(codeFile);
    fprintf(stderr,"cannot allocate program memory");
  }
  codeSize = size / sizeof(unsigned int);
  if (fread(code, sizeof(unsigned int), codeSize, codeFile) != codeSize) {
    fclose(codeFile);
    fprintf(stderr,"cannot read code file '%s'", codeFileName);
  }
  fclose(codeFile);
}

char *disasm(unsigned int *code, int addr) {
  static char buffer[LINE_SIZE];
  unsigned int instr;
  int opcode;
  unsigned int uimmed;
  signed int simmed;
  int numArgs, vmtIndex;

  instr = code[addr];
  opcode = (instr >> 24) & 0x00FF;
  uimmed = instr & 0x00FFFFFF;
  simmed = SIGN_EXTEND(uimmed);
  switch (opcode) {
    case HALT:
      if (uimmed == 0) {
        sprintf(buffer, "halt");
      } else {
        sprintf(buffer, "???");
      }
      break;
    case PUSHC:
      sprintf(buffer, "pushc   %d", simmed);
      break;
    case ADD:
      sprintf(buffer, "add");
      break;
    case SUB:
      sprintf(buffer, "sub");
      break;
    case MUL:
      sprintf(buffer, "mul");
      break;
    case DIV:
      sprintf(buffer, "div");
      break;
    case MOD:
      sprintf(buffer, "mod");
      break;
    case RDINT:
      sprintf(buffer, "rdint");
      break;
    case WRINT:
      sprintf(buffer, "wrint");
      break;
    case ASF:
      sprintf(buffer, "asf     %d", simmed);
      break;
    case RSF:
      sprintf(buffer, "rsf");
      break;
    case PUSHL:
      sprintf(buffer, "pushl   %d", simmed);
      break;
    case POPL:
      sprintf(buffer, "popl    %d", simmed);
      break;
    case EQ:
      sprintf(buffer, "eq");
      break;
    case NE:
      sprintf(buffer, "ne");
      break;
    case LT:
      sprintf(buffer, "lt");
      break;
    case LE:
      sprintf(buffer, "le");
      break;
    case GT:
      sprintf(buffer, "gt");
      break;
    case GE:
      sprintf(buffer, "ge");
      break;
    case JMP:
      sprintf(buffer, "jmp     0x%08X", uimmed);
      break;
    case BRF:
      sprintf(buffer, "brf     0x%08X", uimmed);
      break;
    case BRT:
      sprintf(buffer, "brt     0x%08X", uimmed);
      break;
    case CALL:
      sprintf(buffer, "call    0x%08X", uimmed);
      break;
    case RET:
      sprintf(buffer, "ret");
      break;
    case DROP:
      sprintf(buffer, "drop    %d", simmed);
      break;
    case PUSHR:
      sprintf(buffer, "pushr");
      break;
    case POPR:
      sprintf(buffer, "popr");
      break;
    case DUP:
      sprintf(buffer, "dup");
      break;
    case NEW:
      sprintf(buffer, "new     %d", simmed);
      break;
    case GETF:
      sprintf(buffer, "getf    %d", simmed);
      break;
    case PUTF:
      sprintf(buffer, "putf    %d", simmed);
      break;
    case NEWA:
      sprintf(buffer, "newa");
      break;
    case GETLA:
      sprintf(buffer, "getla");
      break;
    case GETFA:
      sprintf(buffer, "getfa");
      break;
    case PUTFA:
      sprintf(buffer, "putfa");
      break;
    case PUSHN:
      sprintf(buffer, "pushn");
      break;
    case REFEQ:
      sprintf(buffer, "refeq");
      break;
    case REFNE:
      sprintf(buffer, "refne");
      break;
    case VMCALL:
      numArgs = (uimmed >> 16) & 0xFF;
      vmtIndex = uimmed & 0xFFFF;
      sprintf(buffer, "vmcall  %d,%d", numArgs, vmtIndex);
      break;
    case PUSHG:
      sprintf(buffer, "pushg");
      break;
    case POPG:
      sprintf(buffer, "popg");
      break;
    default:
      printf("illegal instruction 0x%08X at address 0x%08X",
            instr, addr);
  }
  return buffer;
}

void viewInstructions(unsigned int *code, int codeSize,
                      int start, int count) {
  int ip;
  unsigned int instr;

  for (ip = start; ip < start + count; ip++) {
    if (ip >= codeSize) {
      printf("        --- end of code ---\n");
      break;
    }
    instr = code[ip];
    printf("0x%08X:  0x%08X    %s\n", ip, instr, disasm(code, ip));
  }
}

int main(int argc, char* argv[]) {
    if(argc <= 1) {
        fprintf(stderr, "Error: no binary file supplied!\n");
    }
    else {
        loadCode(argv[1]);
        viewInstructions(code, codeSize, 0, codeSize -1);
    }
    return 0;
}