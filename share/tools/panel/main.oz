%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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

local

   PanelTopClosed = {NewName}

   \insert configure.oz

   \insert runtime-bar.oz

   \insert load.oz

   \insert dialogs.oz

   \insert make-notes.oz

   \insert top.oz

   fun {PickClosest NT Ts}
      ST#_|Tr = Ts
   in
      {FoldL Tr
       fun {$ T#D TA#_}
	  ND = {Abs TA - NT}
       in
	  case ND<D then TA#ND else T#D end
       end
       ST#{Abs ST - NT}}.1
   end

in

   class PanelClass
      prop locking final
      feat Options
      attr ThisPanelTop:unit

      meth init
	 O = self.Options
      in
	 O = {Dictionary.new}
	 {Dictionary.put O config  false}
	 {Dictionary.put O time    DefaultUpdateTime}
	 {Dictionary.put O mouse   true}
	 {Dictionary.put O history DefaultHistoryRange}
      end

      meth open
	 lock
	    case @ThisPanelTop==unit then
	       ThisPanelTop <- thread
				  {Thread.setThisPriority high}
				  {New PanelTop init(manager:self
						     options:self.Options)}
			       end
	    else skip
	    end
	 end
      end

      meth !PanelTopClosed
	 lock
	    ThisPanelTop <- unit
	 end
      end

      meth option(What ...) = OM
	 lock
	    O = self.Options
	 in
	    {Wait @ThisPanelTop}
	    case
	       case
		  What==update andthen {List.sub {Arity OM} [1 mouse time]}
	       then
		  case {HasFeature OM time} then T=OM.time in
		     case {IsNat T} then
			{Dictionary.put O time {PickClosest T UpdateTimes}}
			true
		     else false
		     end
		  else true
		  end
		  andthen
		  case {HasFeature OM mouse} then M=OM.mouse in
		     case {IsBool M} then {Dictionary.put O mouse M} true
		     else false
		     end
		  else true
		  end
	       elsecase
		  What==history andthen {List.sub {Arity OM} [1 range]}
	       then
		  case {HasFeature OM range} then R=OM.range in
		     case {IsNat R} then
			{Dictionary.put O history {PickClosest R HistoryRanges}}
			true
		     else false
		     end
		  else true
		  end
	       elsecase
		  What==configure andthen {List.sub {Arity OM} [1 2]}
	       then
		  case {HasFeature OM 2} then C=OM.2 in
		     case {IsBool C} then {Dictionary.put O config C} true
		     else false
		     end
		  else true
		  end
	       else false
	       end
	    then
	       case @ThisPanelTop of unit then skip elseof T then
		  {T updateAfterOption}
	       end
	    else
	       {Exception.raiseError panel(option OM)}
	    end
	 end
      end

   end

end

