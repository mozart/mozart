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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(localTime)
   Open( file: File)

export
   log : Log

define
   fun{GetDate}
      D={OS.localTime}
   in
      if D.year<50 then 2000+D.year else 1900+D.year end#"/"#
      if D.mDay<10 then 0#D.mDay else D.mDay end #"/"#
      if D.mon<10 then 0#D.mon else D.mon end #" - "#
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

      meth log( Msg )
         lock
            {@fileHandler write(vs:{GetDate}#Msg#"\n")}
         end
      end
   end
end
