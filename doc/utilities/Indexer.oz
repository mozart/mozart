%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   HTML(empty: EMPTY
        seq: SEQ
        pcdata: PCDATA)
export
   'class': IndexerClass
define
   class IndexerClass
      attr Out: unit
      meth init()
         Out <- EMPTY
      end
      meth enter(Ands HTML)
         Out <- SEQ([@Out p(SEQ({List.foldRTail Ands
                                 fun {$ _#A|Ar In}
                                    A|case Ar of _|_ then PCDATA(', ')
                                      else EMPTY
                                      end|In
                                 end nil})
                            PCDATA(': ') HTML)])
      end
      meth process(?HTML)
         HTML = @Out
      end
   end
end
