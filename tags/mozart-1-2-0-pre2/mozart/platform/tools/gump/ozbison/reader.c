/* Input parser for bison
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


/* read in the grammar specification and record it in the format described in gram.h.

The entry point is reader().  */

#include <stdio.h>
#include <ctype.h>
#include "system.h"
#include "new.h"
#include "symtab.h"
#include "gram.h"

#include <mozart.h>

static void read_tokens(OZ_Term);
static void read_assocs(OZ_Term);
static void read_rules(OZ_Term);
static void packsymbols(void);
static void packgram(void);

typedef struct symbol_list {
  struct symbol_list *next;
  bucket *sym;
  bucket *ruleprec;
} symbol_list;

static symbol_list *grammar;
char **tags;

static OZ_Term leftAtom, rightAtom;

void reader(OZ_Term inputGrammar) {
  bucket *ssym;

  /* initialize the symbol table.  */
  tabinit();

  getsym("'EOF'");

  /* construct the error token */
  getsym("error");
  error_token_number = 1;

  /* construct a token that represents all undefined literal tokens. */
  /* it is always token number 2.  */
  getsym("'$undefined.'");

  leftAtom = OZ_atom("leftAssoc");
  rightAtom = OZ_atom("rightAssoc");
  read_tokens(OZ_getArg(inputGrammar, 1));
  read_assocs(OZ_getArg(inputGrammar, 2));
  read_rules(OZ_getArg(inputGrammar, 3));
  ssym = getsym(OZ_atomToC(OZ_getArg(inputGrammar, 0)));
  start_symbol = ssym->value;

  /* assign the symbols their symbol numbers.  */
  packsymbols();
  /* convert the grammar into the format described in gram.h.  */
  packgram();

  /* free the symbol table data structure
     since symbols are now all referred to by symbol number.  */
  free_symtab();

  src_total = rrc_total = 0;
  nuseless_productions = nuseless_nonterminals = 0;
}

static void read_tokens(OZ_Term tokens) {
  while (!OZ_eq(tokens, OZ_nil())) {
    getsym(OZ_atomToC(OZ_getArg(tokens, 0)));
    tokens = OZ_getArg(tokens, 1);
  }
  ntokens = nsyms;
}

static void read_assocs(OZ_Term assocs) {
  OZ_Term triple, assocAtom;
  bucket *symbol;

  while (!OZ_eq(assocs, OZ_nil())) {
    triple = OZ_getArg(assocs, 0);
    symbol = getsym(OZ_atomToC(OZ_getArg(triple, 0)));
    assocAtom = OZ_getArg(triple, 1);
    if (OZ_eq(assocAtom, leftAtom))
      symbol->assoc = LEFT_ASSOC;
    else if (OZ_eq(assocAtom, rightAtom))
      symbol->assoc = RIGHT_ASSOC;
    else
      symbol->assoc = NON_ASSOC;
    symbol->prec = OZ_intToC(OZ_getArg(triple, 2));
    assocs = OZ_getArg(assocs, 1);
  }
}

/* Parse the input grammar into a one symbol_list structure.
   Each rule is represented by a sequence of symbols: the left hand side
   followed by the contents of the right hand side, followed by a null pointer
   instead of a symbol to terminate the rule.
   The next symbol is the lhs of the following rule.
*/
static void read_rules(OZ_Term rules) {
  OZ_Term rule, rhssymbols;
  bucket *symbol;
  symbol_list *cursym = NULL;

  rline = NEW2(OZ_length(rules), OZ_Term) - 1;
  nrules = 0;
  nitems = 0;
  grammar = NULL;
  while (!OZ_eq(rules, OZ_nil())) {
    rule = OZ_getArg(rules, 0);

    nrules++;
    rline[nrules] = OZ_getArg(rule, 0);
    symbol = getsym(OZ_atomToC(OZ_getArg(rule, 1)));

    if (cursym == NULL) {
      cursym = NEW(symbol_list);
      grammar = cursym;
    } else {
      cursym->next = NEW(symbol_list);
      cursym = cursym->next;
    }
    cursym->sym = symbol;
    if (OZ_width(rule) == 4)
      cursym->ruleprec = getsym(OZ_atomToC(OZ_getArg(rule, 3)));

    rhssymbols = OZ_getArg(rule, 2);
    while (!OZ_eq(rhssymbols, OZ_nil())) {
      cursym->next = NEW(symbol_list);
      cursym = cursym->next;
      cursym->sym = getsym(OZ_atomToC(OZ_getArg(rhssymbols, 0)));
      nitems++;
      rhssymbols = OZ_getArg(rhssymbols, 1);
    }
    /* Put an empty link in the list to mark the end of this rule  */
    cursym->next = NEW(symbol_list);
    cursym = cursym->next;
    nitems++;

    rules = OZ_getArg(rules, 1);
  }

  nvars = nsyms - ntokens;
}


/* assign symbol numbers.
   Set up vectors tags and sprec of names and precedences of symbols.
*/
static void packsymbols(void) {
  register bucket *bp;

  tags = NEW2(nsyms, char *);
  sprec = NEW2(nsyms, short);
  sprec_used = NEW2(nsyms, int);
  sassoc = NEW2(nsyms, short);
  for (bp = firstsymbol; bp != NULL; bp = bp->next) {
    tags[bp->value] = bp->tag;
    sprec[bp->value] = bp->prec;
    sassoc[bp->value] = bp->assoc;
  }
}
      
/* convert the rules into the representation using rrhs, rlhs and ritems.
*/
static void packgram(void) {
  int itemno, ruleno;
  symbol_list *p, *tmp;
  bucket *ruleprec;

  ritem = NEW2(nitems + 1, short);
  rlhs = NEW2(nrules + 1, short);
  rrhs = NEW2(nrules + 1, short);
  rprec = NEW2(nrules, short) - 1;
  rassoc = NEW2(nrules, short) - 1;
  rprecsym = NEW2(nrules, short) - 1;
  rprec_used = NEW2(nrules, int) - 1;

  itemno = 0;
  ruleno = 1;

  p = grammar;
  while (p) {
    rlhs[ruleno] = p->sym->value;
    rrhs[ruleno] = itemno;
    ruleprec = p->ruleprec;
    p = p->next;
    while (p->sym) {
      ritem[itemno++] = p->sym->value;
      /* A rule gets by default the precedence and associativity
         of the last token in it.  */
      if (ISTOKEN(p->sym->value)) {
        rprec[ruleno] = p->sym->prec;
        rassoc[ruleno] = p->sym->assoc;
        rprecsym[ruleno] = p->sym->value;
      }
      tmp = p->next;
      FREE(p);
      p = tmp;
    }
    tmp = p->next;
    FREE(p);
    p = tmp;

    /* If this rule has a %prec,
       the specified symbol's precedence replaces the default.  */
    if (ruleprec) {
      rprec[ruleno] = ruleprec->prec;
      rassoc[ruleno] = ruleprec->assoc;
      rprecsym[ruleno] = ruleprec->value;
    } else
      rprec_used[ruleno] = 1;

    ritem[itemno++] = -ruleno;
    ruleno++;
  }

  ritem[itemno] = 0;
}
