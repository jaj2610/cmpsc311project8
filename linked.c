/* CMPSC 311, Spring 2013, Project 8
 * 
 * Author: Jacob Jones
 * Email: jaj5333@psu.edu
 * 
 * Author: Scott Cheloha
 * Email: ssc5145@psu.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

#include "wrapper.h"
#include "linked.h"



//------------------------------------------------------------------------------

struct string_list *string_list_allocate(void)
{
  struct string_list *list = Malloc(sizeof(struct string_list), __func__, __LINE__);

  list->head = list->tail = NULL;

  return list;
}

//------------------------------------------------------------------------------

void string_list_deallocate(struct string_list * const list)
{
  struct string_node *prev = NULL;
  for (struct string_node *p = list->head; p != NULL; p = p->next)
  {
      Free(prev, __func__, __LINE__);	// Free(NULL) is harmless
      Free(p->body, __func__, __LINE__);
      prev = p;
  }
  Free(prev, __func__, __LINE__);		// now, p == list->tail

  Free(list, __func__, __LINE__);
}

//------------------------------------------------------------------------------

void string_list_print(const struct string_list * const list)
{
	if (list->head != NULL)
	{
		for (struct string_node *p = list->head; p != NULL; p = p->next)
		{
    		printf("  %s\n",
				p->body);
		}
	}

	printf("  -- end of list\n");
}


//------------------------------------------------------------------------------

void string_list_append(struct string_list * const list, const char *body)
{
  struct string_node *p = Malloc(sizeof(struct string_node), __func__, __LINE__);

  p->next = NULL;
  p->body = Strdup(body,  __func__, __LINE__);

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

void string_list_pop(struct string_list * const list, const char *body)
{
	struct string_node *prev = NULL;

  for (struct string_node *p = list->head; p != NULL; p = p->next)
  {
    if (p->body == body)
     {
        if (prev == NULL)
        {
          list->head = p->next;
        }
        else
        {
          prev->next = p->next;
        }

        Free(p->body, __func__, __LINE__);
        Free(p, __func__, __LINE__);
        return;
    }

    prev = p;
  }

}

//------------------------------------------------------------------------------

struct string_node *get_string(struct string_list * const list, const char *body)
{
  for (struct string_node *p = list->head; p != NULL; p = p->next)
  {
    if (strcmp(p->body,body) == 0)
    {
      return p;
    }
  }

  // return NULL if not found
  return NULL;
}

//------------------------------------------------------------------------------

// if (body is on the list already) { return 1 }
// else { put body on the list and return 0 }

int string_list_append_if_new(struct string_list * const list, const char *body)
{
  if (get_string(list, body) != NULL)
  { 
    return 1;
  }   // body is on the list already

  string_list_append(list, body);

  return 0;
}

//------------------------------------------------------------------------------

struct target_list *target_list_allocate(void)
{
  struct target_list *list = Malloc(sizeof(struct target_list), __func__, __LINE__);

  list->head = list->tail = NULL;

  return list;
}

//------------------------------------------------------------------------------

void target_list_deallocate(struct target_list * const list)
{
	struct target *prev = NULL;
	for (struct target *p = list->head; p != NULL; p = p->next)
	{
      Free(prev, __func__, __LINE__); // Free(NULL) is harmless
      Free(p->name, __func__, __LINE__);
      string_list_deallocate(p->prereqs);
      string_list_deallocate(p->recipes);
      prev = p;
  }
  Free(prev, __func__, __LINE__);   // now, p == list->tail

  Free(list, __func__, __LINE__);
}

//------------------------------------------------------------------------------

void target_list_print(const struct target_list * const list,
		char *targetname_to_comment, char *comment_text)
{
	//	puts("current parsed targets:");
	if (list->head != NULL)
	{
      for (struct target *p = list->head; p != NULL; p = p->next)
      {
          printf("  %s",
            	p->name);

			 if (targetname_to_comment != NULL 
				  && strcmp(p->name, targetname_to_comment) == 0)
			 {
				printf("%s", comment_text);
			 }

			 puts("");
		}
	}
  puts("  -- end of list");
}

//------------------------------------------------------------------------------

struct target *target_list_append(struct target_list * const list, const char *name)
{
  struct target *p = Malloc(sizeof(struct target), __func__, __LINE__);

  p->next = NULL;
  p->name = Strdup(name,  __func__, __LINE__);
  p->prereqs = string_list_allocate();
  p->recipes = string_list_allocate();
  p->up_to_date = 1;

  if (list->head == NULL) // empty list, list->tail is also NULL
  {
    list->head = list->tail = p;
  }
  else
  {
    list->tail->next = p;
    list->tail = p;
  }

  return p;
}

//------------------------------------------------------------------------------

void target_list_pop(struct target_list * const list, const char *name)
{
  struct target *prev = NULL;

  for (struct target *p = list->head; p != NULL; p = p->next)
  {
    if (p->name == name)
    {
      if (prev == NULL)
      {
        list->head = p->next;
      }
      else
      {
        prev->next = p->next;
      }

      Free(p->name, __func__, __LINE__);
      string_list_deallocate(p->prereqs);
      string_list_deallocate(p->recipes);
      Free(p, __func__, __LINE__);

      return;
    }

    prev = p;
  }  
}

//------------------------------------------------------------------------------

struct target *get_target(struct target_list * const list, const char *name)
{
  for (struct target *p = list->head; p != NULL; p = p->next)
  {
    if (strcmp(p->name, name) == 0)
    {
      return p;
    }
  }

  // return NULL if not found
  return NULL;
}
