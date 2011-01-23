/*
 * utils.c -- utility functions
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "utils.h"

void error(char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

void *allocate(unsigned size) {
    void *p;

    p = malloc(size);
    if (p == NULL) {
        error("out of memory");
    }
    return p;
}

void release(void *p) {
    if (p == NULL) {
        error("NULL pointer detected in release");
    }
    free(p);
}

unsigned long djb2(char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}
