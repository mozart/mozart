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
   class DummyReporter
      attr Hd: unit Tl: unit
      meth init() X in
	 Hd <- X
	 Tl <- X
      end
      meth store(Msg) NewTl in
	 @Tl = Msg|NewTl
	 Tl <- NewTl
      end
      meth get($)
	 @Tl = nil
	 @Hd
      end
   end

   O = {New DummyReporter init()}

   AST = {Compiler.parseOzFile 'gump/ProductionTemplates.ozg'
	  proc {$ M} {O store(M)} end fun {$ _} true end nil}

   case {O get($)} of nil then
      case AST of [fSynTopLevelProductionTemplates(_)] then skip
      else {Exception.raiseError gump(noProductionTemplates AST)}
      end
   elseof Ms then {Exception.raiseError gump(errorsInProductionTemplates Ms)}
   end
in
   functor
   export
      Default
   body
      [fSynTopLevelProductionTemplates(Default)] = AST
   end
end

