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
   Connection
   Pickle
   
export
   Connect
   NewServer

define

   fun {NewServer C M}
      S P={Port.new S} PS
      O = {New {Class.new [C] nil f(server:PS) nil} M}
   in
      thread
	 {ForAll S proc {$ M}
		      try {O M} catch _ then skip end
		   end}
      end
      proc {PS M}
	 {Port.send P M}
      end
      PS
   end

   fun {Connect Url}
      Server = {Connection.take {Pickle.load Url}}
   in
      proc {$ M}
	 {Server M}
      end
   end


end
