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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
export
   Default
require
   Compiler(parseOzFile)
prepare
   HasErrors = {NewCell false}
   AST = {Compiler.parseOzFile 'gump/ProductionTemplates.ozg'
	  proc {$ M}
	     case M of error(...) then
		{Assign HasErrors true}
	     else skip
	     end
	  end
	  fun {$ _} true end
	  {NewDictionary}}
   if {Access HasErrors} then
      {Exception.raiseError gump(errorsInProductionTemplates)}
   end
   case AST of [fSynTopLevelProductionTemplates(_)] then skip
   else {Exception.raiseError gump(noProductionTemplates AST)}
   end
define
   [fSynTopLevelProductionTemplates(Default)] = AST
end

