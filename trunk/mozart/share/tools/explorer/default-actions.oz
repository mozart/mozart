%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   InfoActions = [add(information:
			 proc {$ N P}
			    {Show N#{P}}
			 end
		      label: 'Show')
		  add(information:
			 proc {$ N P}
			    {Browse N#{P}}
			 end
		      label: 'Browse')
		  add(information:separator)]

   local

      local
	 fun {Close Fs}
	    case {System.isVar Fs} then nil
	    else !Fs=F|Fr in F|{Close Fr}
	    end
	 end
      in
	 fun {OFSArity R}
	    OpenArity = {RecordC.monitorArity R Flag}
	    Flag      = !True
	 in
	    {Close OpenArity}
	 end
      end
      
      
      local
	 proc {MergeTuple I X Y T OE ON}
	    case I==0 then true
	    else T.I={MergeTree X.I Y.I OE ON} {MergeTuple I-1 X Y T OE ON}
	    end
	 end
	 
	 proc {MergeRecord As X Y R OE ON}
	    case As of nil then true
	    [] A|Ar then
	       R.A={MergeTree X.A Y.A OE ON} {MergeRecord Ar X Y R OE ON}
	    end
	 end
	 
      in
	 fun {MergeTree X Y OE ON}
	    if X=Y then {OE X}
	    [] true then
	       case {System.isVar X} orelse {System.isVar Y} then {ON X Y}
	       else XT={Value.type X} in
		  case XT=={Value.type Y} then
		     case XT
		     of tuple  then L={Label X} W={Width X} in
			case L=={Label Y} andthen W=={Width Y} then
			   NT={MakeTuple L W} in {MergeTuple W X Y NT OE ON} NT
			else {ON X Y}
			end
		     [] record then
			L={Label X} W={Width X}
		     in
			case L=={Label Y} andthen W=={Width Y} then
			   A={Arity X}
			in
			   case A=={Arity Y} then
			      NR={MakeRecord L A} in
			      {MergeRecord A X Y NR OE ON} NR
			   else {ON X Y}
			   end
			else {ON X Y}
			end
		     else {ON X Y}
		     end
		  else {ON X Y}
		  end
	       end
	    end
	 end
      end

      fun {Equal X} X end

      fun {NotEqual X Y} X#Y end
   
      fun {Merge P1 P2}
	 {MergeTree {P1} {P2} Equal NotEqual}
      end

   in

      CmpActions = [add(compare:
			   proc {$ N1 P1 N2 P2}
			      {Show N1#N2#{Merge P1 P2}}
			   end
			label: 'Show Merge')
		    add(compare:
			   proc {$ N1 P1 N2 P2}
			      {Browse N1#N2#{Merge P1 P2}}
			   end
			label: 'Browse Merge')
		    add(compare:separator)]



   end

   StatActions = [add(statistics:
			 proc {$ N S}
			    {Show N#{Record.subtract S shape}}
			 end
		      label: 'Show')
		  add(statistics:
			 proc {$ N S}
			    {Browse N#{Record.subtract S shape}}
			 end
		      label: 'Browse')
		  add(statistics:separator)]
   
in

   [InfoActions CmpActions StatActions]

end
