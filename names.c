/* CMPSC 311, Spring 2013, Project 5
 *
 * Author:   Jake Jones
 * Email:    jaj5333@psu.edu
 *
... <additional comment text>
 */

/* base library for CMPSC 311 projects
 * version of 25 Feb. 2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "names.h"
#include "hake.h"
#include "wrapper.h"

//------------------------------------------------------------------------------

void list_names_init(struct list_names * const list, const char *listname)
{
  list->head = list->tail = NULL;
  list->reference_count = 0;
  list->name = Strdup(listname);
}

//------------------------------------------------------------------------------

struct list_names *list_names_allocate(const char *listname)
{
  struct list_names *list = Malloc(sizeof(struct list_names));

  list->head = list->tail = NULL;
  list->reference_count = 0;
  list->name = Strdup(listname);

  return list;
}

//------------------------------------------------------------------------------

struct list_names *list_names_reference(struct list_names * const list)
{
  list->reference_count++;

  return list;
}

//------------------------------------------------------------------------------

void list_names_deallocate(struct list_names * const list)
{
  if (--list->reference_count > 0)
    { return; }

  struct name *prev = NULL;
  for (struct name *p = list->head; p != NULL; p = p->next)
    {
      free(prev);	// free(NULL) is harmless
      free(p->name);
      prev = p;
    }
  free(prev);		// now, prev == list->tail

  free(list->name);
  free(list);
}

//------------------------------------------------------------------------------

void list_names_print(const struct list_names * const list)
{
  printf("list of names: %s\n", safe_string(list->name));

  if (list->head == NULL)
    { printf("  <empty>\n"); }
  else
    {
      for (struct name *p = list->head; p != NULL; p = p->next)
	{ printf("  %s\n", p->name); }
    }
}

//------------------------------------------------------------------------------

void list_names_append(struct list_names * const list, const char *name)
{
  struct name *p = Malloc(sizeof(struct name));

  p->next = NULL;
  p->name = Strdup(name);

  if (list->head == NULL)	// empty list, list->tail is also NULL
    {
      list->head = list->tail = p;
    }
  else
    {
      list->tail->next = p;
      list->tail = p;
    }
}

//------------------------------------------------------------------------------

// if (name is on the list already) { return 1 }
// else { put name on the list and return 0 }

int list_names_append_if_new(struct list_names * const list, const char *name)
{
  for (struct name *p = list->head; p != NULL; p = p->next)
    {
      if (strcmp(p->name, name) == 0)
	{ return 1; }   // name is on the list already
    }

  list_names_append(list, name);

  return 0;
}

//------------------------------------------------------------------------------

void list_names_append_from_file(struct list_names * const list, const char *filename)
{
  FILE *infile = NULL;

  if (strcmp(filename, "-") == 0)
    { infile = stdin; }
  else
    {
      infile = fopen(filename, "r");
      if (infile == NULL)
	{
	  fprintf(stderr, "%s: failed: could not open file %s: %s\n",
	    prog, filename, strerror(errno));
	  exit(EXIT_FAILURE);
	}
    }

  #define MAXLINE 256
  char buffer[MAXLINE+2];		// extra space for newline
  buffer[MAXLINE+2-2] = '\n';		// force a newline
  buffer[MAXLINE+2-1] = '\0';		// to end the string

  char whsp[] = " \t\n\v\f\r";		// whitespace characters
  char comm[] = "#\n";			// comment, newline

  while (fgets(buffer, MAXLINE, infile) != NULL)
    {
      /* Note that fgets() places a newline in buffer, if the input line
       *   was short enough to fit.  Line-too-long is probably an error,
       *   to be caught later.  We work around the rare case by forcing a
       *   newline and not overwriting it.
       * NULL from fgets() indicates end-of-file or error, so in that case
       *   we just quit.
       */

      // remove comment, if present
      // remove trailing newline
      int m = strcspn(buffer, comm);	// index of # or newline
      buffer[m] = '\0';			// remove the tail

      m = strspn(buffer, whsp);		// index of first non-whitespace character
      char *buf = &buffer[m];

      // remove trailing whitespace, by working backward from the end of string
      char *p = strchr(buf, '\0');	// *p is '\0', or p is NULL
      if (p == NULL)
	{
	  fprintf(stderr, "%s: strange string\n", prog);
	  exit(EXIT_FAILURE);
	}
      else
	{
	  p--;				// *p is '\0', so back up one position
	  while (p > buf && isspace(*p))
	    { *p = '\0'; p--; }
	}

      if (*buf == '\0')			// empty line
	{ continue; }

      list_names_append(list, buf);

      // get ready for the next iteration
      buffer[MAXLINE+2-2] = '\n';	// force a newline
      buffer[MAXLINE+2-1] = '\0';	// to end the string
    }

  if (infile != stdin && fclose(infile) != 0)
    {
      fprintf(stderr, "%s: failed: could not close input file %s: %s\n",
	prog, filename, strerror(errno));
      exit(EXIT_FAILURE);
    }
}

//------------------------------------------------------------------------------

// unfinished -- seemed like a good idea, but we didn't actually need it

void list_names_iterate(struct list_names *list, void (*func)(void *))
{
  for (struct name *p = list->head; p != NULL; p = p->next)
    {
      printf("calling func on %s\n", p->name);
    }
}

//------------------------------------------------------------------------------
