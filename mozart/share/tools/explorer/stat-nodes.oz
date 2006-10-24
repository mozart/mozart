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

   fun {GetStat Ns D MDI ?MDO DNI ?DNO SNI ?SNO FNI ?FNO UNI ?UNO}
      case Ns of nil then
	 MDO={Max D MDI} DNO=DNI SNO=SNI FNO=FNI UNO=UNI nil
      [] N|Nr then MDT DNT SNT FNT UNT in
	 case N.kind
	 of succeeded then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI+1 FNT=FNI UNT=UNI  s
	 [] failed then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI+1 UNT=UNI  f
	 [] suspended then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI UNT=UNI+1  b
	 [] choose then
	    c({GetStat {N getKids($)} D+1 MDI ?MDT
	       DNI+1 ?DNT SNI ?SNT FNI ?FNT UNI ?UNT})
	 end
	 |{GetStat Nr D MDT ?MDO DNT ?DNO SNT ?SNO FNT ?FNO UNT ?UNO}
      end
   end
      
in

   StatNodes = c(choose:
		    class $
		       meth stat($)
			  D DN SN FN UN
		       in
			  stat(shape: c({GetStat @kids 1 1 ?D 1
					     ?DN 0 ?SN 0 ?FN 0 ?UN})
			       start: {self.mom findDepth(1 $)}
			       depth: D
			       c:     DN
			       s:     SN
			       f:     FN
			       b:     UN)
		       end
		    end
		 failed:
		    class $
		       meth stat($)
			  stat(shape: failed
			       start: {self.mom findDepth(1 $)}
			       depth: 1
			       c:     0
			       s:     0
			       f:     1
			       b:     0)
		       end
		    end
		 succeeded:
		    class $
		       meth stat($)
			  stat(shape: succeeded
			       start: {self.mom findDepth(1 $)}
			       depth: 1
			       c:     0
			       s:     1
			       f:     0
			       b:     0)
		       end
		    end
		 suspended:
		    class $
		       meth stat($)
			  stat(shape: suspended
			       start: {self.mom findDepth(1 $)}
			       depth: 1
			       c:     0
			       s:     0
			       f:     0
			       b:     1)
		       end
		    end)
   
end
