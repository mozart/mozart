/* Data definitions for internal representation of bison's input,
   Copyright (C) 1984, 1986, 1989, 1992 Free Software Foundation, Inc.

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


/* representation of the grammar rules:

ntokens is the number of tokens, and nvars is the number of variables
(nonterminals).  nsyms is the total number, ntokens + nvars.

Each symbol (either token or variable) receives a symbol number.
Numbers 0 to ntokens-1 are for tokens, and ntokens to nsyms-1 are for
variables.  Symbol number zero is the end-of-input token.  This token
is counted in ntokens.

The rules receive rule numbers 1 to nrules in the order they are written.
Actions and guards are accessed via the rule number.

The rules themselves are described by three arrays: rrhs, rlhs and
ritem.  rlhs[R] is the symbol number of the left hand side of rule R.
The right hand side is stored as symbol numbers in a portion of
ritem.  rrhs[R] contains the index in ritem of the beginning of the
portion for rule R.

If rlhs[R] is -1, the rule has been thrown out by reduce.c
and should be ignored.

The length of the portion is one greater
 than the number of symbols in the rule's right hand side.
The last element in the portion contains minus R, which
identifies it as the end of a portion and says which rule it is for.

The portions of ritem come in order of increasing rule number and are
followed by an element which is zero to mark the end.  nitems is the
total length of ritem, not counting the final zero.  Each element of
ritem is called an "item" and its index in ritem is an item number.

Item numbers are used in the finite state machine to represent
places that parsing can get to.

Precedence levels are recorded in the vectors sprec and rprec.
sprec records the precedence level of each symbol,
rprec the precedence level of each rule.
rprecsym is the symbol-number of the symbol in %prec for this rule (if any).

Precedence levels are assigned in increasing order starting with 1 so
that numerically higher precedence values mean tighter binding as they
ought to.  Zero as a symbol or rule's precedence means none is
assigned.

Associativities are recorded similarly in rassoc and sassoc.  */

#include <mozart.h>


#define	ISTOKEN(s)	((s) < ntokens)
#define	ISVAR(s)	((s) >= ntokens)


extern int nitems;
extern int nrules;
extern int nsyms;
extern int ntokens;
extern int nvars;

extern int src_total, rrc_total;
extern int nuseless_productions, nuseless_nonterminals;

extern short *ritem;
extern short *rlhs;
extern short *rrhs;
extern short *rprec;
extern short *rprecsym;
extern int *rprec_used;
extern short *sprec;
extern int *sprec_used;
extern short *rassoc;
extern short *sassoc;
extern OZ_Term *rline;		/* Source line number of each rule */

extern int start_symbol;


/* associativity values in elements of rassoc, sassoc.  */

#define RIGHT_ASSOC 1
#define LEFT_ASSOC 2
#define NON_ASSOC 3

/* error_token_number is the token number of the error token.  */

extern int error_token_number;
