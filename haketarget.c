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
#include "linked.h"

//------------------------------------------------------------------------------

void hake_target(char *targetname)
{
  // makes sure target exists in parse_targets
  struct target *target_to_verify;

  if ((target_to_verify = get_target(parsed_targets, targetname)) == NULL)
  {
      fprintf(stderr, "%s: fatal error: don't know how to hake %s\n", 
			 prog, targetname);
		exit(EXIT_FAILURE);
  }

  // verify target (check recursively_protected_targets, determine if prereqs are targets/filenames)
  verify_target(target_to_verify);

  // recipes_to_print should now be filled
  if (recipes_to_print->head != NULL)
  {
	  string_list_print(recipes_to_print, 1);
  }
  else
  {
	  printf("'%s' is up to date.", target_to_verify->name);
  }

  return;
}

//------------------------------------------------------------------------------

void verify_target(struct target *parent_target)
{
	// Verify that the target in question is not being
	// "verified" farther up the recursive call chain to verify_target()
  if (get_target(recursively_protected_targets, parent_target->name) != NULL)
  {
    fprintf(stderr, "%s: error: recursive call to target '%s'\n",
			 prog, parent_target->name);

		puts("Current target-chain:");
		target_list_print(recursively_protected_targets, parent_target->name,
				" <--- target's target-chain specifies it as a prerequisite");

    exit(EXIT_FAILURE);
  }

  target_list_append(recursively_protected_targets, parent_target->name);

  // Look at each of the parent_target's prereqs to determine
  // if the target is up to date
  //verify_prerequisites(target_to_verify)
  struct string_node *current_prereq = parent_target->prereqs->head;
  struct target *child_target;
  struct stat file_info;

  // If the prerequisite list is NULL (the target has no prereqs),
  // the target must always be "haked", and its recipes printed
  if (current_prereq == NULL)
  {
		parent_target->up_to_date = 0;
  }

	while (current_prereq != NULL)
	{
		// If current_prereq is a recognized target,
		// call verify_target() and check whether it is up to date
		if ((child_target = get_target(parsed_targets, current_prereq->body)) != NULL)
		{
			verify_target(child_target);

			// If the child target is out of date,
			// the target in question is out of date, too,
			// so we print its recipes, too.
			if (!child_target->up_to_date)
			{
				parent_target->up_to_date = 0;
			}
    	}
		// If current_prereq is not a 
		// recognized target, we assume it is a file path.
		else
		{
      	// If stat() for the file path fails,
			// print an error and quit
			if (stat(current_prereq->body, &file_info) == -1)
      	{
        		fprintf(stderr, "%s: error: %s: %s\n",
				 		 prog, current_prereq->body, strerror(errno));
        		exit(EXIT_FAILURE);
      	}

      	// If the file path exists, we compare its
			// modification time to the access time for 
			// the hakefile that declared parent_target
			if (file_info.st_mtime > parent_target->file_access_time)
      	{
        		parent_target->up_to_date = 0;
      	}
    	}

		current_prereq = current_prereq->next;
	}

  // pop parent_target from recursively_protected_targets
  target_list_pop(recursively_protected_targets, parent_target->name);


  // if target_to_verify is out of date, add its recipes to the list of recipes to print
  if (!parent_target->up_to_date)
  {
    for (struct string_node *p = parent_target->recipes->head; p != NULL; p = p->next)
    {
      string_list_append(recipes_to_print, p->body);
    }

	 parent_target->up_to_date = 1;
  }

  return;
}

//------------------------------------------------------------------------------


#if 0
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

  // pop target_to_verify from recursively_protected_targets
  target_list_pop(recursively_protected_targets, target_to_verify->name);
}
#endif
//------------------------------------------------------------------------------
