/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alejandro Arbelaez, 2007
 *    Gustavo Gutierrez, 2007
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

/*
	This file contains the rototypes of the gecode porpagators post 
	functions.
*/
 
void 	Gecode::linear (Space *sp, const IntVarArgs &x, IntRelType r, int c, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntVarArgs &x, IntRelType r, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntVarArgs &x, IntRelType r, int c, BoolVar b, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntVarArgs &x, IntRelType r, IntVar y, BoolVar b, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntArgs &a, const IntVarArgs &x, IntRelType r, int c, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntArgs &a, const IntVarArgs &x, IntRelType r, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntArgs &a, const IntVarArgs &x, IntRelType r, int c, BoolVar b, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const IntArgs &a, const IntVarArgs &x, IntRelType r, IntVar y, BoolVar b, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const BoolVarArgs &x, IntRelType r, int c, IntConLevel=ICL_DEF);
void 	Gecode::linear (Space *sp, const BoolVarArgs &x, IntRelType r, IntVar y, IntConLevel=ICL_DEF);

/*
void 	Gecode::cumulatives (Space *sp, const IntVarArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntVarArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntVarArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntVarArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
void 	Gecode::cumulatives (Space *sp, const IntArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF);
*/

void 	Gecode::eq (Space *home, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *home, IntVar x, int n, IntConLevel=ICL_DEF);
void 	Gecode::eq (Space *home, IntVar x0, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *home, IntVar x, int n, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *home, const IntVarArgs &x, IntConLevel icl=ICL_DEF);


void 	Gecode::count (Space *home, const IntVarArgs &x, int n, IntRelType r, int m, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, int m, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, int n, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF);


void 	Gecode::gcc (Space *home, const IntVarArgs &x, const IntArgs &c, intS m, intS unspec_low, intS unspec_up, intS min, intS max, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, const IntArgs &c, intS m, intS unspec, intS min, intS max, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, intS lb, intS ub, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, intS ub, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, const IntVarArgs &c, intS min, intS max, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, const IntArgs &v, const IntVarArgs &c, intS m, intS unspec_low, intS unspec_up, boolS all, intS min, intS max, IntConLevel icl);
void 	Gecode::gcc (Space *home, const IntVarArgs &x, const IntArgs &v, const IntVarArgs &c, intS m, intS unspec, boolS all, intS min, intS max, IntConLevel icl);

//void 	Gecode::channel (Space *home, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF);


void 	Gecode::dom (Space *home, IntVar x, intS l, intS m, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *home, IntVarArgs &x, intS l, intS m, IntConLevel=ICL_DEF);
//void 	Gecode::dom (Space *home, IntVar x, const IntSet &s, IntConLevel=ICL_DEF);
//void 	Gecode::dom (Space *home, IntVarArgs &x, const IntSet &s, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *home, IntVar x, intS l, intS m, BoolVar b, IntConLevel=ICL_DEF);
//void 	Gecode::dom (Space *home, IntVar x, const IntSet &s, BoolVar b, IntConLevel=ICL_DEF);


void 	Gecode::bool_not (Space *home, BoolVar b0, BoolVar b1, IntConLevel=ICL_DEF);
void 	Gecode::bool_eq (Space *home, BoolVar b0, BoolVar b1, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *home, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
//void 	Gecode::bool_and (Space *home, const BoolVarArgs &b, BoolVar c, IntConLevel=ICL_DEF);
//void 	Gecode::bool_and (Space *home, const BoolVarArgs &b, bool c, IntConLevel=ICL_DEF);

void 	Gecode::bool_or (Space *home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_or (Space *home, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
//void 	Gecode::bool_or (Space *home, const BoolVarArgs &b, BoolVar c, IntConLevel=ICL_DEF);
//void 	Gecode::bool_or (Space *home, const BoolVarArgs &b, bool c, IntConLevel=ICL_DEF);
void 	Gecode::bool_imp (Space *home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_imp (Space *home, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_eqv (Space *home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_eqv (Space *home, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_xor (Space *home, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_xor (Space *home, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);



void 	Gecode::sortedness (Space *home, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF);
void 	Gecode::sortedness (Space *home, const IntVarArgs &x, const IntVarArgs &y, const IntVarArgs &z, IntConLevel icl=ICL_DEF);

void 	Gecode::distinct (Space *home, const IntVarArgs &x, IntConLevel icl=ICL_DEF);
void 	Gecode::distinct (Space *home, const IntArgs &n, const IntVarArgs &x, IntConLevel icl=ICL_DEF);

//void 	Gecode::element (Space *home, const IntArgs &n, IntVar x0, IntVar x1, IntConLevel=ICL_DEF);
//void 	Gecode::element (Space *home, const IntVarArgs &x, IntVar y0, IntVar y1, IntConLevel icl=ICL_DEF);


void 	Gecode::rel (Space *home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *home, IntVar x, IntRelType r, int c, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *home, IntVar x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *home, const IntVarArgs &x, IntRelType r, const IntVarArgs &y, IntConLevel icl=ICL_DEF);

// A macro to create a DFA is not available yet.
// void 	Gecode::regular (Space *home, const IntVarArgs &x, DFA &d, IntConLevel=ICL_DEF);

void 	Gecode::min (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);
void 	Gecode::min (Space *home, const IntVarArgs &x, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::max (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);
void 	Gecode::max (Space *home, const IntVarArgs &x, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::abs (Space *home, IntVar x0, IntVar x1, IntConLevel=ICL_DEF);
void 	Gecode::mult (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);

