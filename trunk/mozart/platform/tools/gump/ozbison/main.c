/* Top level entry point of bison,
   Copyright (C) 1984, 1986, 1989, 1992, 1995 Free Software Foundation, Inc.

This file is part of Bison, the GNU Compiler Compiler.

Bison is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

Bison is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Bison; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#include <stdio.h>
#include <setjmp.h>
#include "system.h"
#include "machine.h"	/* for MAXSHORT */

#include <oz.h>

int verboseflag;

/* The name this program was run with, for messages.  */
char *program_name;
FILE *foutput = NULL;

extern void getargs(), openfiles(), reader(), reduce_grammar();
extern void set_derives(), set_nullable(), generate_states();
extern void lalr(), initialize_conflicts(), ozbison_verbose(), terse();
extern OZ_Term output();

static jmp_buf env;

void done(int k) {
  if (foutput) {
    fclose(foutput);
    foutput = NULL;
  }
  longjmp(env, k);
}

OZ_C_proc_begin(bison_generate, 3)
{
  int k;
  char *verbosefile = OZ_atomToC(OZ_getCArg(1));

  verboseflag = verbosefile[0] != '\0';

  if (verboseflag) {
    foutput = fopen(verbosefile, "w");
    if (foutput == NULL) {
      fprintf(stderr, "fatal error: cannot open output file %s\n", verbosefile);
      done (1);
    }
  }

  if ((k = setjmp(env)) != 0)
    return OZ_raiseC("ozbison", 1, OZ_int(k));

  /* read the input.
     In file reader.c.
     The other parts are recorded in the grammar; see gram.h.  */
  reader(OZ_getCArg(0));

  /* find useless nonterminals and productions and reduce the grammar.  In
     file reduce.c */
  reduce_grammar();

  /* record other info about the grammar.  In files derives and nullable.  */
  set_derives();
  set_nullable();

  /* convert to nondeterministic finite state machine.  In file LR0.
     See state.h for more info.  */
  generate_states();

  /* make it deterministic.  In file lalr.  */
  lalr();

  /* Find and record any conflicts: places where one token of lookahead is not
     enough to disambiguate the parsing.  In file conflicts.
     Also resolve s/r conflicts based on precedence declarations.  */
  initialize_conflicts();

  /* print information about results, if requested.  In file print. */
  if (verboseflag) {
    ozbison_verbose();
    fclose(foutput);
    foutput = NULL;
  } else
    terse();

  /* output the tables and the parser to ftable.  In file output. */
  return OZ_unify(output(), OZ_getCArg(2));
}
OZ_C_proc_end

/* functions to report errors which prevent a parser from being generated */


/* Print a message for the fatal occurence of more than MAXSHORT
   instances of whatever is denoted by the string S.  */

void
toomany(s)
     char *s;
{
  fprintf(stderr, "fatal error: limit of %d exceeded, too many %s\n", MAXSHORT, s);
  done(1);
}

/* Abort for an internal error denoted by string S.  */

void
berror(s)
     char *s;
{
  fprintf(stderr, "internal error, %s\n", s);
  abort();
}
