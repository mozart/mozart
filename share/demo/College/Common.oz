%%%
%%% Authors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Joerg Wuertz <wuertz@de.ibm.com>
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
%%%   Joerg Wuert, 1997
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

export

   Editor
   
   Monday
   Tuesday
   Wednesday
   Thursday
   Friday
   
   QuartersPerDay
   QuartersPerHour

   DemoMode

import

   Error
   

prepare
   %% my favorite editor
   Editor          = "emacs"
   
   %% the week days
   Monday          = 1#36
   Tuesday         = 37#72
   Wednesday       = 73#108
   Thursday        = 109#144
   Friday          = 145#180
   
   QuartersPerDay  = 36
   QuartersPerHour = 4

   DemoMode = on

define

   {Error.registerFormatter college
    fun {$ E}
       T = 'error in college timetabling system'
    in
       case E
       of college(A Xs S) then
	  %% expected: A: application Xs:list, S:virtualString
	  error(kind: T
		items: ([hint(l: 'In statement' m: apply(A Xs))
			 hint(l: 'Reason      ' m: S)]))
	  
       else
	  error(kind: T
		items: [line(oz(E))])
       end
    end}
end
