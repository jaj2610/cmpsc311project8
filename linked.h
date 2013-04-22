/* CMPSC 311, Spring 2013, Project 8
 * 
 * Author: Jacob Jones
 * Email: jaj5333@psu.edu
 * 
 * Author: Scott Cheloha
 * Email: ssc5145@psu.edu
 */

#ifndef LINKED_H
#define LINKED_H

//------------------------------------------------------------------------------

// used for prereqs and recipes
struct string_node
{
	struct string_node *next;
	char *body;
};

// used for lists of prereqs and recipes
struct string_list
{
	struct string_node *head;
	struct string_node *tail;
};

//------------------------------------------------------------------------------

/* string_list_allocate() allocates a new string_list */
struct string_list *string_list_allocate(void);

/* string_list_deallocate() deallocates a string_list completely */
void string_list_deallocate(struct string_list * const list);

/* string_list_print() prints entire string_list */
void string_list_print(const struct string_list * const list);

/* string_list_append() adds a node to a string_list */
void string_list_append(struct string_list * const list, const char *body);

/* string_list_allocate() removes a node from a string_list */
void string_list_pop(struct string_list * const list, const char *body);

/* get_string() returns a string with the body body */
struct target *get_string(struct string_list * const list, const char *body);

/* if (body is on the list already) { return 1 }
 * else { put body on the list and return 0 } */

int string_list_append_if_new(struct list_bodys * const list, const char *body);

//------------------------------------------------------------------------------

// used for targets
struct target
{
	struct target *next;
	char *name;
	struct string_list *prereqs;
	struct string_list *recipes;
	int needs_to_be_haked;	// default 0; indicates whether or not target's recipes need to be printed
};

// used for lists of targets
struct target_list
{
	struct target *head;
	struct target *tail
};

//------------------------------------------------------------------------------

/* target_list_allocate() allocates a new target_list */
struct target_list *target_list_allocate(void);

/* target_list_deallocate() deallocates a target_list completely */
void target_list_deallocate(struct target_list * const list);

/* target_list_print() prints entire target_list */
void target_list_print(const struct target_list * const list);

/* target_list_append() adds a node to a target_list */
void target_list_append(struct target_list * const list, const char *body);

/* target_list_allocate() removes a node from a target_list */
void target_list_pop(struct target_list * const list, const char *body);

/* get_target() returns a target with the name name */
struct target *get_target(struct target_list * const list, const char *name);

//------------------------------------------------------------------------------

#endif
