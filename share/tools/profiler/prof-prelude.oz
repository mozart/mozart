%%%
%%% Authors:
%%%   Author's name (Author's email address)
%%%
%%% Contributors:
%%%   optional, Contributor's name (Contributor's email address)
%%%
%%% Copyright:
%%%   Organization or Person (Year(s))
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
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

\insert prof-string
\insert prof-tk

%% some builtins...
Profile = profile(mode:    {`Builtin` setProfileMode 1}
		  getInfo: {`Builtin` statisticsGetProcs 1}
		  reset:   {`Builtin` statisticsReset 0} )

%% send a warning/error message
proc {ProfilerMessage M}
   %% disabled...
   %{System.showInfo {ProfilerMessagePrefix} # M}
   skip
end
proc {ProfilerError M}
   {System.showInfo ProfilerErrorPrefix # M}
end

StatusHelp = {NewName}

proc {Compile VS}
   case {Compiler.getOPICompiler} of false then
      skip
   elseof CompilerObject then
      {CompilerObject feedVirtualString(VS)}
   end
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

