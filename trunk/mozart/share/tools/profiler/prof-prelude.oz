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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

\insert prof-string
\insert prof-tk

%% some builtins...
Profile = profile(mode:    {`Builtin` setProfileMode 1}
		  getInfo: {`Builtin` statisticsGetProcs 1}
		  reset:   {`Builtin` statisticsReset 0} )

StatusHelp = {NewName}

proc {EnqueueCompilerQuery M}
   {Emacs.condSend.compiler enqueue(M)}
end

fun {UnknownFile F}
   F == nofile orelse F == ''
end

fun {StripPath File}
   case {UnknownFile File} then
      '???'
   else
      S = {Str.rchr {Atom.toString File} &/}
   in
      case {List.length S} > 1 then
	 {S2A S.2}
      else
	 '???'
      end
   end
end
