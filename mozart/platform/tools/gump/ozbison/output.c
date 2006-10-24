/* Output the generated parsing program for bison,
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


/* functions to output parsing data to various files.  Entries are:

  output ()

Output the parsing tables to an Oz tuple with label 'ftable'.

The parser tables consist of these tables.
Starred ones needed only for the semantic parser.

yyTranslateDefault = integer which token numbers are mapped to by default

yytranslate = vector mapping yylex's token numbers into bison's token numbers.

yytname = vector of string-names indexed by bison token number

yytoknum = vector of yylex token numbers corresponding to entries in yytname

yyrline = vector of line-numbers of all rules.  For yydebug printouts.

yyrhs = vector of items of all rules.
        This is exactly what ritems contains.  For yydebug and for semantic
	parser.

yyprhs[r] = index in yyrhs of first item for rule r.

yyr1[r] = symbol number of symbol that rule r derives.

yyr2[r] = number of symbols composing right hand side of rule r.

* yystos[s] = the symbol number of the symbol that leads to state s.

yydefact[s] = default rule to reduce with in state s,
	      when yytable doesn't specify something else to do.
	      Zero means the default is an error.

yydefgoto[i] = default state to go to after a reduction of a rule that
	       generates variable ntokens + i, except when yytable
	       specifies something else to do.

yypact[s] = index in yytable of the portion describing state s.
            The lookahead token's type is used to index that portion
            to find out what to do.

	    If the value in yytable is positive,
	    we shift the token and go to that state.

	    If the value is negative, it is minus a rule number to reduce by.

	    If the value is zero, the default action from yydefact[s] is used.

yypgoto[i] = the index in yytable of the portion describing 
             what to do after reducing a rule that derives variable i + ntokens.
             This portion is indexed by the parser state number, s,
	     as of before the text for this nonterminal was read.
	     The value from yytable is the state to go to if 
             the corresponding value in yycheck is s.

yytable = a vector filled with portions for different uses,
          found via yypact and yypgoto.

yycheck = a vector indexed in parallel with yytable.
	  It indicates, in a roundabout way, the bounds of the
	  portion you are trying to examine.

	  Suppose that the portion of yytable starts at index p
	  and the index to be examined within the portion is i.
	  Then if yycheck[p+i] != i, i is outside the bounds
	  of what is actually allocated, and the default
	  (from yydefact or yydefgoto) should be used.
	  Otherwise, yytable[p+i] should be used.

yyFINAL = the state number of the termination state.
yyFLAG = most negative short int.  Used to flag ??
yyLAST = ??

*/

#include <stdio.h>
#include <mozart.h>
#include "system.h"
#include "machine.h"
#include "new.h"
#include "gram.h"
#include "state.h"


extern char **tags;
extern int tokensetsize;
extern int final_state;
extern core **state_table;
extern shifts **shift_table;
extern errs **err_table;
extern reductions **reduction_table;
extern short *accessing_symbol;
extern unsigned *LA;
extern short *LAruleno;
extern short *lookaheads;
extern char *consistent;
extern short *goto_map;
extern short *from_state;
extern short *to_state;

void output_token_translations();
void output_gram();
void output_stos();
void output_rule_data();
void output_defines();
void output_actions();
void token_actions();
void save_row();
void goto_actions();
void save_column();
void sort_actions();
void pack_table();
void output_base();
void output_table();
void output_check();
void free_itemset();
void free_shifts();
void free_reductions();
void free_itemsets();
int action_row();
int default_goto();
int matching_state();
int pack_vector();

extern void berror();
extern void done();
extern char *int_to_string();
extern void reader_output_yylsp();

static OZ_Term ftable;
static int nvectors;
static int nentries;
static short **froms;
static short **tos;
static short *tally;
static short *width;
static short *actrow;
static short *state_count;
static short *order;
static short *base;
static short *pos;
static short *table;
static short *check;
static int lowzero;
static int high;



