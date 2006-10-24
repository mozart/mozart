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

functor
import
   Open
export
   out: HtmlOut
prepare
   fun {HtmlStd Body}
      '#'('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">\n' Body)
   end
define
   proc {HtmlOut Body}
      {New class $
	      from Open.html Open.file
	      prop final
	      meth tag(T)
		 Open.file, init(name:stdout)
		 Open.html, header
		 Open.html, tag(T)
		 Open.file, close
	      end
	   end
       tag({HtmlStd Body}) _}
   end
end


