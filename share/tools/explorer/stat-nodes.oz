%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {GetStat Ns D MDI ?MDO DNI ?DNO SNI ?SNO FNI ?FNO UNI ?UNO}
      case Ns of nil then
	 MDO={Max D MDI} DNO=DNI SNO=SNI FNO=FNI UNO=UNI nil
      [] N|Nr then MDT DNT SNT FNT UNT in
	 case N.kind
	 of solved then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI+1 FNT=FNI UNT=UNI  solved
	 [] failed then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI+1 UNT=UNI  failed
	 [] unstable then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI UNT=UNI+1  unstable
	 [] choice then
	    choice({GetStat {N getKids($)} D+1 MDI ?MDT
		    DNI+1 ?DNT SNI ?SNT FNI ?FNT UNI ?UNT})
	 end
	 |{GetStat Nr D MDT ?MDO DNT ?DNO SNT ?SNO FNT ?FNO UNT ?UNO}
      end
   end

   fun {GetDepth O N}
      case O==False then N else {GetDepth O.mom N+1} end
   end
      
in

   StatNodes = c(choice:
		    class $
		       meth stat($)
			  D DN SN FN UN
		       in
			  stat(shape:    choice({GetStat @kids 1 1 ?D 1
						 ?DN 0 ?SN 0 ?FN 0 ?UN})
			       start:    {GetDepth self.mom 1}
			       depth:    D
			       choice:   DN
			       solved:   SN
			       failed:   FN
			       unstable: UN)
		       end
		    end
		 failed:
		    class $
		       meth stat($)
			  stat(shape:    failed
			       start:    {GetDepth self.mom 1}
			       depth:    1
			       choice:   0
			       solved:   0
			       failed:   1
			       unstable: 0)
		       end
		    end
		 solved:
		    class $
		       meth stat($)
			  stat(shape:    solved
			       start:    {GetDepth self.mom 1}
			       depth:    1
			       choice:   0
			       solved:   1
			       failed:   0
			       unstable: 0)
		       end
		    end
		 unstable:
		    class $
		       meth stat($)
			  stat(shape:    unstable
			       start:    {GetDepth self.mom 1}
			       depth:    1
			       choice:   0
			       solved:   0
			       failed:   0
			       unstable: 1)
		       end
		    end)
   
end
