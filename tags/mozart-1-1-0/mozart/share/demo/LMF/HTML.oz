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
   Open

export
   out: HtmlOut
   
prepare
   
   fun {HtmlStd Title Body}
      '#'('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2//EN">'
	  html(head(title(Title))
	       'body'(h1(Title)
		      br
		      Body)))
   end


define
   
   proc {HtmlOut Title Body}
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
       tag({HtmlStd Title Body}) _}
   end

end
