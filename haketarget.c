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
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "haketarget.h"
#include "hake.h"

//------------------------------------------------------------------------------

void hake_target(char *targetname)
{
  // makes sure target exists in parse_targets
  struct target *target_to_verify;

  if ((target_to_verify = get_target(parse_targets, targetname)) == NULL)
  {
    fprintf(stderr, "error: don't know how to hake %s\n", targetname);
    exit(1);
  }

  // verify target (check recursively_protected_targets, determine if prereqs are targets/filenames)
  verify_target(target_to_verify);

  // recipes_to_print should now be filled
  string_list_print(recipes_to_print);

  return;
}

//------------------------------------------------------------------------------

void verify_target(struct target *target_to_verify)
{
	// we need to ensure that target_to_verify cannot
  // specify itself as a prerequisite and that no subtarget can specify it as a prerequisite
  if ((target_to_verify = get_target(recursively_protected_targets, targetname)) == NULL)
  {
    target_list_append(recursively_protected_targets, targetname);
  }
  else
  {
    fprintf(stderr, "error: %s is recursively protected\n", targetname);
    exit(1);
  }

  // verify the target's prerequisites
  verify_prerequisites(target_to_verify);

  // if the target_to_verify needs to be haked, add its recipes to the list of recipes to print
  if (target_to_verify.needs_to_be_haked)
  {
    for (struct string_list *p = target_to_verify->recipes->head; p != NULL; p = p->next)
    {
      string_list_append(target_to_verify->recipes, recipes_to_print);
    }
  }

  return;
}

//------------------------------------------------------------------------------

void verify_prerequisites(struct target *target_to_verify)
{
  struct string_node *current_prereq = target_to_verify->recipes->head;
  struct stat file_info;
  struct target *local_target;

  while (current_prereq != NULL)
  {
    if ((local_target = get_target(parse_targets, current_prereq->body)) != NULL)
    {
      verify_target(local_target->name);
    }
    else
    {
      // assume filename and deal with current_prereq as such
      if (stat(current_prereq->body, &file_info) == -1)
      {
        fprintf(stderr, "error: file %s is not found\n", current_prereq->body);
        exit(1);
      }

      // compare file modify times
      if (file_info.st_mtime > target_to_verify->file_access_time)
      {
        target_to_verify->needs_to_be_haked = 1;
      }
    }

    current_prereq = current_prereq->next;
  }

  // pop target_to_verify off of recursively_protected_targets
  target_list_pop(recursively_protected_targets, target_to_verify->name);
}

//------------------------------------------------------------------------------
