%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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

functor
import
   OS(localTime)
   Open( file: File)
   System(show showInfo)
export
   log : Log
   
define
   fun{GetDate}
      D={OS.localTime}
   in
      if D.year<50 then 2000+D.year else 1900+D.year end#"/"#
      if D.mDay<10 then 0#D.mDay else D.mDay end #"/"#
      if (D.mon + 1)<10 then 0#(D.mon + 1) else (D.mon + 1) end #" - "#
      if D.hour<10 then 0#D.hour else D.hour end #":"#
      if D.min<10 then 0#D.min else D.min end #":"#
      if D.sec<10 then 0#D.sec else D.sec end #" : "
   end
 
   class Log prop locking
      feat
	 fileName

      attr
	 fileHandler
	 
      meth init( file: F )
	 self.fileName = F
	 fileHandler <- {New File init( name: self.fileName
					flags: [ write create append])}
      end

      meth close()
	 {@fileHandler close}
      end

      meth log(Msg)
	 thread
	    {self Log2(Msg {GetDate})}
	 end
	 {Thread.preempt {Thread.this}}
      end
      
      meth Log2(Msg Date resend:R<=0)
	 lock
	    try
	       if R==0 then
		  {@fileHandler write(vs:Date#Msg#"\n")}%
	       else
		  {@fileHandler write(vs:"["#R#"] "#Date#Msg#"\n")}%
	       end
	    catch X then
	       {System.show X}
	       {System.showInfo "* Failed to send ["#R#"]: "#Date#Msg#"\n"}
	       {Delay 1000}
	       {self Log2(Msg Date resend:R+1)}
	    end
	 end
      end
   end
end