OZ_Term
output()
{
  ftable = OZ_nil();

  free_itemsets();
  output_defines();
  /* This is now unconditional because debugging printouts can use it.  */
  output_gram();
  output_stos();
  output_rule_data();
  output_token_translations();
  output_actions();

  /* Warnings */
  if (src_total > 0 || rrc_total > 0) {
    ftable = OZ_cons(OZ_pairA("conflicts",
			      OZ_pair2(OZ_int(src_total),
				       OZ_int(rrc_total))), ftable);
  }
  if (nuseless_productions > 0 || nuseless_nonterminals > 0) {
    ftable = OZ_cons(OZ_pairA("useless",
			      OZ_pair2(OZ_int(nuseless_nonterminals),
				       OZ_int(nuseless_productions))), ftable);
  }

  return OZ_recordInit(OZ_atom("ozbisonTables"), ftable);
}


static void toftable(char *atom, OZ_Term value) {
  ftable = OZ_cons(OZ_pair2(OZ_atom(atom), value), ftable);
}

static void makeRecord(char *label, OZ_Term *table, int low, int high) {
  OZ_Term propList;
  int n = high - low, i;

  propList = OZ_nil();
  for (i = n; i >= 0; i--)
    propList = OZ_cons(OZ_pair2(OZ_int(i + low), table[i]), propList);

  toftable(label, OZ_recordInit(OZ_atom(label), propList));
}

static void makeShortRecord(char *label, short *table, int low, int high) {
  OZ_Term propList;
  int n = high - low, i;

  propList = OZ_nil();
  for (i = n; i >= 0; i--)
    propList = OZ_cons(OZ_pair2(OZ_int(i + low), OZ_int(table[i])), propList);

  toftable(label, OZ_recordInit(OZ_atom(label), propList));
}

static void makeIntRecord(char *label, int *table, int low, int high) {
  OZ_Term propList;
  int n = high - low, i;

  propList = OZ_nil();
  for (i = n; i >= 0; i--)
    propList = OZ_cons(OZ_pair2(OZ_int(i + low), OZ_int(table[i])), propList);

  toftable(label, OZ_recordInit(OZ_atom(label), propList));
}

static void makeStringRecord(char *label, char **table, int low, int high) {
  OZ_Term propList;
  int n = high - low, i;

  propList = OZ_nil();
  for (i = n; i >= 0; i--)
    propList = OZ_cons(OZ_pair2(OZ_int(i + low), OZ_atom(table[i])), propList);

  toftable(label, OZ_recordInit(OZ_atom(label), propList));
}


static int convertHex(int c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  else if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  else
    return c - 'A' + 10;
}

/* return values:
   0: is atom token
   1--255: is single character literal token i
   -1: not an atom token
*/
static int convertAtom(char *s) {
  int r = 0, w = 0;
  if (s[r] == '\'') {
    r++;
    while (s[r] != '\'')
      if (s[r] == '\\') {
	r++;
	switch (s[r++]) {
	case 'a': s[w++] = '\a'; break;
	case 'b': s[w++] = '\b'; break;
	case 'f': s[w++] = '\f'; break;
	case 'n': s[w++] = '\n'; break;
	case 'r': s[w++] = '\r'; break;
	case 't': s[w++] = '\t'; break;
	case 'v': s[w++] = '\v'; break;
	case 'x': s[w++] = convertHex(s[r]) * 16 + convertHex(s[r + 1]);
	  r += 2; break;
	case '0': case '1': case '2': case '3':
	case '4': case '5': case '6': case '7':
	  s[w++] = (s[r - 1] - '0') * 64 + (s[r] - '0') * 8 + (s[r + 1] - '0');
	  r += 2; break;
	default:
	  s[w++] = s[r - 1]; break;
	}
      } else
	s[w++] = s[r++];
    s[w] = '\0';
    if (w == 1)
      return s[0];
    else
      return 0;
  } else if (s[r] >= 'a' && s[r] <= 'z')
    return 0;
  else
    return -1;
}

