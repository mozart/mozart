%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997, 1998
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

functor

prepare
   MaxRes    = 10
   MaxJobs   = 10
   MaxDur    = 10
   MaxSpan   = 1000
   ResColors = ('deep sky blue' # green3 # 
		tomato2 # firebrick4 # wheat3 # 
		palegoldenrod # pink2 # cyan4 # darkorange # darkorchid)
   
   DurUnit     = 16
   DurFrame    = 4
   JobDistance = DurUnit + 4

import
   Tk(font)
   
define

   Text      = {New Tk.font tkInit(family:helvetica size:~12)}
   TextBold  = {New Tk.font tkInit(family:helvetica size:~12 weight:bold)}
   Type      = {New Tk.font tkInit(family:courier   size:~12)}
   TypeSmall = {New Tk.font tkInit(family:courier   size:~10 weight:bold)}

export
   MaxRes MaxJobs MaxDur MaxSpan
   ResColors
   DurUnit DurFrame JobDistance
   Text TextBold Type TypeSmall

end
   
   
