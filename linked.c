/* CMPSC 311, Spring 2013, Project 7
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
#include "pr7.h"



//------------------------------------------------------------------------------

struct process_list *process_list_allocate(void)
{
  struct process_list *list = Malloc(sizeof(struct process_list), __func__, __LINE__);

  list->head = list->tail = NULL;
  list->reference_count = 0;

  return list;
}

//------------------------------------------------------------------------------

void process_list_deallocate(struct process_list * const list)
{
  if (--list->reference_count > 0)
    { return; }

  struct node *prev = NULL;
  for (struct node *p = list->head; p != NULL; p = p->next)
  {
      Free(prev, __func__, __LINE__);	// Free(NULL) is harmless
      Free(p->command, __func__, __LINE__);
      prev = p;
  }
  Free(prev, __func__, __LINE__);		// now, prev == list->tail

  Free(list, __func__, __LINE__);
}

//------------------------------------------------------------------------------

void process_list_print(const struct process_list * const list)
{
  if (list->head == NULL)
    { ; }
  else
    {
      puts("Processes:");
      puts("   PID       PGID    STATUS     CMD");
      for (struct node *p = list->head; p != NULL; p = p->next)
		{ 
			printf("   [%d]   [%d]   [%s]   %s\n",
					(int) p->pid, (int) p->pgid, p->status, p->command); 
		}
    }
}

//------------------------------------------------------------------------------

void process_list_append(struct process_list * const list, const char *command, pid_t pid, pid_t pgid)
{
  struct node *p = Malloc(sizeof(struct node), __func__, __LINE__);

  p->next = NULL;
  p->status = Strdup("RUNNING",  __func__, __LINE__);
  p->pid = pid;
  p->pgid = pgid;
  p->command = Strdup(command,  __func__, __LINE__);

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

void process_list_pop(struct process_list * const list, pid_t pid)
{
	struct node *prev = NULL;

	for (struct node *p = list->head; p != NULL; p = p->next)
	{
    if (p->pid == pid)
    {
      if (prev == NULL)
      {
        list->head = p->next;
      }
      else
      {
        prev->next = p->next;
      }

      printf("\n--%s: %s (%d) has terminated.\n",
      	prog, p->command, (int) p->pid);
      
		  /* Free allocated resources for node */
      Free(p->command, __func__, __LINE__);
      Free(p->status, __func__, __LINE__);
		  Free(p, __func__, __LINE__);

      return;
    }

		prev = p;
	}

  fprintf(stderr, "--%s: process %d is not currently running.\n", prog, (int) pid);
}
