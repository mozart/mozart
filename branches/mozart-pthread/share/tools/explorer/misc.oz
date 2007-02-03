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
   fun {ComputeCM Is}
      case {Reverse Is}
      of Unit|RevNum then
	 Num = {Reverse RevNum}
	 N   = if {String.isInt Num} then
		  {Int.toFloat {String.toInt Num}}
	       elseif {String.isFloat Num} then
		  {String.toFloat Num}
	       else false
	       end
	 F   = case Unit    
	       of &i then 2.54
	       [] &c then 1.00
	       [] &m then 10.0
	       [] &p then 0.035277778
	       else false
	       end
      in
	 if N==false orelse F==false then false
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
	 case {ComputeCM RW} of false then false
	 elseof W then
	    case {ComputeCM RH} of false then false
	    elseof H then o(width:W height:H)
	    end
	 end
      else false
      end
   end
end


