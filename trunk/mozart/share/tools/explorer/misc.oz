%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local
   fun {ComputeCM Is}
      case {Reverse Is}
      of Unit|RevNum then
	 Num = {Reverse RevNum}
	 N   = case {String.isInt Num} then
		  {Int.toFloat {String.toInt Num}}
	       elsecase {String.isFloat Num} then
		  {String.toFloat Num}
	       else False
	       end
	 F   = case Unit    
	       of &i then 2.54
	       [] &c then 1.00
	       [] &m then 10.0
	       [] &p then 0.035277778
	       else False
	       end
      in
	 case N==False orelse F==False then False
	 else N*F
	 end
      end
   end
   
in
   fun {CheckSize Is}
      RW XRH
   in
      {List.takeDropWhile {Filter Is Char.isGraph}
       fun {$ I} I\=&x end ?RW ?XRH}
      case XRH of &x|RH then
	 case {ComputeCM RW} of !False then False
	 elseof W then
	    case {ComputeCM RH} of !False then False
	    elseof H then o(width:W height:H)
	    end
	 end
      else False
      end
   end
end