void
output_token_translations()
{
  OZ_Term propList;
  int i, j;

  propList = OZ_nil();
  for (i = ntokens - 1; i >= 0; i--) {
    j = convertAtom(tags[i]);
    if (j >= 0) {
      propList = OZ_cons(OZ_pair2(OZ_atom(tags[i]), OZ_int(i)), propList);
      if (j > 0)
	propList = OZ_cons(OZ_pair2(OZ_int(j), OZ_int(i)), propList);
    }
  }

  toftable("synTranslate", OZ_recordInit(OZ_atom("synTranslate"), propList));
  makeStringRecord("synTokenNames", tags, 0, nsyms - 1);
  FREE(tags);
}


void
output_gram()
{
  register int i;

  makeShortRecord("synPRhs", rrhs, 0, nrules);

  for (i = 1; ritem[i] != 0; i++)
    if (ritem[i] < 0)
      ritem[i] = 0;
  makeShortRecord("synRhs", ritem, 0, i - 1);

  FREE(ritem);
}


void
output_stos()
{
  short tmp = accessing_symbol[0];
  accessing_symbol[0] = 0;
  makeShortRecord("synStoS", accessing_symbol, 0, nstates - 1);
  accessing_symbol[0] = tmp;
}


void
output_rule_data()
{
  register int i;

  makeRecord("synRulePosition", rline + 1, 1, nrules);
  FREE(rline + 1);
  toftable("synNTOKENS", OZ_int(ntokens));
  toftable("synNNTS", OZ_int(nvars));
  toftable("synNRULES", OZ_int(nrules));
  toftable("synNSTATES", OZ_int(nstates));

  rlhs[0] = 0;
  makeShortRecord("synR1", rlhs, 0, nrules);
  FREE(rlhs);

  rrhs[0] = 0;
  for (i = 1; i < nrules; i++)
    rrhs[i] = rrhs[i + 1] - rrhs[i] - 1;
  rrhs[nrules] = nitems - rrhs[nrules] - 1;
  makeShortRecord("synR2", rrhs, 0, nrules);
  FREE(rrhs);
}


void
output_defines()
{
  toftable("synFINAL", OZ_int(final_state));
  toftable("synFLAG", OZ_int(MINSHORT));
}



/* compute and output yydefact, yydefgoto, yypact, yypgoto, yytable and yycheck.  */

void
output_actions()
{
  nvectors = nstates + nvars;

  froms = NEW2(nvectors, short *);
  tos = NEW2(nvectors, short *);
  tally = NEW2(nvectors, short);
  width = NEW2(nvectors, short);

  token_actions();
  free_shifts();
  free_reductions();
  FREE(lookaheads);
  FREE(LA);
  FREE(LAruleno);
  FREE(accessing_symbol);

  goto_actions();
  FREE(goto_map + ntokens);
  FREE(from_state);
  FREE(to_state);

  sort_actions();
  pack_table();
  output_base();
  output_table();
  output_check();
}



/* figure out the actions for the specified state, indexed by lookahead token type.

   The yydefact table is output now.  The detailed info
   is saved for putting into yytable later.  */

void
token_actions()
{
  register int i;
  int *defact;

  actrow = NEW2(ntokens, short);
  defact = NEW2(nstates, int);
  for (i = 0; i < nstates; i++)
    {
      defact[i] = action_row(i);
      save_row(i);
    }
  makeIntRecord("synDefAct", defact, 0, nstates - 1);

  FREE(defact);
  FREE(actrow);
  FREE(consistent);
  for (i = 0; i < nstates; i++)
    {
      if (err_table[i])
	{
	  FREE(err_table[i]);
	}
    }
  FREE(err_table);
}



