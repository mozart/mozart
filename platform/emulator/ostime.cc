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

#include "base.hh"
#include "value.hh"

#include <time.h>

static OZ_Term make_time(const struct tm* tim) {
  OZ_MAKE_RECORD_S("time",9,
		   {"hour" OZ_COMMA 
		      "isDst" OZ_COMMA 
		      "mDay" OZ_COMMA 
		      "min" OZ_COMMA 
		      "mon" OZ_COMMA 
		      "sec" OZ_COMMA 
		      "wDay" OZ_COMMA 
		      "yDay" OZ_COMMA 
		      "year"},
		   { oz_int(tim->tm_hour) OZ_COMMA 
		       oz_int(tim->tm_isdst) OZ_COMMA 
		       oz_int(tim->tm_mday) OZ_COMMA 
		       oz_int(tim->tm_min) OZ_COMMA 
		       oz_int(tim->tm_mon) OZ_COMMA 
		       oz_int(tim->tm_sec) OZ_COMMA 
		       oz_int(tim->tm_wday) OZ_COMMA 
		       oz_int(tim->tm_yday) OZ_COMMA 
		       oz_int(tim->tm_year) },r);

  return r;
}

OZ_BI_define(ostime_time, 0,1) {
  OZ_RETURN_LONG(time(0));
} OZ_BI_end

OZ_BI_define(ostime_gmtime,1,1) {
  OZ_declareLong(0,t);
  time_t timebuf = (time_t) t;

  OZ_RETURN(make_time(gmtime(&timebuf)));
} OZ_BI_end

OZ_BI_define(ostime_localtime,1,1) {
  OZ_declareLong(0,t);
  time_t timebuf = (time_t) t;

  OZ_RETURN(make_time(localtime(&timebuf)));
} OZ_BI_end


/*
 * The builtin table
 */

#ifndef MODULES_LINK_STATIC

#include "modOsTime-if.cc"

#endif
