%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
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

functor $
import
   NativeEmitter
export
   'create' : Create
define
   local
      fun {ToString V}
	 {VirtualString.toString V}
      end
      
      fun {HasPrefix Ps Ns}
	 case Ps
	 of nil  then true
	 [] P|Pr then
	    case Ns
	    of N|Nr then P == N andthen {HasPrefix Pr Nr}
	    [] _    then false
	    end
	 end
      end

      fun {GetId Key#Item}
	 if {IsName Key}
	 then "ignore"
	 elsecase Item
	 of 'function'(Id _ _) then Id
	 [] 'union'(...)       then {ToString Key}
	 [] 'struct'(...)      then {ToString Key}
	 [] 'enum'(...)        then {ToString Key}
	 else "ignore"
	 end
      end
      
      fun {FilterEntry Prefix Entry}
	 {HasPrefix Prefix {GetId Entry}}
      end

      proc {EmitNative Types#Entries#File}
	 Emitter = {New NativeEmitter.'class' init(Types File)}
      in
	 {Emitter emit(Entries)}
	 {Emitter emitInterfacePrefix}
	 {Emitter emitInterface(Entries)}
	 {Emitter emitInitStub(File)}
	 {Emitter close}
      end
   in
      fun {Create T}
	 GdkEntries GtkEntries GtkCanvasEntries
      in
	 {List.partition
	  {List.partition
	   {List.partition {Dictionary.entries T} fun {$ E} {FilterEntry "gdk" E} end GdkEntries}
	   fun {$ E} {FilterEntry "gtk_canvas" E} end GtkCanvasEntries}
	  fun {$ E} {FilterEntry "gtk" E} end GtkEntries _}
	 {ForAll [T#GdkEntries#"GdkNative.c"
		  T#GtkEntries#"GtkNative.c"
		  T#GtkCanvasEntries#"GtkCanvasNative.c"] EmitNative}
	 T
      end
   end
end