/* Decide what to do for each type of token if seen as the lookahead token in specified state.
   The value returned is used as the default action (yydefact) for the state.
   In addition, actrow is filled with what to do for each kind of token,
   index by symbol number, with zero meaning do the default action.
   The value MINSHORT, a very negative number, means this situation
   is an error.  The parser recognizes this value specially.

   This is where conflicts are resolved.  The loop over lookahead rules
   considered lower-numbered rules last, and the last rule considered that likes
   a token gets to handle it.  */

int
action_row(state)
int state;
{
  register int i;
  register int j;
  register int k;
  register int m;
  register int n;
  register int count;
  register int default_rule;
  register int nreds;
  register int max;
  register int rule;
  register int shift_state;
  register int symbol;
  register unsigned mask;
  register unsigned *wordp;
  register reductions *redp;
  register shifts *shiftp;
  register errs *errp;
  int nodefault = 0;  /* set nonzero to inhibit having any default reduction */

  for (i = 0; i < ntokens; i++)
    actrow[i] = 0;

  default_rule = 0;
  nreds = 0;
  redp = reduction_table[state];

  if (redp)
    {
      nreds = redp->nreds;

      if (nreds >= 1)
	{
	  /* loop over all the rules available here which require lookahead */
	  m = lookaheads[state];
	  n = lookaheads[state + 1];

	  for (i = n - 1; i >= m; i--)
	    {
	      rule = - LAruleno[i];
	      wordp = LA + i * tokensetsize;
	      mask = 1;

	      /* and find each token which the rule finds acceptable to come next */
	      for (j = 0; j < ntokens; j++)
		{
		  /* and record this rule as the rule to use if that token follows.  */
		  if (mask & *wordp)
		    actrow[j] = rule;

		  mask <<= 1;
		  if (mask == 0)
		    {
		      mask = 1;
		      wordp++;
		    }
		}
	    }
	}
    }

  shiftp = shift_table[state];

  /* now see which tokens are allowed for shifts in this state.
     For them, record the shift as the thing to do.  So shift is preferred to reduce.  */

  if (shiftp)
    {
      k = shiftp->nshifts;

      for (i = 0; i < k; i++)
	{
	  shift_state = shiftp->shifts[i];
	  if (! shift_state) continue;

	  symbol = accessing_symbol[shift_state];

	  if (ISVAR(symbol))
	    break;

	  actrow[symbol] = shift_state;

	  /* do not use any default reduction if there is a shift for error */

	  if (symbol == error_token_number) nodefault = 1;
	}
    }

  errp = err_table[state];

  /* See which tokens are an explicit error in this state
     (due to %nonassoc).  For them, record MINSHORT as the action.  */

  if (errp)
    {
      k = errp->nerrs;

      for (i = 0; i < k; i++)
	{
	  symbol = errp->errs[i];
	  actrow[symbol] = MINSHORT;
	}
    }

  /* now find the most common reduction and make it the default action for this state.  */

  if (nreds >= 1 && ! nodefault)
    {
      if (consistent[state])
	default_rule = redp->rules[0];
      else
	{
	  max = 0;
	  for (i = m; i < n; i++)
	    {
	      count = 0;
	      rule = - LAruleno[i];
    
	      for (j = 0; j < ntokens; j++)
		{
		  if (actrow[j] == rule)
		    count++;
		}
    
	      if (count > max)
		{
		  max = count;
		  default_rule = rule;
		}
	    }
    
	  /* actions which match the default are replaced with zero,
	     which means "use the default" */
    
	  if (max > 0)
	    {
	      for (j = 0; j < ntokens; j++)
		{
		  if (actrow[j] == default_rule)
		    actrow[j] = 0;
		}
    
	      default_rule = - default_rule;
	    }
	}
    }

  /* If have no default rule, the default is an error.
     So replace any action which says "error" with "use default".  */

  if (default_rule == 0)
    for (j = 0; j < ntokens; j++)
      {
	if (actrow[j] == MINSHORT)
	  actrow[j] = 0;
      }

  return (default_rule);
}


