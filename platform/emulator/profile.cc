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
#include "builtins.hh"

OZ_BI_define(BIstatisticsReset, 0,0)
{
  ozstat.initCount();
  return PROCEED;
} OZ_BI_end

OZ_BI_define(BIstatisticsGetProcs, 0,1)
{
  OZ_RETURN(PrTabEntry::getProfileStats());
} OZ_BI_end

OZ_BI_define(BIsetProfileMode, 1,0)
{
  oz_declareIN(0,onoff);
  if (oz_isTrue(oz_deref(onoff))) {
    am.setProfileMode();
  } else {
    am.unsetProfileMode();
  }
  return PROCEED;
} OZ_BI_end


#ifndef MODULES_LINK_STATIC

#include "modProfile-if.cc"

#endif
