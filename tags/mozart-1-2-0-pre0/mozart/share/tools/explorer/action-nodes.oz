%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local

   class Leaf
      meth isInSubtree(CurX Depth FindX $)
	 Depth==0 andthen CurX+@offset>FindX
      end

      meth findByX(_ _ _ $)
	 self
      end
   end

   local
      fun {GetRightBorder D Es X}
	 if D>0 then
	    case Es of nil then false
	    [] E|Er then {GetRightBorder D-1 Er E.2+X}
	    end
	 else X
	 end
      end
   in
      class Choose
	 meth FindKids(Ks Depth CurX FindX $)
	    K|Kr = Ks
	 in
	    if Kr==nil then
	       if K.kind==choose then
		  {K findByX(Depth-1 CurX FindX $)}
	       else K
	       end
	    elseif {K isInSubtree(CurX Depth-1 FindX $)} then
	       if K.kind==choose then
		  {K findByX(Depth-1 CurX FindX $)}
	       else K
	       end
	    else Choose,FindKids(Kr Depth CurX FindX $)
	    end
	 end
   
	 meth findByX(Depth MomX FindX $)
	    if Depth>0 then
	       if @isHidden then self
	       else Choose,FindKids(@kids Depth MomX+@offset FindX $)
	       end
	    else self
	    end
	 end

	 meth isInSubtree(CurX Depth FindX $)
	    case {GetRightBorder Depth @shape.2 @offset+CurX}
	    of false then false
	    elseof BorderX then FindX<BorderX
	    end
	 end
      end
   end
   
in

   ActionNodes=c(succeeded: Leaf
		 failed:    Leaf
		 suspended: Leaf
		 choose:    Choose)

end