void
save_row(state)
int state;
{
  register int i;
  register int count;
  register short *sp;
  register short *sp1;
  register short *sp2;

  count = 0;
  for (i = 0; i < ntokens; i++)
    {
      if (actrow[i] != 0)
	count++;
    }

  if (count == 0)
    return;

  froms[state] = sp1 = sp = NEW2(count, short);
  tos[state] = sp2 = NEW2(count, short);

  for (i = 0; i < ntokens; i++)
    {
      if (actrow[i] != 0)
	{
	  *sp1++ = i;
	  *sp2++ = actrow[i];
	}
    }

  tally[state] = count;
  width[state] = sp1[-1] - sp[0] + 1;
}



/* figure out what to do after reducing with each rule,
   depending on the saved state from before the beginning
   of parsing the data that matched this rule.

   The yydefgoto table is output now.  The detailed info
   is saved for putting into yytable later.  */

void
goto_actions()
{
  register int i;
  register int k;
  int *defgoto;

  state_count = NEW2(nstates, short);
  defgoto = NEW2(nsyms - ntokens, int);

  for (i = ntokens; i < nsyms; i++)
    {
      k = default_goto(i);
      defgoto[i - ntokens] = k;
      save_column(i, k);
    }
  makeIntRecord("synDefGoto", defgoto, ntokens, nsyms - 1);

  FREE(defgoto);
  FREE(state_count);
}



int
default_goto(symbol)
int symbol;
{
  register int i;
  register int m;
  register int n;
  register int default_state;
  register int max;

  m = goto_map[symbol];
  n = goto_map[symbol + 1];

  if (m == n)
    return (-1);

  for (i = 0; i < nstates; i++)
    state_count[i] = 0;

  for (i = m; i < n; i++)
    state_count[to_state[i]]++;

  max = 0;
  default_state = -1;

  for (i = 0; i < nstates; i++)
    {
      if (state_count[i] > max)
	{
	  max = state_count[i];
	  default_state = i;
	}
    }

  return (default_state);
}


void
save_column(symbol, default_state)
int symbol;
int default_state;
{
  register int i;
  register int m;
  register int n;
  register short *sp;
  register short *sp1;
  register short *sp2;
  register int count;
  register int symno;

  m = goto_map[symbol];
  n = goto_map[symbol + 1];

  count = 0;
  for (i = m; i < n; i++)
    {
      if (to_state[i] != default_state)
	count++;
    }

  if (count == 0)
    return;

  symno = symbol - ntokens + nstates;

  froms[symno] = sp1 = sp = NEW2(count, short);
  tos[symno] = sp2 = NEW2(count, short);

  for (i = m; i < n; i++)
    {
      if (to_state[i] != default_state)
	{
	  *sp1++ = from_state[i];
	  *sp2++ = to_state[i];
	}
    }

  tally[symno] = count;
  width[symno] = sp1[-1] - sp[0] + 1;
}



/* the next few functions decide how to pack 
   the actions and gotos information into yytable. */

void
sort_actions()
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int w;

  order = NEW2(nvectors, short);
  nentries = 0;

  for (i = 0; i < nvectors; i++)
    {
      if (tally[i] > 0)
	{
	  t = tally[i];
	  w = width[i];
	  j = nentries - 1;

	  while (j >= 0 && (width[order[j]] < w))
	    j--;

	  while (j >= 0 && (width[order[j]] == w) && (tally[order[j]] < t))
	    j--;

	  for (k = nentries - 1; k > j; k--)
	    order[k + 1] = order[k];

	  order[j + 1] = i;
	  nentries++;
	}
    }
}


