%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   PanelTopClosed = {NewName}

   \insert configure.oz

   \insert discrete-scale.oz
   
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
      from BaseObject
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
	    case
	       case
		  What==update andthen {List.sub {Arity OM} [1 time mouse]}
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
		  What==history andthen	{List.sub {Arity OM} [1 range]}
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
		  What==configurable andthen {List.sub {Arity OM} [1 2]}
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
	       E = {System.get errors}
	    in
	       {System.showError '*** Panel Configuration Error'} 
	       {System.showError ('*** Option configuration: ' #
				  {System.valueToVirtualString OM E.depth E.width})}
	       {System.showError ''}
	    end
	 end
      end

   end

end
