/*
 *  Main authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>     
 *     
 *
 *  Contributing authors:
 *
 *  Copyright:
 *     Alejandro Arbelaez
 *     
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */
					
   proc {MakeDistrTuple V ?T}
      T = {VectorToTuple V}
      if {Record.all T IsVar} then skip else
	 {Exception.raiseError
	  kernel(type MakeDistrTuple [V T] 'vector(intvars)' 1
		 'Distribution vector must contain IntVar variables.')}
      end
   end
   
   SelVal = map(min: Reflect.min
		max: Reflect.max
	       )

   %% Generic only
   GenSelVar = map(naive:   fun {$ _ _}
			       false
			    end
		   size:   fun {$ X Y}
			      {Reflect.size X} < {Reflect.size Y}
			   end
		  )
	    
   GenSelFil = map(undet:  fun {$ X}
			      {Bool.'not' {IsDet X}} andthen ({Reflect.size X} > 1)			      
			   end)

   GenSelPro = map(noProc: unit)
   
   GenSelSel = map(id:     fun {$ X}
			      X
			   end)
   
   fun {MapSelect Map AOP}
      if {IsAtom AOP} then Map.AOP else AOP end
   end
   
   fun {PreProcessSpec Spec}
      FullSpec = {Adjoin
		  generic(order:     size
			  filter:    undet
			  select:    id
			  value:     min
			  procedure: noProc)
		  case Spec
		  of naive then generic(order:naive)
		  [] ff    then generic
		  [] splitUpper then generic(value:splitUpper)
		  [] rand then generic(order:rand)
		  [] bool then generic(value:bool)
		  else Spec
		  end}
   in
      gen(order:     {MapSelect GenSelVar FullSpec.order}
	  value:     {MapSelect SelVal FullSpec.value}
	  select:    {MapSelect GenSelSel FullSpec.select}
	  filter:    {MapSelect GenSelFil FullSpec.filter}
	  procedure: {MapSelect GenSelPro FullSpec.procedure})
   end
   

   %% Returns the filtered list of vars
   %% as well as the chosen variable.
   fun {ChooseAndRetFiltVars Vars Order Filter}
      NewVars
      fun {Loop Vars Accu NewTail}
	 case Vars of nil then
	    NewTail=nil
	    Accu|NewVars
	 [] H|T then
	    if {Filter H} then LL in NewTail=(H|LL)
	       {Loop T
		if Accu==unit orelse {Order H Accu}
		then H else Accu end
		LL}
	    else {Loop T Accu NewTail} end
	 end
      end
   in
      {Loop Vars unit NewVars}
   end

      
   proc {IntVarDistribute RawSpec Vec}
      case {PreProcessSpec RawSpec}
      of gen(value:     SelVal
	     order:     Order
	     select:    Select
	     filter:    Fil
	     procedure: Proc) then
	 if {Width Vec}>0 then
	    
	    proc {Do Xs}
	       {Space.waitStable}
	       E|Fs={ChooseAndRetFiltVars Xs Order Fil}

	    in
	       if E\=unit then
		  V={Select E}
	       in
		  if Proc\=unit then
		     {Proc}
		     {Space.waitStable}
		  end		  
		  choice
		     %{System.show aqui_equal#{IsDet V}#V}			
		     {Rel V Rt.'=:' {SelVal V} Cl.bnd}
		  []
		     %{System.show aqui_different#{IsDet V}#V}
		     {Rel V Rt.'\\=:' {SelVal V} Cl.bnd}
		  end		  
		  {Do Fs}
	       end
	    end
	 in
	    {Do {VectorToList Vec}}
	 end
      end
   end

