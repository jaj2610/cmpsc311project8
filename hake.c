/* CMPSC 311, Spring 2013, Project 8
 * 
 * Author: Jacob Jones
 * Email: jaj5333@psu.edu
 * 
 * Author: Scott Cheloha
 * Email: ssc5145@psu.edu
 * 
 * Original Author: Don Heller
 * Email:    dheller@cse.psu.edu
 * 
 * Command-line Options
 *    -h           print help
 *    -v           v_flag mode; enable extra printing; can be repeated
 *    -f file      input filename; default is hakefile or Hakefile
 *
 */

//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "linked.h"
#include "macro.h"
#include "hake.h"
#include "wrapper.h"
#include "haketarget.h"

//------------------------------------------------------------------------------

// option flags and option-arguments set from the command line
char *prog;

struct string_list *filenames;
struct target_list *parsed_targets;

struct string_list *recipes_to_print;
struct target_list *recursively_protected_targets;



int v_flag = 0;
int d_flag = 0;
int f_flag = 0;	// number of -f options supplied


//------------------------------------------------------------------------------

static void usage(int status)
{
	if (status == EXIT_SUCCESS)
	{
		printf("usage: %s [-h] [-v] [-f file]\n", prog);
		printf("  -h           print help\n");
		printf("  -v           v_flag mode; enable extra printing; can be repeated\n");
		printf("  -f file      input filename; default is hakefile or Hakefile\n");
	}
	else
	{
		fprintf(stderr, "%s: Try '%s -h' for usage information.\n", prog, prog);
	}

	exit(status);
}

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	// for use with getopt(3)
	int ch;

	// allocate memory for global lists
	filenames = string_list_allocate();
	parsed_targets = target_list_allocate();
	recipes_to_print = string_list_allocate();
	recursively_protected_targets = target_list_allocate();

	extern char *optarg;
	extern int optind;
	extern int optopt;
	extern int opterr;

	// program name as actually used
	prog = argv[0];
	/* In extremely strange situations, argv[0] could be NULL, or point to an
	 * empty string.  Let's just ignore that for now.
	 */

	// first pass, everything except -f options (let the -v options accumulate)
	while ((ch = getopt(argc, argv, ":hvdf:")) != -1)
	{
		switch (ch)
		{
			case 'h':
				usage(EXIT_SUCCESS);
		 		break;
			case 'v':
				v_flag = 1;
				break;
			case 'd':
				d_flag = 1;
				break;
			case 'f':
				// later
				break;
			case '?':
				fprintf(stderr, "%s: invalid option '%c'\n", prog, optopt);
				usage(EXIT_FAILURE);
				break;
			case ':':
				fprintf(stderr, "%s: invalid option '%c' (missing argument)\n",
						prog, optopt);
				usage(EXIT_FAILURE);
				break;
			default:
				usage(EXIT_FAILURE);
				break;
		}
	}

	// Scan the argv array again and process the -f arguments
	optind = 1;
	int error_count = 0;
	while ((ch = getopt(argc, argv, ":hvdf:")) != -1)
	{
		switch (ch) 
		{
			case 'f':
				f_flag++;		// number of -f options supplied
				error_count += read_file(optarg, 0);
				break;
			default:
				break;
		}
	}

	if (f_flag == 0 && !read_file("hakefile", 1) && !read_file("Hakefile", 1))
	{
		fprintf(stderr, "%s: no input\n", prog);
		usage(EXIT_FAILURE);
	}

	if (error_count != 0)
	{
		fprintf(stderr, "%s: error count is %d, cannot hake target(s)\n",
				prog, error_count);
		exit(EXIT_FAILURE);
	}
	
	// Print the recipes for the targets that are out of date
	for (int i = optind; i < argc; i++)
 	{
		hake_target(argv[i]);
	}

	exit(EXIT_SUCCESS);
}

//------------------------------------------------------------------------------

