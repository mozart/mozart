/*
 *  Authors:
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
 * 
 *  Last change:
 *    $_Date$ by $_Author$
 *    $_Revision$
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

#include "value.hh"

//
// kost@ : 'ope' is the original PrTabEntry.
// kost@ : 'pe' is the PrTabEntry for the copy.
//         'start' points to the first procedure's instruction;
ProgramCounter copyCode(PrTabEntry *ope, PrTabEntry *pe,
			ProgramCounter start, TaggedRef alist);
