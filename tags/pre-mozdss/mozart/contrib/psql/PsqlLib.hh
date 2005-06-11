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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
extern "C" void ozpwarning(const char*);


#endif // __PSQLLIB__

