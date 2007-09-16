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

void 	Gecode::eq (Space *sp, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *sp, IntVar x, int n, IntConLevel=ICL_DEF);
void 	Gecode::eq (Space *sp, IntVar x0, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *sp, IntVar x, int n, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::eq (Space *sp, const IntVarArgs &x, IntConLevel icl=ICL_DEF);


void 	Gecode::count (Space *sp, const IntVarArgs &x, int n, IntRelType r, int m, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *sp, const IntVarArgs &x, IntVar y, IntRelType r, int m, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *sp, const IntVarArgs &x, int n, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF);
void 	Gecode::count (Space *sp, const IntVarArgs &x, IntVar y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF);
/*
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, const IntArgs &c, int m, int unspec_low, int unspec_up, int min, int max, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, const IntArgs &c, int m, int unspec, int min, int max, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, int lb, int ub, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, int ub, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, const IntVarArgs &c, int min, int max, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, const IntArgs &v, const IntVarArgs &c, int m, int unspec_low, int unspec_up, bool all, int min, int max, IntConLevel icl);
void 	Gecode::gcc (Space *sp, const IntVarArgs &x, const IntArgs &v, const IntVarArgs &c, int m, int unspec, bool all, int min, int max, IntConLevel icl);

void 	Gecode::channel (Space *sp, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF);
*/
/*
void 	Gecode::dom (Space *sp, IntVar x, int l, int m, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *sp, IntVarArgs &x, int l, int m, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *sp, IntVar x, const IntSet &s, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *sp, IntVarArgs &x, const IntSet &s, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *sp, IntVar x, int l, int m, BoolVar b, IntConLevel=ICL_DEF);
void 	Gecode::dom (Space *sp, IntVar x, const IntSet &s, BoolVar b, IntConLevel=ICL_DEF);
*/
/*
void 	Gecode::bool_not (Space *sp, BoolVar b0, BoolVar b1, IntConLevel=ICL_DEF);
void 	Gecode::bool_eq (Space *sp, BoolVar b0, BoolVar b1, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *sp, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *sp, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *sp, const BoolVarArgs &b, BoolVar c, IntConLevel=ICL_DEF);
void 	Gecode::bool_and (Space *sp, const BoolVarArgs &b, bool c, IntConLevel=ICL_DEF);
void 	Gecode::bool_or (Space *sp, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_or (Space *sp, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_or (Space *sp, const BoolVarArgs &b, BoolVar c, IntConLevel=ICL_DEF);
void 	Gecode::bool_or (Space *sp, const BoolVarArgs &b, bool c, IntConLevel=ICL_DEF);
void 	Gecode::bool_imp (Space *sp, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_imp (Space *sp, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_eqv (Space *sp, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_eqv (Space *sp, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_xor (Space *sp, BoolVar b0, BoolVar b1, BoolVar b2, IntConLevel=ICL_DEF);
void 	Gecode::bool_xor (Space *sp, BoolVar b0, BoolVar b1, bool b2, IntConLevel=ICL_DEF);
*/


void 	Gecode::sortedness (Space *sp, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF);
void 	Gecode::sortedness (Space *sp, const IntVarArgs &x, const IntVarArgs &y, const IntVarArgs &z, IntConLevel icl=ICL_DEF);

void 	Gecode::distinct (Space *sp, const IntVarArgs &x, IntConLevel icl=ICL_DEF);
void 	Gecode::distinct (Space *sp, const IntArgs &n, const IntVarArgs &x, IntConLevel icl=ICL_DEF);

//void 	Gecode::element (Space *sp, const IntArgs &n, IntVar x0, IntVar x1, IntConLevel=ICL_DEF);
//void 	Gecode::element (Space *sp, const IntVarArgs &x, IntVar y0, IntVar y1, IntConLevel icl=ICL_DEF);


void 	Gecode::rel (Space *sp, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *sp, IntVar x, IntRelType r, int c, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *sp, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *sp, IntVar x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF);
void 	Gecode::rel (Space *sp, const IntVarArgs &x, IntRelType r, const IntVarArgs &y, IntConLevel icl=ICL_DEF);

// A macro to create a DFA is not available yet.
// void 	Gecode::regular (Space *sp, const IntVarArgs &x, DFA &d, IntConLevel=ICL_DEF);

void 	Gecode::min (Space *sp, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);
void 	Gecode::min (Space *sp, const IntVarArgs &x, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::max (Space *sp, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);
void 	Gecode::max (Space *sp, const IntVarArgs &x, IntVar y, IntConLevel=ICL_DEF);
void 	Gecode::abs (Space *sp, IntVar x0, IntVar x1, IntConLevel=ICL_DEF);
void 	Gecode::mult (Space *sp, IntVar x0, IntVar x1, IntVar x2, IntConLevel=ICL_DEF);