void
pack_table()
{
  register int i;
  register int place;
  register int state;

  base = NEW2(nvectors, short);
  pos = NEW2(nentries, short);
  table = NEW2(MAXTABLE, short);
  check = NEW2(MAXTABLE, short);

  lowzero = 0;
  high = 0;

  for (i = 0; i < nvectors; i++)
    base[i] = MINSHORT;

  for (i = 0; i < MAXTABLE; i++)
    check[i] = -1;

  for (i = 0; i < nentries; i++)
    {
      state = matching_state(i);

      if (state < 0)
	place = pack_vector(i);
      else
	place = base[state];

      pos[i] = place;
      base[order[i]] = place;
    }

  for (i = 0; i < nvectors; i++)
    {
      if (froms[i])
	FREE(froms[i]);
      if (tos[i])
	FREE(tos[i]);
    }

  FREE(froms);
  FREE(tos);
  FREE(tally);
  FREE(width);
  FREE(pos);
  FREE(order);
}



int
matching_state(vector)
int vector;
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int w;
  register int match;
  register int prev;

  i = order[vector];
  if (i >= nstates)
    return (-1);

  t = tally[i];
  w = width[i];

  for (prev = vector - 1; prev >= 0; prev--)
    {
      j = order[prev];
      if (width[j] != w || tally[j] != t)
	return (-1);

      match = 1;
      for (k = 0; match && k < t; k++)
	{
	  if (tos[j][k] != tos[i][k] || froms[j][k] != froms[i][k])
	    match = 0;
	}

      if (match)
	return (j);
    }

  return (-1);
}



int
pack_vector(vector)
int vector;
{
  register int i;
  register int j;
  register int k;
  register int t;
  register int loc;
  register int ok;
  register short *from;
  register short *to;

  i = order[vector];
  t = tally[i];

  if (t == 0)
    berror("pack_vector");

  from = froms[i];
  to = tos[i];

  for (j = lowzero - from[0]; j < MAXTABLE; j++)
    {
      ok = 1;

      for (k = 0; ok && k < t; k++)
	{
	  loc = j + from[k];
	  if (loc > MAXTABLE)
	    {
	      done(OZ_pair2(OZ_pairAI("maximum table size (",MAXTABLE),
			    OZ_atom(") exceeded")));
	    }

	  if (table[loc] != 0)
	    ok = 0;
	}

      for (k = 0; ok && k < vector; k++)
	{
	  if (pos[k] == j)
	    ok = 0;
	}

      if (ok)
	{
	  for (k = 0; k < t; k++)
	    {
	      loc = j + from[k];
	      table[loc] = to[k];
	      check[loc] = from[k];
	    }

	  while (table[lowzero] != 0)
	    lowzero++;

	  if (loc > high)
	    high = loc;

	  return (j);
	}
    }

  berror("pack_vector");
  return 0;	/* JF keep lint happy */
}



/* the following functions output yytable, yycheck
   and the vectors whose elements index the portion starts */

void
output_base()
{
  makeShortRecord("synPAct", base, 0, nstates - 1);
  makeShortRecord("synPGoto", base + nstates, ntokens, nvectors - nstates - 1 + ntokens);

  FREE(base);
}


void
output_table()
{
  toftable("synLAST", OZ_int(high));
  makeShortRecord("synTable", table, 0, high);
  FREE(table);
}


void
output_check()
{
  makeShortRecord("synCheck", check, 0, high);
  FREE(check);
}


void
free_itemsets()
{
  register core *cp,*cptmp;

  FREE(state_table);

  for (cp = first_state; cp; cp = cptmp) {
    cptmp=cp->next;
    FREE(cp);
  }
}


void
free_shifts()
{
  register shifts *sp,*sptmp;/* JF derefrenced freed ptr */

  FREE(shift_table);

  for (sp = first_shift; sp; sp = sptmp) {
    sptmp=sp->next;
    FREE(sp);
  }
}


void
free_reductions()
{
  register reductions *rp,*rptmp;/* JF fixed freed ptr */

  FREE(reduction_table);

  for (rp = first_reduction; rp; rp = rptmp) {
    rptmp=rp->next;
    FREE(rp);
  }
}
