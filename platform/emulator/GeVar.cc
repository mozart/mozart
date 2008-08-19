/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
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

 
#include "GeVar.hh"

void GeVar::test(){
  printf("test\n");fflush(stdout);
}
  
#include <sstream>

//template <class VarImp, Gecode::PropCond pc>
void GeVar::printStreamV(ostream &out,int depth) {
#ifdef DEBUG_CHECK
  printf("GeVar->printStreamV Warning ensuring domReflector disabled.\n");
  fflush(stdout);
  toStream(out);
#else
  ensureDomReflection();
  toStream(out);
#endif
}
