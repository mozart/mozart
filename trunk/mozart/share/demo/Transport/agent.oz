%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
   
fun {NewAgent Class Mess}
   Stream
   ThisPort   = {NewPort Stream}
in
   thread
      ThisObject = {New Class Mess}
   in
      try
	 {ForAll Stream ThisObject}
      catch system(_ debug:_) then
	 %% The toplevel widget has been closed
	 {ThisObject close}
      end
   end

   proc {$ Mess}
      {Send ThisPort Mess}
   end
end