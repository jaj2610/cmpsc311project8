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

#ifndef CMPSC311_NAMES_H
#define CMPSC311_NAMES_H

//------------------------------------------------------------------------------

// singly-linked list of names

struct name
{
  struct name *next;	// NULL indicates end-of-list
  char *name;		// from strdup()
};

struct list_names
{
  struct name *head;	// NULL indicates empty list
  struct name *tail;};

//------------------------------------------------------------------------------

struct list_names *list_names_allocate(void);
void list_names_deallocate(struct list_names * const list);

void list_names_print(const struct list_names * const list);

void list_names_append(struct list_names * const list, const char *name);
 int list_names_append_if_new(struct list_names * const list, const char *name);
void list_names_append_from_file(struct list_names * const list, const char *filename);

void list_names_iterate(struct list_names *list, void (*func)(void *));

//------------------------------------------------------------------------------

/* Notes
 *
 * The assumption is that a struct name can appear on only one list, but the
 * list itself could have more than one reference to it.
 *
 * list_names_allocate() allocates a new list; list_names_reference() creates
 * another reference to an existing list.
 *
 * Do not apply list_names_deallocate() to something that did not come from
 * list_names_allocate() or list_names_reference().
 *
 * list_names_append_if_new() works as follows:
 *     if (name is on the list already) { return 1 }
 *     else { put name on the list and return 0 }
 *
 * If list_names_allocate() or one of the list_names_append() functions
 * can't allocate enough memory, the program ends with an error message.
 */

//------------------------------------------------------------------------------

/* Examples
 *
 * Declare, initialize and print a list
 *   struct list_names list;
 *   list_names_init(&list, "something");
 *   list_names_print(&list);
 *
 * Declare and initialize a list (incorrect)
 *   struct list_names list = { "something", NULL, NULL, 0 };
 *
 * Allocate a list, initialized
 *   struct list_names *list = list_names_allocate("something");
 *   if (list == NULL) { ... allocation failed ... }
 *   list_names_print(list);
 *
 * Deallocate a list
 *   struct list_names *list = list_names_allocate("something");
 *   list_names_deallocate(list);
 *   list = NULL;
 *
 * Deallocate a list (incorrect)
 *   struct list_names list;
 *   list_names_init(&list, "something");
 *   list_names_deallocate(list);
 *   list = NULL;
 *
 * Put a name on the end of the list (assume a declared list)
 *   list_names_append(&list1, "one");
 *
 * Put a name on the end of the list (assume an allocated list)
 *   list_names_append(list2, "two");
 */

//------------------------------------------------------------------------------

#endif