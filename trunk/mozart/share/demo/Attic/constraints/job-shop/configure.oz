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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   HelvFamily        = '-*-helvetica-medium-r-normal--*-'
   HelvBoldFamily    = '-*-helvetica-bold-r-normal--*-'
   CourierFamily     = '-*-courier-medium-r-normal--*-'
   CourierBoldFamily = '-*-courier-bold-r-normal--*-'
   FontMatch         = '-*-*-*-*-*-*'
   
   FontSize          = 120
in
   HelvBold         = HelvBoldFamily    # FontSize # FontMatch
   Helv             = HelvFamily        # FontSize # FontMatch
   Courier          = CourierFamily     # FontSize # FontMatch
   SmallCourierBold = CourierBoldFamily # 100 # FontMatch
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

