%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor

prepare
   
   local
      HelvFamily        = '-*-helvetica-medium-r-normal--*-'
      HelvBoldFamily    = '-*-helvetica-bold-r-normal--*-'
      CourierFamily     = '-*-courier-medium-r-normal--*-'
      CourierBoldFamily = '-*-courier-bold-r-normal--*-'
      FontMatch         = '-*-*-*-*-*-*'
      
      FontSize          = 120
      SmallSize         = 100
   in
      [HelvBold Helv Courier SmallCourierBold] =
      {Map [HelvBoldFamily    # FontSize  # FontMatch
	    HelvFamily        # FontSize  # FontMatch
	    CourierFamily     # FontSize  # FontMatch
	    CourierBoldFamily # SmallSize # FontMatch]
       VirtualString.toAtom}
   end

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

export
   MaxRes MaxJobs MaxDur MaxSpan
   ResColors
   DurUnit DurFrame JobDistance
   HelvBold Helv Courier SmallCourierBold

end
   
   