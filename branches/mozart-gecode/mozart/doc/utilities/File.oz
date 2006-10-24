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
   Open(file text)
export
   text: TextFile
   read: ReadFile
   write: WriteFile

   BaseName
   ChangeExtension
define
   class TextFile from Open.file Open.text
      prop final
      meth readAll($)
	 case TextFile, getS($) of false then ""
	 elseof S then S#'\n'#TextFile, readAll($)
	 end
      end
   end

   proc {ReadFile File ?VS} F in
      F = {New TextFile init(name: File flags: [read])}
      {F readAll(?VS)}
      {F close()}
   end

   proc {WriteFile VS File} F in
      F = {New Open.file init(name: File flags: [write create truncate])}
      {F write(vs: VS)}
      {F close()}
   end

   fun {BaseName V}
      {Reverse
       {List.takeWhile {Reverse {VirtualString.toString V}}
	fun {$ C} C \= &/ end}}
   end

   fun {ChangeExtension VS OldExt NewExt}
      OrigS = {VirtualString.toString VS}
      fun {ChangeExtensionSub S OldExt NewExt}
	 if S == OldExt then NewExt
	 else
	    case S of C|Cr then
	       C|{ChangeExtensionSub Cr OldExt NewExt}
	    [] nil then NewExt
	    end
	 end
      end
   in
      {ChangeExtensionSub OrigS
       {VirtualString.toString OldExt}
       {VirtualString.toString NewExt}}
   end
end
