%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

\insert prof-string
\insert prof-tk

StatusHelp = {NewName}

proc {EnqueueCompilerQuery M}
   {Emacs.condSend.compiler enqueue(M)}
end

fun {UnknownFile F}
   F == ''
end

fun {StripPath File}
   if {UnknownFile File} then
      '???'
   else
      S = {Str.rchr {Atom.toString File} &/}
   in
      if {List.length S} > 1 then
	 {S2A S.2}
      else
	 '???'
      end
   end
end

proc {SendEmacs M}
   if {Cget emacs} then
      case {Cget emacsInterface} of false then skip
      elseof I then {I M}
      end
   end
end
