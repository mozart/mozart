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
export
   tokens            : Tokens
   toString          : ToString
   toAtom            : ToAtom
   checkPrefix       : CheckPrefix
   stripPrefix       : StripPrefix
   cutPrefix         : CutPrefix
   translateName     : TranslateName
   downTranslate     : MakeClassPrefix
   firstLower        : FirstLower
   firstUpper        : FirstUpper
   indent            : Indent
   filterGdkTypes    : FilterGdkTypes
   filterGtkTypes    : FilterGtkTypes
   filterCanvasTypes : FilterCanvasTypes
define
   %%
   %% Tokenizer
   %%
   
   local
      fun {DoTokens AllTs CurTs Ls Ts}
	 case Ls
	 of nil  then
	    if CurTs == nil
	    then {Reverse AllTs}
	    else {Reverse {Reverse CurTs}|AllTs}
	    end
	 [] L|Lr then
	    if {Member L Ts}
	    then
	       if CurTs == nil
	       then {DoTokens AllTs CurTs Lr Ts}
	       else {DoTokens {Reverse CurTs}|AllTs nil Lr Ts}
	       end
	    else {DoTokens AllTs L|CurTs Lr Ts}
	    end
	 end
      end
   in
      fun {Tokens Ls Ts}
	 {DoTokens nil nil Ls Ts}
      end
   end

   %%
   %% Conversion Stuff
   %%

   fun {ToString V}
      {VirtualString.toString V}
   end
   
   fun {ToAtom V}
      {String.toAtom {ToString V}}
   end

   %%
   %% Prefix Stuff
   %%
   
   fun {CheckPrefix Xs Ys}
      case Xs
      of X|Xr then
	 case Ys
	 of Y|Yr then X == Y andthen {CheckPrefix Xr Yr}
	 [] _    then false
	 end
      [] nil then true
      end
   end
   
   fun {StripPrefix V}
      case V
      of &_|Vr then Vr
      [] V     then V
      end
   end

   fun {CutPrefix Xs Ys}
      case Xs
      of X|Xr then
	 case Ys
	 of Y|Yr then
	    if {Char.toUpper X} == {Char.toUpper Y}
	    then {CutPrefix Xr Yr} else Ys end
	 [] nil  then nil
	 end
      [] nil  then Ys
      end
   end
   
   %%
   %% Name Translation gtk_sample_fun -> gtkSampleFun
   %%
   
   local
      fun {ToUpper Ss}
	 case Ss
	 of S|Sr then {Char.toUpper S}|Sr
	 [] S    then S
	 end
      end
      fun {DoTranslate Ss}
	 case Ss
	 of S|Sr then {ToUpper S}#{DoTranslate Sr}
	 [] nil  then nil
	 end
      end
   in
      fun {TranslateName N}
	 Ss = {Tokens {ToString N} [&_]}
      in
	 case Ss
	 of S|Sr then {ToString {FirstLower S}#{DoTranslate Sr}}
	 [] S    then S
	 end
      end
   end

   fun {FirstLower Vs}
      case Vs
      of V|Vr then {Char.toLower V}|Vr
      [] V    then V
      end
   end

   fun {FirstUpper Vs}
      case Vs
      of V|Vr then {Char.toUpper V}|Vr
      [] V    then V
      end
   end

   %%
   %% Name Translation gtkDoo -> gtk_Doo_
   %%

   local
      fun {MakeClassPrefixIter Ss NewSs Flag}
	 case Ss
	 of S|Sr then
	    if {Char.isLower S}
	    then {MakeClassPrefixIter Sr S|NewSs true}
	    elseif Flag
	    then {MakeClassPrefixIter Sr {Char.toLower S}|&_|NewSs false}
	    else {MakeClassPrefixIter Sr {Char.toLower S}|NewSs true}
	    end
	 [] nil  then {Reverse &_|NewSs}
	 end
      end
   in
      fun {MakeClassPrefix Ss}
	 {MakeClassPrefixIter {FirstLower {ToString Ss}} nil true}
      end
   end
   
   %%
   %% Indentation
   %%

   fun {Indent N}
      if N == 0 then "" else "   "#{Indent (N - 1)} end
   end

   %%
   %% Gdk/Gtk/GtkCanvas Function Filter
   %%

   fun {FilterGdkTypes Type}
      if {IsName Type}
      then false
      else
	 TypeS = {TranslateName {ToString Type}}
      in
	 {CheckPrefix "gdk" TypeS}
      end
   end
   
   fun {FilterGtkTypes Type}
      if {IsName Type}
      then false
      else
	 TypeS = {TranslateName {ToString Type}}
      in
	 if {CheckPrefix "gdk" TypeS}
	 then false
	 elseif {CheckPrefix "gtkCanvas" TypeS}
	 then false
	    else true
	 end
      end
   end
   
   fun {FilterCanvasTypes Type}
      if {IsName Type}
      then false
      else
	 TypeS = {TranslateName {ToString Type}}
      in
	 {CheckPrefix "gtkCanvas" TypeS}
      end
   end
end
