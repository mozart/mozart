%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   HasErrors = {NewCell false}

   AST = {Compiler.parseOzFile 'gump/ProductionTemplates.ozg'
	  proc {$ M}
	     case M of error(...) then
		{Assign HasErrors true}
	     else skip
	     end
	  end
	  fun {$ _} true end
	  nil}

   if {Access HasErrors} then
      {Exception.raiseError gump(errorsInProductionTemplates)}
   end
   case AST of [fSynTopLevelProductionTemplates(_)] then skip
   else {Exception.raiseError gump(noProductionTemplates AST)}
   end
in
   functor
   export
      Default
   body
      [fSynTopLevelProductionTemplates(Default)] = AST
   end
end

