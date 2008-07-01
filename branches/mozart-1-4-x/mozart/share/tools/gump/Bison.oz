%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1996-1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% Interface to the Bison parse table generator
%%

local
   MAXSHORT = 32767
in
   fun {Bison NSymbols Grammar VerboseFile Rep}
      if Grammar.4 == nil then   % no rules
	 {Rep error(kind: 'bison error' msg: 'empty grammar')}
      elseif NSymbols > MAXSHORT then
	 {Rep error(kind: 'bison error'
		    msg: ('too many symbols (tokens plus nonterminals);'#
			  'maximum allowed is '#MAXSHORT))}
      else
	 T = {Thread.this}
	 RaiseOnBlock = {Debug.getRaiseOnBlock T}
      in
	 {Debug.setRaiseOnBlock T false}
	 {Wait BisonModule.generate}
	 {Debug.setRaiseOnBlock T RaiseOnBlock}
	 {BisonModule.generate Grammar VerboseFile}
      end
   end
end






