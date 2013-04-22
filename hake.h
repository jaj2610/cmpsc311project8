/* CMPSC 311, Spring 2013, Project 8
 * 
 * Author: Jacob Jones
 * Email: jaj5333@psu.edu
 * 
 * Author: Scott Cheloha
 * Email: ssc5145@psu.edu
 */

/*----------------------------------------------------------------------------*/


#ifndef HAKE_H
#define HAKE_H

#include <stdio.h>

// maximum line length in an input file (buffer size in read_lines)
#define MAXLINE 4096

extern char *prog;
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;

extern int v_flag;	// verbosity specified
extern int d_flag;

// return 1 if successful, 0 if not
// "success" means the file could be opened for reading, or that we had seen
//    the file before and don't need to read it again
// quiet == 0 enables error messages if the file can't be opened
// quiet == 1 suppresses error messages if the file can't be opened
int read_file(char *filename, int quiet);

// fp comes from the file (named filename) opened by read_file() using fopen()
void read_lines(char *filename, FILE *fp);

int verify_tar_pre(const char * buf);

#endif /* HAKE_H */
