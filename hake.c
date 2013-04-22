/* CMPSC 311, Spring 2013, Project 5 solution
 *
 * Author:   Don Heller
 * Email:    dheller@cse.psu.edu
 *
 * Hake -- a fishy version of Make
 *
 * version 2, 25 Feb. 2013
 *   macro definition, expansion
 *   all required Project 5 features implemented
 * version 1, 25 Feb. 2013
 *   Project 4 solution
 * 
 * Command-line Options
 *    -h           print help
 *    -v           verbose mode; enable extra printing; can be repeated
 *    -f file      input filename; default is hakefile or Hakefile
 *
 */

//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "cmpsc311.h"
#include "names.h"
#include "macro.h"

//------------------------------------------------------------------------------

static int read_file(char *filename, int quiet);
  // return 1 if successful, 0 if not
  // "success" means the file could be opened for reading, or that we had seen
  //    the file before and don't need to read it again
  // quiet == 0 enables error messages if the file can't be opened
  // quiet == 1 suppresses error messages if the file can't be opened

static void read_lines(char *filename, FILE *fp);
  // fp comes from the file (named filename) opened by read_file() using fopen()

// maximum line length in an input file (buffer size in read_lines)
#define MAXLINE 4096

//------------------------------------------------------------------------------

static void usage(int status)
{
  if (status == EXIT_SUCCESS)
    {
      printf("usage: %s [-h] [-v] [-f file]\n", prog);
      printf("  -h           print help\n");
      printf("  -v           verbose mode; enable extra printing; can be repeated\n");
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
  extern char *optarg;
  extern int optind;
  extern int optopt;
  extern int opterr;

  // program name as actually used
  prog = argv[0];
  /* In extremely strange situations, argv[0] could be NULL, or point to an
   * empty string.  Let's just ignore that for now.
   */

  // exit status
  int status = EXIT_SUCCESS;

  // option flags and option-arguments set from the command line
  int f_flag = 0;	// number of -f options supplied

  // first pass, everything except -f options (let the -v options accumulate)
  while ((ch = getopt(argc, argv, ":hvf:")) != -1)
    {
      switch (ch) {
        case 'h':
          usage(EXIT_SUCCESS);
          break;
        case 'v':
          verbose++;
          break;
	case 'f':
	  // later
	  break;
        case '?':
          fprintf(stderr, "%s: invalid option '%c'\n", prog, optopt);
          usage(EXIT_FAILURE);
          break;
        case ':':
          fprintf(stderr, "%s: invalid option '%c' (missing argument)\n", prog, optopt);
          usage(EXIT_FAILURE);
          break;
        default:
          usage(EXIT_FAILURE);
          break;
      }
    }

  // scan the argv array again, from the beginning
  optind = 1;
  while ((ch = getopt(argc, argv, ":hvf:")) != -1)
    {
      switch (ch) {
	case 'f':
	  f_flag++;		// number of -f options supplied
	  (void) read_file(optarg, 0);
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

  // OK, we got all this data, now what?  That's a later project.

  for (int i = optind; i < argc; i++)
    {
      printf("  target selected: %s\n", argv[i]);
    }

  return status;
}

//------------------------------------------------------------------------------

// return 1 if successful, 0 if not
// "success" means the file could be opened for reading, or that we had seen
//    the file before and don't need to read it again
// quiet == 0 enables error messages if the file can't be opened
// quiet == 1 suppresses error messages if the file can't be opened

static int read_file(char *filename, int quiet)
{
  verify(filename != NULL, "null arg filename");
  verify(filename[0] != '\0', "empty arg filename");

  if (verbose > 0)
    { fprintf(stderr, "%s: read_file(%s)\n", prog, filename); }

  // file names come from -f and include
  static struct list_names * filenames = NULL;
  if (filenames == NULL) filenames = list_names_allocate("filenames");
    // by construction, filenames is now not NULL

  if (verbose > 1)
    { list_names_print(filenames); }

  // if (filename is on the list already) { return 1 }
  // else { put filename on the list and continue }
  if (list_names_append_if_new(filenames, filename) == 1)
    { return 1; }

  if (verbose > 0)
    { list_names_print(filenames); }

  if (strcmp(filename, "-") == 0)
    { read_lines("[stdin]", stdin); return 1; }

  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
    {
      if (quiet == 0)
	fprintf(stderr, "%s: could not open input file %s: %s\n", prog, filename, strerror(errno));
      return 0;
    }

  read_lines(filename, fp);

  if (fclose(fp) != 0)
    {
      fprintf(stderr, "%s: could not close input file %s: %s\n", prog, filename, strerror(errno));
    }

  return 1;
}

//------------------------------------------------------------------------------

static void read_lines(char *filename, FILE *fp)
{
  verify(filename != NULL, "null arg filename");
  verify(filename[0] != '\0', "empty arg filename");
  verify(fp != NULL, "null arg fp");

  if (verbose > 0)
    { fprintf(stderr, "%s: read_lines(%s)\n", prog, filename); }

  char original[MAXLINE+2];	// from fgets()
  char expanded[MAXLINE+2];	// after macro expansion
  char buffer[MAXLINE+2];	// working copy, safe to modify

  char whsp[] = " \t\n\v\f\r";			// whitespace characters
  int line_number = 0;
  int recipe_line_number = 0;

  bool have_target = false;			// recipes must follow targets

  while (fgets(original, MAXLINE, fp) != NULL) {
    // it is possible that the input line was too long, so terminate the string cleanly
    original[MAXLINE] = '\n';
    original[MAXLINE+1] = '\0';

    line_number++;
    if (verbose > 0) printf("%s: %s: line %d: %s", prog, filename, line_number, original);

    // assume original[] is constructed properly
    // assume expanded[] is large enough
    macro_expand(original, expanded);
    if (verbose > 0) printf("%s: %s: line %d: %s", prog, filename, line_number, expanded);

    strcpy(buffer, expanded);			// copy, safe to modify

    char *buf = buffer;

    while (*buf == ' ') buf++;			// skip past leading spaces (not tabs!)

    char *p_hash = strchr(buf, '#');		// a comment starts with #
    if (p_hash != NULL)
      { *p_hash = '\0'; }			// remove the comment

    int n = 0;					// remove trailing whitespace
    while (buf[n] != '\0')
      {
        int n1 = strspn(&buf[n], whsp);		// buf[n .. n+n1-1] is whitespace
        int n2 = strcspn(&buf[n + n1], whsp);	// buf[n+n1 .. n+n1+n2-1] is not
        if (n2 == 0) { buf[n] = '\0'; break; }	// remove trailing whitespace
        n += n1 + n2;
      }

    if (buf[0] == '\0')				// nothing left?
      { continue; }

    char *p_colon = strchr(buf, ':');		// : indicates a target-prerequisite line
    char *p_equal = strchr(buf, '=');		// = indicates a macro definition

    if (buffer[0] == '\t')
      {
	recipe_line_number++;
        if (verbose > 0) printf("  diagnosis: recipe line %d\n", recipe_line_number);
	if (have_target == false)
	  {
	    fprintf(stderr, "%s: %s: line %d: recipe but no target\n", prog, filename, line_number);
	    continue;
	  }
	// (save this for a later project)
      }
    else if (p_colon != NULL)
      {
	recipe_line_number = 0;
        if (verbose > 0) printf("  diagnosis: target-prerequisite\n");
	have_target = true;
	// (save this for a later project)
      }
    else if (p_equal != NULL)
      {
        if (verbose > 0) printf("  diagnosis: macro definition\n");
	have_target = false;
        // name = body
        // *p_equal is '='
        char *name_start = buf;
        while (*name_start == ' ' || *name_start == '\t')       // skip past spaces and tabs
          { name_start++; }
        char *name_end = p_equal-1;
        while (*name_end == ' ' || *name_end == '\t')
          { name_end--; }
        name_end++;
        *name_end = '\0';
        char *body_start = p_equal+1;
        while (*body_start == ' ' || *body_start == '\t')
          { body_start++; }
        char *body_end = body_start;
        while (*body_end != '\0')       // end of string
          { body_end++; }
        while (*body_end == ' ' || *body_end == '\t')
          { body_end--; }
        body_end++;
        *body_end = '\0';
        if (verbose > 1) macro_list_print();
        macro_set(name_start, body_start);
        if (verbose > 1) macro_list_print();
      }
    else if (strncmp("include", buf, 7) == 0)
      {
        if (verbose > 0) printf("  diagnosis: include\n");
	have_target = false;
	char *name_start = buf + 7;				// skip past "include"
	while (*name_start == ' ' || *name_start == '\t')	// skip past spaces and tabs
	  { name_start++; }
	if (*name_start == '\0')
	  {
	    // following GNU Make, this is not an error
	    if (verbose > 0) fprintf(stderr, "%s: %s: line %d: include but no filename\n", prog, filename, line_number);
	    continue;
	  }
	else if (*name_start == '\'' || *name_start == '"')		// quoted filename
	  {
	    // find matching quote, remove it
	    char *q = name_start + 1;				// skip past ' or "
	    while (*q != *name_start && *q != '\0') q++;	// find end of string or line
	    if (*q == '\0')
	      {
		fprintf(stderr, "%s: %s: line %d: file name error [%s]\n", prog, filename, line_number, name_start);
		continue;
	      }
	    name_start++;	// skip past opening quote
	    *q = '\0';		// remove closing quote
	  }
        read_file(name_start, 0);
      }
    else
      {
        if (verbose > 0) printf("  diagnosis: something else\n");
	have_target = false;
	fprintf(stderr, "%s: %s: line %d: not recognized: %s", prog, filename, line_number, original);
      }
  }

  if (ferror(fp))	// error when reading the file
    { fprintf(stderr, "%s: %s: read error: %s\n", prog, filename, strerror(errno)); }

  return;
}

//------------------------------------------------------------------------------

