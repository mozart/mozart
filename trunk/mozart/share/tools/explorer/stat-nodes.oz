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
	 of succeeded then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI+1 FNT=FNI UNT=UNI  succeeded
	 [] failed then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI+1 UNT=UNI  failed
	 [] blocked then
	    MDT={Max D+1 MDI} DNT=DNI SNT=SNI FNT=FNI UNT=UNI+1  blocked
	 [] choose then
	    choose({GetStat {N getKids($)} D+1 MDI ?MDT
		    DNI+1 ?DNT SNI ?SNT FNI ?FNT UNI ?UNT})
	 end
	 |{GetStat Nr D MDT ?MDO DNT ?DNO SNT ?SNO FNT ?FNO UNT ?UNO}
      end
   end

   fun {GetDepth O N}
      case O==False then N else {GetDepth O.mom N+1} end
   end
      
in

   StatNodes = c(choose:
		    class $
		       meth stat($)
			  D DN SN FN UN
		       in
			  stat(shape:     choose({GetStat @kids 1 1 ?D 1
						  ?DN 0 ?SN 0 ?FN 0 ?UN})
			       start:     {GetDepth self.mom 1}
			       depth:     D
			       choose:    DN
			       succeeded: SN
			       failed:    FN
			       blocked:   UN)
		       end
		    end
		 failed:
		    class $
		       meth stat($)
			  stat(shape:     failed
			       start:     {GetDepth self.mom 1}
			       depth:     1
			       choose:    0
			       succeeded: 0
			       failed:    1
			       blocked:   0)
		       end
		    end
		 succeeded:
		    class $
		       meth stat($)
			  stat(shape:     succeeded
			       start:     {GetDepth self.mom 1}
			       depth:     1
			       choose:    0
			       succeeded: 1
			       failed:    0
			       blocked:   0)
		       end
		    end
		 blocked:
		    class $
		       meth stat($)
			  stat(shape:     blocked
			       start:     {GetDepth self.mom 1}
			       depth:     1
			       choose:    0
			       succeeded: 0
			       failed:    0
			       blocked:   1)
		       end
		    end)
   
end
