/*
%%%
%%% Authors:
%%%   Lars Rasmusson (Lars.Rasmusson@sics.se)
%%%
%%% Copyright:
%%%   Lars Rasmusson, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
*/

#ifndef __PSQLLIB_H
#define __PSQLLIB_H

#include "mozart.h"

// DEBUG macro

#ifdef DEBUG_PSQLLIB
#define DEBUG(X) X
#else
#define DEBUG(X)
#endif

int osGetAlarmTimerInterval();
void osSetAlarmTimer(int t);


#endif // __PSQLLIB__
