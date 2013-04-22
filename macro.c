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

//--------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmpsc311.h"
#include "macro.h"

//--------------------------------------------------------------------------------

// list of known macros, initially empty

struct macro {
  struct macro *next;
  char *name;
  char *body;
  int body_length;
};

static struct macro macro_list = {
  .next = NULL,
  .name = NULL,
  .body = NULL,
  .body_length = 0
};

//--------------------------------------------------------------------------------

void macro_list_print(void)
{
  printf("Macro list\n");

  for (struct macro *p = macro_list.next; p != NULL; p = p->next)
    {
      printf("  %s = %s (%d)\n", p->name, p->body, p->body_length);
    }

  printf("  -- end of list\n");
}

//--------------------------------------------------------------------------------

static struct macro *get_macro(char *name)
{
  for (struct macro *p = macro_list.next; p != NULL; p = p->next)
    {
      if (strcmp(p->name, name) == 0)
        { return p; }
    }

  return NULL;
}

//--------------------------------------------------------------------------------

char *macro_body(char *name)
{
  struct macro *p = get_macro(name);

  return (p == NULL) ? "" : p->body;
}

//--------------------------------------------------------------------------------

int macro_length(char *name)
{
  struct macro *p = get_macro(name);

  return (p == NULL) ? 0 : p->body_length;
}

//--------------------------------------------------------------------------------

void macro_set(char *name, char *body)
{
  struct macro *p = get_macro(name);

  if (p == NULL)
  {
    p = Malloc(sizeof(struct macro) __func__, __LINE__);
    p->next = macro_list.next;
    macro_list.next = p;
    p->name = Strdup(name, __func__, __LINE__);
    p->body = Strdup(body, __func__, __LINE__);
    p->body_length = strlen(body);
  }
  else
  {
    Free(p->body);
    p->body = Strdup(body, __func__, __LINE__);
    p->body_length = strlen(body);
  }
}

//--------------------------------------------------------------------------------

// assume in[] is constructed properly
// assume out[] is large enough
void macro_expand(char *in, char *out)
{
  while (*in != '\0')
  {
    if (*in == '$')	// macro, to be expanded
    {
      char *name = in+2;	// skip past ${
              // note - we don't actually check for {
      char *end = name;
      while (*end != '}') 
      {
        end++;
      }
      *end = '\0';	// temporary
      char *body = macro_body(name);
      *end = '}';	// restore original }
      strcpy(out, body);
      out += strlen(body);
      in = end+1;	// skip past }
    }
    else
    {
	    *out++ = *in++;
	  }
  }

  *out = '\0';
}

//--------------------------------------------------------------------------------

// assume in[] is constructed properly
// returns length of the expansion
int macro_expand_length(char *in)
{
  int n = 0;

  while (*in != '\0')
  {
    if (*in == '$')	// macro, to be expanded
    {
  	  char *name = in+2;	// skip past ${
              // note - we don't actually check for {
  	  char *end = name;
  	  while (*end != '}')
      {
        end++;
      }
  	  *end = '\0';	// temporary
  	  int body_length = macro_length(name);
  	  *end = '}';	// restore original }
  	  n += body_length;
  	  in = end+1;	// skip past }
	  }
    else
    {
  	  in++;
  	  n++;
	  }
  }

  return n;
}

//--------------------------------------------------------------------------------

