%
% Authors:
%   Donatien Grolaux (2000)
%
% Copyright:
%   (c) 2000 Université catholique de Louvain
%
% Last change:
%   $Date$ by $Author$
%   $Revision$
%
% This file is part of Mozart, an implementation
% of Oz 3:
%   http://www.mozart-oz.org
%
% See the file "LICENSE" or
%   http://www.mozart-oz.org/LICENSE.html
% for information on usage and redistribution
% of this file, and for a DISCLAIMER OF ALL
% WARRANTIES.
%
%  The development of QTk is supported by the PIRATES project at
%  the Université catholique de Louvain.

functor

import
   System(show:Show)
   Tk
   QTkDevel(splitParams:        SplitParams
	    tkInit:             TkInit
	    init:               Init
	    assert:             Assert
	    setGet:             SetGet
	    subtracts:          Subtracts
	    noArgs:             NoArgs
	    qTkClass:           QTkClass
	    execTk:             ExecTk
	    returnTk:           ReturnTk
	    convertToType:      ConvertToType
	    globalInitType:     GlobalInitType
	    globalUnsetType:    GlobalUnsetType
	    globalUngetType:    GlobalUngetType)

export

   NewFont
   NewSilentFont
   SetCallBack
   RemoveCallBack
   GetFontObj
   GetFontId
   GetFonts
   GetFont
   IsFont
   
define

   TypeInfo=r(all:r(family:vs
		    size:int
		    weight:[normal bold]
		    slant:[roman italic]
		    underline:boolean
		    overstrike:boolean)
	      uninit:r
	      unset:r
	      unget:r
	     )
   
   FS={NewCell _}
   FontDict={WeakDictionary.new {Access FS}}
   SilentFontDict={WeakDictionary.new _}
   {WeakDictionary.close SilentFontDict}
   NP={NewCell nil}
   Lock={NewLock}
   
   thread
      proc{Loop}
	 case {Access FS} of K#X|Xs then
	    {Notify del(K)}
	    {Assign FS Xs}
	    {Loop}
	 end
      end
   in
      {Loop}
   end

   proc{Notify What}
      lock Lock then
	 {ForAll {Access NP} proc{$ P} {P What} end}
      end
   end

   proc{SetCallBack P Init}
      lock Lock then
	 Init={GetFonts}
	 {Assign NP P|{Access NP}}
      end
   end

   proc{RemoveCallBack P}
      lock Lock then
	 {Assign NP {List.subtract {Access NP} P}}
      end
   end

   fun{FontToList F}
      fun{Loop X}
	 case X of Opt|Val|Ls then
	    O={String.toAtom Opt.2}
	 in
	    O#{ConvertToType Val TypeInfo.all.O}|{Loop Ls}
	 else nil end
      end
   in
      {Loop {Tk.returnList font(configure F)}}
   end
   
   fun{GetFonts}
      fun{Loop L}
	 case L of X|Xs then
	    Ob={WeakDictionary.condGet FontDict {VirtualString.toAtom X} unit}
	 in
	    if Ob==unit then {Loop Xs}
	    else
	       add({VirtualString.toAtom X} {List.toRecord font {FontToList X}})|{Loop Xs}
	    end
	 else nil end
      end
      TkL={Tk.returnList font(names)}
   in
      {Loop TkL}
   end

   fun{GetFontObj Id}
      {WeakDictionary.condGet FontDict Id Id}
   end

   fun{GetFontId Obj}
      {VirtualString.toAtom {Tk.getTclName Obj}}
   end
   
   fun{NewFont D}
      {New QTkNotifyFont {Record.adjoin D Init}}
   end

   fun{NewSilentFont D}
      {New QTkSilentFont {Record.adjoin D Init}}
   end
   
   fun{GetFont Str}
      X={VirtualString.toAtom Str}
      Ob={WeakDictionary.condGet FontDict X
	  {WeakDictionary.condGet SilentFontDict X unit}}
   in
      if Ob==unit then
	 FN={Tk.returnListAtom font(names)}
      in
	 if {List.member X FN} then
	    {Tk.return font(configure X)}
	 else
	    {VirtualString.toString Str}
	 end
      else Ob
      end
   end

   fun{IsFont Fnt}
      {CondSelect Fnt widgetType unit}==font
   end
   
   class QTkFont
      
      from Tk.font SetGet
	 
      feat
	 widgetType:font
	 typeInfo:TypeInfo
	 
	 
      meth !Init(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    Tk.font,{Record.adjoin M tkInit}
	 end
      end
      meth set(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    {ExecTk font configure(self d(M))}
	 end
      end
      meth get(...)=M
	 lock
	    Actual={CondSelect M actual false}
	    DisplayOf={HasFeature M displayof}
	    N={Subtracts M [actual displayof]}
	 in
	    {Assert self.widgetType self.typeInfo N}
	    {Record.forAllInd N
	     proc{$ I R}
		{ReturnTk font
		 if Actual then
		    if DisplayOf then
		       actual(self "-displayof" DisplayOf "-"#I R)
		    else
		       actual(self "-"#I R)
		    end
		 else
		    configure(self "-"#I R)
		 end
		 self.typeInfo.all.I}
	     end}
	 end
      end
      meth delete
	 lock
	    Id={VirtualString.toAtom {Tk.getTclName self}}
	 in
	    {ExecTk font delete(self)}
	 end
      end
      meth families(F)
	 lock
	    fun{Loop Str}
	       case Str
	       of &{|_ then
		  L R
	       in
		  {List.takeDropWhile Str fun{$ C} C\=&} end L R}
		  if R\=nil then
		     {List.drop L 1}|{Loop {List.drop R 2}}
		  else
		     {List.drop L 1}|nil
		  end
	       [] nil then nil
	       else
		  L R
	       in
		  {List.takeDropWhile Str fun{$ C} C\=&  end L R}
		  if R\=nil then
		     L|{Loop {List.drop R 1}}
		  else
		     L|nil
		  end
	       end
	    end
	 in
	    F={Loop {ReturnTk font families($) no}}
	 end
      end
      meth measure(Text Ret displayof:D<=NoArgs)
	 lock
	    if D==NoArgs then
	       {ReturnTk font measure(self Text Ret)}
	    else
	       {ReturnTk font measure(self "-displayof" D Text Ret)}
	    end
	 end
      end
      meth metrics(Opt Ret displayof:D<=NoArgs)
	 lock
	    if D==NoArgs then
	       {ReturnTk font metrics(self "-"#Opt Ret)}
	    else
	       {ReturnTk font metrics(self "-displayof" D "-"#Opt Ret)}
	    end
	 end
      end
   end

   class QTkSilentFont

      from QTkFont

      meth !Init(...)=M
	 Id
      in
	 QTkFont,M
	 Id={GetFontId self}
	 {WeakDictionary.put SilentFontDict Id self}
      end
      meth delete(...)=M
	 Id={GetFontId self}
      in
	 QTkFont,M
	 {WeakDictionary.remove SilentFontDict Id}
      end
   end

   class QTkNotifyFont

      from QTkFont

      meth !Init(...)=M
	 Id
      in
	 QTkFont,M
	 Id={GetFontId self}
	 {WeakDictionary.put FontDict Id self}
	 {Notify add(Id {Record.adjoin M font})}
      end

      meth set(...)=M
	 QTkFont,M
	 {Notify chg({GetFontId self} M)}
      end

      meth delete(...)=M
	 Id={GetFontId self}
      in
	 QTkFont,M
	 {WeakDictionary.remove FontDict Id}
	 {Notify del(Id)}
      end

   end
   
end