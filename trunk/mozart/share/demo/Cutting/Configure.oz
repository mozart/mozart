%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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

import
   Tk(font)
   
export
   fonts:  Fonts
   colors: Colors
   delays: Delays
   
prepare

   Colors = colors(glass:   steelblue1
		   cut:     firebrick
		   sketch:  c(202 225 255)
		   entry:   wheat
		   bad:     c(238 44 44)
		   okay:    c(255 127 0)
		   good:    c(180 238 180)
		   neutral: ivory
		   bg:      ivory)

   Delays = delays(cut:  400
		   wait: 1200)

define

   Fonts = fonts(normal:
		    {New Tk.font tkInit(family:helvetica size:~12)} 
		 bold:
		    {New Tk.font tkInit(family:helvetica size:~12
					weight:bold)} )

end
