%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   class Solved

      meth getInfo($)
	 %% If this method is invoked, then no solution is stored, and
	 %% it cannot be the root node, since the root node is treated
	 %% specially in <nodes.oz>
	 CurNs CurCopy
      in
	 <<findPathAndCopy(?CurNs ?CurCopy)>>
	 case {Solve CurCopy CurNs}
	 of solved(P _) then P
	 else False
	 end
      end

      meth isInSubtree(CurX Depth FindX $)
	 Depth==0 andthen CurX+@offset>FindX
      end

      meth findByX(_ _ _ $)
	 self
      end
   end

   class FailedOrUnstable

      meth isInSubtree(CurX Depth FindX $)
	 Depth==0 andthen CurX+@offset>FindX
      end

   end

   local
      fun {GetRightBorder D Es X}
	 case D>0 then
	    case Es of nil then False
	    [] E|Er then {GetRightBorder D-1 Er E.2+X}
	    end
	 else X
	 end
      end
   in
      class Choice

	 meth getInfo($)
	    CurNs CurCopy
	 in
	    <<findPathAndCopy(?CurNs ?CurCopy)>>
	    case {Solve CurCopy CurNs}
	    of choice(P _) then P
	    else False
	    end
	 end

	 meth FindKids(Ks Depth CurX FindX $)
	    !Ks=K|Kr
	 in
	    case Kr==nil then
	       case K.kind of choice then
		  {K findByX(Depth-1 CurX FindX $)}
	       else K
	       end
	    elsecase {K isInSubtree(CurX Depth-1 FindX $)} then
	       case K.kind of choice then
		  {K findByX(Depth-1 CurX FindX $)}
	       else K
	       end
	    else <<Choice FindKids(Kr Depth CurX FindX $)>>
	    end
	 end
   
	 meth findByX(Depth MomX FindX $)
	    case Depth>0 then
	       case @isHidden then self
	       else <<Choice FindKids(@kids Depth MomX+@offset FindX $)>>
	       end
	    else self
	    end
	 end

	 meth isInSubtree(CurX Depth FindX $)
	    case {GetRightBorder Depth @shape.2 @offset+CurX}
	    of !False then False
	    elseof BorderX then FindX<BorderX
	    end
	 end
      end
   end
   
in

   ActionNodes=c(solved:   Solved
		 failed:   FailedOrUnstable
		 unstable: FailedOrUnstable
		 choice:   Choice)

end
