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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Gdbm at 'x-oz://contrib/gdbm'
   HTML(seq: SEQ pcdata: PCDATA)
export
   'class': CrossReferencerClass
define
   class CrossReferencerClass
      attr
	 Prefix: unit TopKey: unit DB: unit Reporter: unit
	 TopEnter: unit ToEnter: unit
      meth init(X Y DBName Rep)
	 TopKey <- X
	 Prefix <- Y
	 DB <- case DBName of unit then unit
	       else
		  try
		     {Gdbm.new write(DBName)}
		  catch _ then
		     {Gdbm.new create(DBName)}
		  end
	       end
	 Reporter <- Rep
	 ToEnter <- nil
      end
      meth close()
	 if @DB \= unit then
	    case @TopEnter of unit then skip
	    elseof HTML then
	       try
		  {Gdbm.put @DB {VirtualString.toAtom @TopKey}
		   (@Prefix#@TopKey#'/index.html')#HTML}
	       catch error(dp(...) ...) then skip
	       end
	    end
	    {ForAll @ToEnter
	     proc {$ ID#To#HTML}
		try
		   {Gdbm.put @DB {VirtualString.toAtom @TopKey#':'#ID}
		    (@Prefix#@TopKey#'/'#To)#HTML}
		catch error(dp(...) ...) then skip
		end
	     end}
	    ToEnter <- nil
	    {Gdbm.close @DB}
	    DB <- unit
	 end
      end
      meth get(DocID SubID ?To ?HTML) Res1 Res2 in
	 Res1 = case @DB of unit then unit
		else Key in
		   Key = {VirtualString.toAtom DocID}
		   {Gdbm.condGet @DB Key unit}
		end
	 Res2 = case @DB of unit then unit
		elsecase SubID of unit then unit
		else Key in
		   Key = {VirtualString.toAtom DocID#':'#SubID}
		   {Gdbm.condGet @DB Key unit}
		end
	 if Res1 == unit orelse SubID \= unit andthen Res2 == unit then
	    {@Reporter warn(kind: 'cross-reference warning'
			    msg: 'unresolved external reference'
			    items: (hint(l: 'Document' m: DocID)|
				    case SubID of unit then nil
				    else [hint(l: 'Key' m: SubID)]
				    end))}
	 end
	 case Res1 of unit then
	    case Res2 of unit then
	       To = unit
	       HTML = PCDATA('[??]')
	    [] X#Y then
	       To = X
	       HTML = Y
	    end
	 [] A#B then
	    case Res2 of unit then
	       To = A
	       HTML = B
	    [] X#Y then
	       To = X
	       HTML = SEQ([Y PCDATA(' of ') B])
	    end
	 end
      end
      meth put(ID To HTML)
	 if @TopKey \= unit then
	    ToEnter <- ID#To#HTML|@ToEnter
	 end
      end
      meth putTop(HTML)
	 if @TopKey \= unit then
	    TopEnter <- HTML
	 end
      end
   end
end
