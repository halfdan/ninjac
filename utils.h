/*
 * utils.h -- utility functions
 */


#ifndef _UTILS_H_
#define _UTILS_H_


void error(char *fmt, ...);
void *allocate(unsigned size);
void release(void *p);
unsigned long djb2(char* str);

#endif /* _UTILS_H_ */
