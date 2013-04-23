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
#include <sys/types.h>
#include <time.h>
#include "linked.h"

// maximum line length in an input file (buffer size in read_lines)
#define MAXLINE 4096

extern char *prog;

/* A list of filenames recognized by hake.
 * Includes the initial hakefile (or whatever the user
 * specifies with -f) and any included files.
 */
extern struct string_list *filenames;

/* A global list of targets parsed by hake. */
extern struct target_list *parsed_targets;

extern struct string_list *recipes_to_print;

extern struct target_list *recursively_protected_targets;

extern int v_flag;	// verbosity specified
extern int d_flag;	// debug output specified

// return 1 if successful, 0 if not
// "success" means the file could be opened for reading, or that we had seen
//    the file before and don't need to read it again
// quiet == 0 enables error messages if the file can't be opened
// quiet == 1 suppresses error messages if the file can't be opened
int read_file(char *filename, int quiet);

// fp comes from the file (named filename) opened by read_file() using fopen()
// Returns 
int read_lines(char *filename, FILE *fp, time_t access_time);

/* Parses a "target : prerequisite" line into a struct target.
 * Returns a pointer to the new target on success.
 * 	-- Also adds the target to parsed_targets global target_list
 * 		(see above)
 * Returns NULL on failure.
 */
struct target *parse_target(char *buf, char *p_colon,
		char *filename, time_t file_access_time, int line_number);

/* Parses the prerequisites from the a "target : prerequisite" line
 * into a list of prereqs.
 * Returns a pointer to the string_list of parsed prereqs.
 */
struct string_list *parse_prereqs(char *prereqs, struct target *target,
		char *filename, int line_number);

/* Parses a macro line and adds the new macro to the macrolist in macro.h
 * Returns 0 on success 
 * Returns 1 on failure.*/
int parse_macro(char *buf, char *p_equal,
		const char *filename, int line_number);

/* Parses an include line and calls read_file() on the included file. */
void parse_include(char *buf, const char *filename, int line_number);

/* Removes leading/trailing whitespace on lines in the hakefile.
 * Also overwrites comments ('#') with '\0', effectively
 * removing them from the view of the interpreter.
 */
void clean_up_whitespace(char *buf);

#endif /* HAKE_H */
