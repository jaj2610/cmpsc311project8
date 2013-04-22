/* CMPSC 311, Spring 2013, Project 5 solution
 *
 */

/* base library for CMPSC 311 projects
 * version of 25 Feb. 2013
 */

#ifndef CMPSC311_H
#define CMPSC311_H

#include <stdio.h>
#include <stdbool.h>

//------------------------------------------------------------------------------

// global variables set from the command line

extern char *prog;	// program name
extern int verbose;	// -v option, extra output
			// -v can be repeated for even more output

  // verbosity levels
  // 0  off, default
  // 1  normal, behavioral info
  // 2  extra, functionality info
  // 3  memory allocation info, from Malloc(), Strdup()
  // 4  too much, information overload

//------------------------------------------------------------------------------

// utility functions
//   safe_string(() -- guard against null pointer for character string output

const char *safe_string(const char *str);

//------------------------------------------------------------------------------

// check function arguments
//   verify() -- tf is expected to be true; if not, print msg and quit

#define verify(tf, msg) cmpsc311_verify(tf, msg, __func__, __LINE__)

void cmpsc311_verify(const bool tf,
	const char *msg, const char *func, const int line);

//------------------------------------------------------------------------------

// check function return values
//   function	standards
//   malloc	C and Posix
//   strdup	      Posix
//   fopen	C and Posix
// We follow the standard protoypes of the original functions.
// Compare these to the error-checking wrappers in CS:APP, csapp.h and csapp.c.

#define Malloc(size)         cmpsc311_malloc(size, __func__, __LINE__)
#define Strdup(s)            cmpsc311_strdup(s, __func__, __LINE__)
#define Fopen(filename,mode) cmpsc311_fopen(filename, mode, __func__, __LINE__)

void *cmpsc311_malloc(size_t size,
	const char *func, const int line);
char *cmpsc311_strdup(const char *s,
	const char *func, const int line);
FILE *cmpsc311_fopen(const char * restrict filename, const char * restrict mode,
	const char *func, const int line);

//------------------------------------------------------------------------------

#endif

