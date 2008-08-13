/*
 *  Authors:
 *    Christian Schulte <schulte@ps.uni-sb.de>
 * 
 *  Copyright:
 *    Christian Schulte, 1999
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

#if defined(INTERFACE)
#pragma implementation "distributor.hh"
#endif

#include "distributor.hh"

int Distributor::getAlternatives() {
	printf("Called abstract class getAlternatives\n");fflush(stdout);
	return 0;
}

int Distributor::commit(Board *, int ) {
	printf("Called abstract class commit\n");fflush(stdout);
	return 0;
}

int Distributor::commit(Board *, int, int r) {
  return (r > 2) ? -2 : 2;
}

int Distributor::notifyStable(Board *) {
  return -1;
}

int Distributor::commitBranch(Board *, TaggedRef) {
  return 0;
}

OZ_Return Distributor::tell(RefsArray *x) {
  Assert(false);
}
