/* CMPSC 311, Spring 2013, Project 8
 * 
 * Author: Jacob Jones
 * Email: jaj5333@psu.edu
 * 
 * Author: Scott Cheloha
 * Email: ssc5145@psu.edu
 */

/* Originally from:
 * CMPSC 311, Spring 2013, Project 5 solution
 */

#ifndef CMPSC311_MACRO_H
#define CMPSC311_MACRO_H

void  macro_list_print(void);
char *macro_body(char *name);
static struct macro *get_macro(char *name);
void  macro_set(char *name, char *body);

// assume in[] is constructed properly
// assume out[] is large enough
void  macro_expand(char *in, char *out);

// assume in[] is constructed properly
// returns length of the expansion
int   macro_expand_length(char *in);

#endif

