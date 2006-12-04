/*
 *  Authors:
 *    Michael Mehl (mehl@dfki.de)
 *    Kostja Popow (popow@ps.uni-sb.de)
 *    Ralf Scheidhauer (Ralf.Scheidhauer@ps.uni-sb.de)
 *    Christian Schulte <schulte@ps.uni-sb.de>
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *    Denys Duchier (duchier@ps.uni-sb.de)
 *    Leif Kornstaedt (kornstae@ps.uni-sb.de)
 * 
 *  Copyright:
 *    Michael Mehl, 1997,1998
 *    Kostja Popow, 1997
 *    Ralf Scheidhauer, 1997
 *    Christian Schulte, 1997
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

#include "base.hh"
#include "am.hh"
#include "builtins.hh"
#include "os.hh"
#include "var_base.hh"

OZ_BI_define(BIgarbageCollection,0,0)
{
  am.setSFlag(StartGC);

  return BI_PREEMPT;
} OZ_BI_end

OZ_BI_define(BIsystemEq,2,1) {
  oz_declareSafeDerefIN(0,a);
  oz_declareSafeDerefIN(1,b);
  OZ_RETURN(oz_bool(oz_eq(a,b)));
} OZ_BI_end


int oz_var_getSuspListLength(OzVariable *cv);

OZ_BI_define(BIconstraints,1,1)
{
  oz_declareDerefIN(0,in);

  int len = 0;
  Assert(!oz_isRef(in));
  if (oz_isVar(in)) {
    len=oz_var_getSuspListLength(tagged2Var(in));
  }
  OZ_RETURN_INT(len);
} OZ_BI_end


/* ---------------------------------------------------------------------
 * System
 * --------------------------------------------------------------------- */

static
OZ_Return printVS(char*s,int n, int fd, Bool newline)
{
  char c = '\n';
  if ((ossafewrite(fd,s,n) < 0) ||
      (newline && (ossafewrite(fd,&c,1) < 0))) {
    if (isDeadSTDOUT())
      //am.exitOz(1);
      return PROCEED;
    else
      return oz_raise(E_ERROR,E_KERNEL,"writeFailed",1,OZ_string(OZ_unixError(ossockerrno())));
  }
  return PROCEED;
}

OZ_BI_define(BIprintInfo,1,0)
{
  OZ_declareVS(0,s,n);
  return printVS(s,n,STDOUT_FILENO,NO);
} OZ_BI_end


OZ_BI_define(BIshowInfo,1,0)
{
  OZ_declareVS(0,s,n);
  return printVS(s,n,STDOUT_FILENO,OK);
} OZ_BI_end

OZ_BI_define(BIprintError,1,0)
{
  OZ_declareVS(0,s,n);
  prefixError(); // print popup code for opi
  return printVS(s,n,STDERR_FILENO,NO);
} OZ_BI_end

OZ_BI_define(BIshowError,1,0)
{
  OZ_declareVS(0,s,n);
  prefixError(); // print popup code for opi
  return printVS(s,n,STDERR_FILENO,OK);
} OZ_BI_end

OZ_Return printInline(TaggedRef term, Bool newline = NO)
{
  int len;
  //printf("system.cc printInline %d\n",term);fflush(stdout);
  char *s = OZ__toC(term,ozconf.printDepth,ozconf.printWidth,&len);
  return printVS(s,len,STDOUT_FILENO,newline);
}

OZ_DECLAREBI_USEINLINEREL1(BIprint,printInline)


OZ_Return showInline(TaggedRef term)
{
  return printInline(term,OK);
}

OZ_DECLAREBI_USEINLINEREL1(BIshow,showInline)

OZ_BI_define(BIgetPrintName,1,1) {
  OZ_RETURN(oz_getPrintName(OZ_in(0)));
} OZ_BI_end

// ---------------------------------------------------------------------------

OZ_BI_define(BIonToplevel,0,1)
{

  OZ_RETURN(oz_bool(OZ_onToplevel()));
} OZ_BI_end


//#ifndef MODULES_LINK_STATIC
//
//#include "modSystem-if.cc"
//
//#endif