// return 0 if successful, >0 if not
// "success" means the file could be opened for reading, or that we had seen
//    the file before and don't need to read it again
// quiet == 0 enables error messages if the file can't be opened
// quiet == 1 suppresses error messages if the file can't be opened
int read_file(char *filename, int quiet)
{
	if (v_flag)
	{
		fprintf(stderr, "%s: read_file(%s)\n", prog, filename);
	}

	// if (filename is on the list already) { return 1 }
	// else { put filename on the list and continue }
	if (string_list_append_if_new(filenames, filename) == 1)
	{
		fprintf(stderr, "%s: error: file '%s' already included or parsed\n",
				prog, filename);
		return 1;
	}

	if (v_flag)
	{
		puts("Filenames:");
		string_list_print(filenames, 0);
	}

	struct stat info;

	if (strcmp(filename, "-") == 0)
	{
		time_t t;
		return read_lines("[stdin]", stdin, t);
	}
	else if (stat(filename, &info) == -1)
	{
		fprintf(stderr, "%s: could not stat() input file %s: %s\n",
				prog, filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	FILE *fp;
	
	if ((fp = Fopen(filename, "r", quiet, __func__, __LINE__)) == NULL)
	{
		if (quiet == 0)
		{
			fprintf(stderr, "%s: could not open input file %s: %s\n",
					prog, filename, strerror(errno));

			exit(EXIT_FAILURE);
		}
	
		return 1;
	}

	int error_count = read_lines(filename, fp, info.st_atime);

	if (Fclose(fp, __func__, __LINE__) != 0)
	{
		fprintf(stderr, "%s: could not close input file %s: %s\n",
				prog, filename, strerror(errno));
	}

	return error_count;
}

//------------------------------------------------------------------------------

int read_lines(char *filename, FILE *fp, time_t file_access_time)
{
	if (v_flag)
	{
		fprintf(stderr, "%s: read_lines(%s)\n", prog, filename);
		printf("\tfile last accessed: %s", ctime(&file_access_time));
	}

	char original[MAXLINE+2];	// from fgets()
	char expanded[MAXLINE+2];	// after macro expansion
	char buffer[MAXLINE+2];	// working copy, safe to modify

	int line_number = 0;

	bool have_target = false;			// recipes must follow targets
	struct target *current_target = NULL;
	struct string_list *current_recipes = NULL;

	int error_count = 0;

	while (fgets(original, MAXLINE, fp) != NULL)
	{
	/* --- Clean up targets and recipes before reading next line --- */
		if (have_target && current_recipes == NULL)
		{
			current_recipes = string_list_allocate();
		}
		
		if (!have_target && current_recipes != NULL)
		{
			if (current_recipes->head != NULL)
			{
				string_list_deallocate(current_target->recipes);
				current_target->recipes = current_recipes;
			}

			if (v_flag)
			{
				printf("%s: processed recipes for target '%s'\nRecipe list:\n",
						prog, current_target->name);
				string_list_print(current_target->recipes, 0);
			}
			
			current_target = NULL;
			current_recipes = NULL;
		}

		// it is possible that the input line was too long,
		// so terminate the string cleanly
		original[MAXLINE] = '\n';
		original[MAXLINE+1] = '\0';

		line_number++;
	
	/* --- Expand macros on the line --- */

		// assume original[] is constructed properly
		// assume expanded[] is large enough
		macro_expand(original, expanded);
		if (d_flag)
		{
			printf("%s: %s: line %d: %s", 
					prog, filename, line_number, expanded);
		}

		strcpy(buffer, expanded);	// copy, safe to modify

		char *buf = buffer;

	/* --- Clean up extraneous whitespace, check for strange errors --- */
		clean_up_whitespace(buf);
		
		// If we have an empty line, ignore it
		if (buf[0] == '\0')
		{
			continue;
		}

		// A ':' or '=' indicates a target-prerequisite or macro line
		char *first_colon_or_equal = strpbrk(buf, ":=");

		// Check for illegal ':' or '=' occuring after initial ':' or '='
		char *extra_colon_or_equal;

		if (first_colon_or_equal != NULL 
				&& (extra_colon_or_equal = strpbrk(first_colon_or_equal+1, "=:")) != NULL)
		{
			error_count++;

			fprintf(stderr, "%s: %s:%d: error: unexpected '%c' found after initial '%c'\n",
				prog, filename, line_number, *extra_colon_or_equal, *first_colon_or_equal);

			continue;
		}

	/* --- Take appropriate action for each line type --- */
		if (buf[0] == '\t')
		{
			if (have_target == false)
			{
				error_count++;
				fprintf(stderr, "%s: %s:%d: error: recipe without target\n",
						prog, filename, line_number);
				
				continue;
			}
			
			// append the rest of the line following the initial '\t'
			// to the local recipe list
			string_list_append(current_recipes, buf+1);
		}
		else if (*first_colon_or_equal == ':')
		{
			// If we already have a target,
			// attempt to append current_recipes
			// to the target's list of recipes
			if (have_target)
			{
				if (current_recipes->head != NULL)
				{
					string_list_deallocate(current_target->recipes);
					current_target->recipes = current_recipes;
				}

				if (v_flag)
				{
					printf("%s: processed recipes for target '%s'\nRecipe list:\n",
							prog, current_target->name);
					string_list_print(current_target->recipes, 0);
				}
				
				current_target = NULL;
				current_recipes = NULL;
			}

			// Attempt to make a new target
			if ((current_target = parse_target(buf, first_colon_or_equal, 
							filename, file_access_time, line_number)) != NULL)
			{
				have_target = true;

				// If verbose, print current target list
				if (v_flag)
				{
					printf("%s: parsed target '%s'\n",
							prog, current_target->name);
					puts("Parsed targets:");
					target_list_print(parsed_targets, NULL, NULL);
				}
			}
			else
			{
				error_count++;
				have_target = false;

				fprintf(stderr, "%s: %s:%d: error: failed to parse target\n",
						prog, filename, line_number);
			}
		}
		else if (*first_colon_or_equal == '=')
		{
			have_target = false;

			if ((parse_macro(buf, first_colon_or_equal, filename, line_number)) == 1)
			{
				error_count++;
				fprintf(stderr, "%s: %s:%d: error: failed to parse macro\n",
						prog, filename, line_number);
			}
			else
			{
				// If verbose, print current macro list
				if (v_flag)
				{
					printf("Macro list:\n");
					macro_list_print();
				}
			}
		}
		else if (strncmp("include", buf, 7) == 0)
		{
			if (v_flag)
			{
				printf("  diagnosis: include\n");
			}

			have_target = false;

			parse_include(buf, filename, line_number);
		}
		else
		{
			error_count++;
			have_target = false;
			fprintf(stderr, "%s: %s:%d: error: not recognized: %s",
					prog, filename, line_number, original);
		}
	}

	/* Clean up recipes from while-loop */
	if (have_target)
	{
		if (current_recipes->head != NULL)
		{
			string_list_deallocate(current_target->recipes);
			current_target->recipes = current_recipes;
		}

		if (v_flag)
		{
			printf("%s: processed recipes for target '%s'\nRecipe list:\n",
					prog, current_target->name);
			string_list_print(current_target->recipes, 0);
		}
		
		current_target = NULL;
		current_recipes = NULL;
	}

	// error when reading the file
	if (ferror(fp))
	{
		fprintf(stderr, "%s: %s: read error: %s\n",
				prog, filename, strerror(errno));
	}

	return error_count;
}

//------------------------------------------------------------------------------

struct target *parse_target(char *buf, char *p_colon,
		char *filename, time_t file_access_time, int line_number)
{
	// format:
	// 	target : prereq_1 prereq_2 prereq_3 . . .
	
	/* Parse target name */
	char *name_start = buf;
	while (*name_start == ' ' || *name_start == '\t') // skip past spaces and tabs
	{
		name_start++;
	}

	// Check for empty target name
	if (name_start == p_colon)
	{
		fprintf(stderr, "%s: %s:%d: error: empty target name\n",
				prog, filename, line_number);
		return NULL;
	}

	char *name_end = p_colon-1;
	while (*name_end == ' ' || *name_end == '\t')
	{
		name_end--;
	}

	name_end++;
	*name_end = '\0';

	/* If the name does not already exist,
	 * append new target with name to global parsed targets list.
	 * See hake.h for global list declaration. 
	 */
	struct target *target;
	if ((target = get_target(parsed_targets, name_start)) == NULL)
	{
		target = target_list_append(parsed_targets, name_start);
	}
	else
	{
		fprintf(stderr, "%s: %s:%d: warning: target '%s' already declared\n",
				prog, filename, line_number, name_start);
	}

	target->file_access_time = file_access_time;

	/* Parse the prerequisites separately,
	 * and add them to the new_target struct
	 * as a linked list of c-strings. 
	 */
	char *prereqs_start = p_colon+1;
	
	parse_prereqs(prereqs_start, target, filename, line_number);

	return target;
}

//------------------------------------------------------------------------------

struct string_list *parse_prereqs(char *prereqs, struct target *target,
		char *filename, int line_number)
{
	char *delimiter;

	// Check for illegal ':' or '=' occuring after
	// initial ':' in target : prerequisite line
	if ((delimiter = strpbrk(prereqs, "=:")) != NULL)
	{
		fprintf(stderr, "%s: %s:%d: error: unexpected '%c' found after initial ':'\n",
			prog, filename, line_number, *delimiter);
	}
	else
	{
		// Run through all of prereqs until endline or end of buffer.
		// If 
		while (*prereqs != '\n' && *prereqs != '\0')
		{
			char *p_start = prereqs;

			// skip whitespace and tabs at beginning of p_start
			while (*p_start == ' ' || *p_start == '\t')
			{
				p_start++;
			}

			// end
			if (*p_start == '\n' || *p_start == '\0')
			{
				return target->prereqs;
			}

			// set p_end to next whitespace or '\0' after p_start
			char *p_end = p_start;

			while (*p_end != ' ' && *p_end != '\t' && *p_end != '\n' && *p_end != '\0')
			{
				p_end++;
			}

			// p_end to '\0' to end p_start
			*p_end = '\0';


			// append to newtarget's prereqs list
			string_list_append_if_new(target->prereqs, p_start);

			// skip to character after parsed prereq
			prereqs = p_end + 1;
		}
	}

	return target->prereqs;
}

//------------------------------------------------------------------------------

int parse_macro(char *buf, char *p_equal, 
		const char *filename, int line_number)
{
	// name = body
	// *p_equal is '='
	char *name_start = buf;
	while (*name_start == ' ' || *name_start == '\t') // skip past spaces and tabs
	{
		name_start++;
	}

	// check for empty macro name
	if (name_start == p_equal)
	{
		fprintf(stderr, "%s: %s:%d: error: empty macro name:\n",
				prog, filename, line_number);
		return 1;
	}

	char *name_end = p_equal-1;
	while (*name_end == ' ' || *name_end == '\t')
	{
		name_end--;
	}

	name_end++;
	*name_end = '\0';
	char *body_start = p_equal+1;

	while (*body_start == ' ' || *body_start == '\t')
	{
		body_start++;
	}

	char *body_end = body_start;
	while (*body_end != '\0')       // end of string
	{
		body_end++;
	}
	while (*body_end == ' ' || *body_end == '\t')
	{
		body_end--;
	}

	body_end++;
	*body_end = '\0';

	macro_set(name_start, body_start);

	if (v_flag)
	{
		printf("%s: defined macro '%s'\n",
				prog, name_start);
	}

	return 0;
}

void parse_include(char *buf, const char *filename, int line_number)
{
	char *name_start = buf + 7;				// skip past "include"
	
	while (*name_start == ' ' || *name_start == '\t')	// skip past spaces and tabs
	{
		name_start++;
	}

	if (name_start == buf + 7)
	{
		fprintf(stderr, "%s: %s:%d: error: no space between include and filename\n",
				prog, filename, line_number);
		exit(EXIT_FAILURE);
	}

	if (*name_start == '\0')
	{
		// following GNU Make, this is not an error
		if (v_flag) 
		{
			fprintf(stderr, "%s: %s:%d: error: include without filename\n",
					prog, filename, line_number);
		}
		
		return;
	}
	else if (*name_start == '\'' || *name_start == '"')		// quoted filename
		{
		// find matching quote, remove it
		char *q = name_start + 1;				// skip past ' or "
		while (*q != *name_start && *q != '\0') 
		{
			q++;	// find end of string or line
		}

		if (*q == '\0')
		{
			fprintf(stderr, "%s: %s:%d: file name error [%s]\n",
					prog, filename, line_number, name_start);
			exit(EXIT_FAILURE);
		}

		name_start++;	// skip past opening quote
		*q = '\0';		// remove closing quote
	}

	if ((read_file(name_start, 0)) == 2)
	{
		fprintf(stderr, "%s: %s:%d: error: cannot recursively include %s\n",
				prog, filename, line_number, name_start);
		exit(EXIT_FAILURE);
	}
}

//------------------------------------------------------------------------------

void clean_up_whitespace(char *buf)
{
	char whsp[] = " \t\n\v\f\r";			// whitespace characters

	// skip past leading spaces (not tabs!)
	while (*buf == ' ')
	{
		buf++; 
	}

	// remove comment, if present
	char *p_hash = strchr(buf, '#');		// a comment starts with #
	if (p_hash != NULL)
	{
		*p_hash = '\0'; // remove the comment
	}

	// remove trailing whitespace
	int n = 0;					
	while (buf[n] != '\0')
	{
		int n1 = strspn(&buf[n], whsp);		// buf[n .. n+n1-1] is whitespace
		int n2 = strcspn(&buf[n + n1], whsp);	// buf[n+n1 .. n+n1+n2-1] is not

		if (n2 == 0)
		{
			buf[n] = '\0';
			break;
		}

		n += n1 + n2;
	}
}

