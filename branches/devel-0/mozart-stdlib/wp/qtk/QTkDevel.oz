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


\define DEBUG

functor

import
   System(show:Show)
   Browser(browse:Browse)
   Tk
   Error
   QTk at 'QTkBare.ozf'
   Property(get)
   GS at 'GetSignature.ozf'
   QTkMigratable(getMigratableClass:GetMigratableClass) at 'QTkMigratable.ozf'
   QTkFont(getFont:GetFont) at 'QTkFont.ozf'
   
export

   split:              Split
   splitGeometry:      SplitGeometry
   splitParams:        SplitParams
   makeClass:          MakeClass
   execTk:             ExecTk
   lastInt:            LastInt
   returnTk:           ReturnTk
   tkInit:             TkInit
   checkType:          CheckType
   convertToType:      ConvertToType
   setAssertLevel:     SetAssertLevel
   assert:             Assert
   setGet:             SetGet
   qTkAction:          QTkAction
   qTkClass:           QTkClass
   subtracts:          Subtracts
   qTkTooltips:        TkToolTips
   setTooltips:        SetToolTips
   globalInitType:     GlobalInitType
   globalUnsetType:    GlobalUnsetType
   globalUngetType:    GlobalUngetType
   flattenLabel:       FlattenLabel
   mapLabelToObject:   MapLabelToObject
%   registerWidget:     RegisterWidget
%   getWidget:          GetWidget
   qTk:                NQTk
   QTkDesc
%   loadTk:             LoadTk
%   loadTkPI:           LoadTkPI
   NewLook
   DefLook
%   PropagateLook
%   SetAlias
%   UnSetAlias
%   GetAlias
   GetBuilder
   Builder
   Init
   WInfo
   Feature
   ParentFeature
   EventDict
   StoreInEventDict
   GetSignature
   Grid
   StoreResource
   NewResource
   GetObjectFromTclId
   NotifyResource
   GetResourceClass

   
prepare
   FlattenLabel={NewName}
   MapLabelToObject={NewName}
   Grid={NewName}
   QTkDesc = {NewName}
   Builder = {NewName}
   Init    = {NewName}
   EventDict={NewName}
   StoreInEventDict={NewName}   
   Register={NewName}
   GetSignature={NewName}
   NewResource={NewName}
   StoreResource={NewName}
   NotifyResource={NewName}
   GetObjectFromTclId={NewName}
   GetResourceClass={NewName}

%%% Functions and procedures that are heavily used when developping QTk widgets

   fun{Split Str}
      %
      % This function splits a string representing a tcl list into an Oz list
      %
      R
      fun{Loop L S A B}
	 case L of X|Xs then
	    case X
	    of 32 then % space
	       if S==0 andthen A==0 andthen B==0 then % no " { [ pending
		  R=Xs
		  nil
	       else
		  X|{Loop Xs S A B}
	       end
	    [] 34 then % "
	       if S==0 then
		  X|{Loop Xs 1 A B}
	       else
		  X|{Loop Xs 0 A B}
	       end
	    [] 123 then % {
	       X|{Loop Xs S A+1 B}
	    [] 91 then % [
	       X|{Loop Xs S A B+1}
	    [] 93 then % ]
	       X|{Loop Xs S A B-1}
	    [] 125 then % }
	       X|{Loop Xs S A-1 B}
	    else
	       X|{Loop Xs S A B}
	    end
	 else
	    R=nil
	    nil
	 end
      end
      fun{Purge Str}
	 case Str of X|Xs then
	    if X==34 orelse X==123 orelse X==91 then
	       {List.take Xs {Length Xs}-1}
	    else
	       Str
	    end
	 else
	    Str
	 end
      end
   in
      if Str==nil then nil else
	 {Purge {Loop Str 0 0 0}}|{Split R}
      end
   end
   
   fun {SplitGeometry Str}
      %
      % This function splits a string of integers separated by control characters
      % into an Oz list of the integers. ( e.g "100x200+10+40" into [100 200 10 40])
      %
      R
      fun{Loop Str}
	 case Str of X|Xs then
	    if X>=48 andthen X=<57 then
	       X|{Loop Xs}
	    else
	       R=Xs
	       nil
	    end
	 else
	    R=nil
	    nil
	 end
      end
   in
      if Str==nil then nil else
	 {String.toInt {Loop Str}}|{SplitGeometry R}
      end
   end

   fun{TkInit Var}
      %
      % This function returns a record that is Var minus several features that aren't
      % valid tk parameters.
      %
      {Record.adjoin
       {Record.filterInd Var
	fun{$ I R} {Int.is I}==false andthen
	   {Member I [glue padx pady init return tooltips handle action look]}==false
	end}
       tkInit}
   end

   proc{SplitParams Rec L A B}
      %
      % This procedure splits Rec into two records where all features named in the
      % L list are placed into the B variable
      %
      {Record.partitionInd Rec
       fun{$ I _}
	  {List.member I L}
       end
       B A}
   end

   fun{Subtracts Rec L}
      %
      % This function returns the record Rec minus all feature that are in the list L
      %
%      case L
%      of X|Xs then {Subtracts {Record.subtract Rec X} Xs}
%      else Rec
%      end
      {Record.filterInd Rec fun{$ I _} {Member I L}==false end}
   end
   
%   fun{RecordToTk R}
%      {List.toRecord
%       tk
%       {List.append
%	[1#{Label R}]
%	{List.map {Record.toListInd R}
%	 fun{$ V}
%	    case V of I#J then
%	       if {IsInt I} then I+1#J else
%		  V
%	       end
%	    end
%	 end}}}
%   end

   fun{LastInt R}
      fun{Max N L}
	 case L of X|Xs then
	    case X of I#_ then
	       if {Int.is I} andthen I>N then
		  {Max I Xs}
	       else
		  {Max N Xs}
	       end
	    end
	 else
	    N
	 end
      end
   in
      {Max 0 {Record.toListInd R}}
   end
   
   %
   % These three variables are usual parameters to specify the type information of
   % widgets
   %
   
   GlobalInitType=r(glue:nswe
		    padx:natural
		    pady:natural
%		    return:free
		    feature:atom
		    parent:no
		    handle:free
		    tooltips:vs
		    blackbox:free
		    look:no)

   GlobalUnsetType={Record.adjoinAt {Record.subtract GlobalInitType tooltips} return unit}

   GlobalUngetType={Record.adjoinAt {Record.subtract {Record.subtract GlobalInitType tooltips} blackbox} return unit}

   %% Taken from Tk.oz from Christian Schulte
   
%   Stok  = String.token
   Stoks = String.tokens
   S2F   = String.toFloat
   S2I   = String.toInt
   SIF   = String.isFloat
   SII   = String.isInt

%   V2S   = VirtualString.toString
   
   %%
   %% Some Character/String stuff
   %%
   local
      fun {TkNum Is BI ?BO}
	 case Is of nil then BO=BI nil
	 [] I|Ir then
	    case I
	    of &- then &~|{TkNum Ir BI BO}
	    [] &. then &.|{TkNum Ir true BO}
	    [] &e then &e|{TkNum Ir true BO}
	    [] &E then &E|{TkNum Ir true BO}
	    else I|{TkNum Ir BI BO}
	    end
	 end
      end
   in
%      fun {TkStringToString S}
%	 S
%      end

      TkStringToAtom = StringToAtom

      fun {TkStringToInt S}
	 %% Read a number and convert it to an integer
	 OS IsAFloat in OS={TkNum S false ?IsAFloat}
	 if IsAFloat andthen {SIF OS} then
	    {FloatToInt {S2F OS}}
	 elseif {Not IsAFloat} andthen {SII OS} then
	    {S2I OS}
	 else false
	 end
      end

      fun {TkStringToFloat S}
	 %% Read a number and convert it to a float
	 OS IsAFloat in OS={TkNum S false ?IsAFloat}
	 if IsAFloat andthen {SIF OS} then
	    {S2F OS}
	 elseif {Not IsAFloat} andthen {SII OS} then
	    {IntToFloat {S2I OS}}
	 else false
	 end
      end

      fun {TkStringToListString S}
	 {Stoks S & }
      end

      fun {TkStringToListAtom S}
	 {Map {Stoks S & } TkStringToAtom}
      end

      fun {TkStringToListInt S}
	 {Map {Stoks S & } TkStringToInt}
      end

      fun {TkStringToListFloat S}
	 {Map {Stoks S & } TkStringToFloat}
      end
   end

   %% Back to my own code

define

%   WeakDictionary=Dictionary
   
   fun{ConvertToType Str Type}
      %
      % This function converts a string into the specified type
      %
      if {IsList Type} then
	 R={List.dropWhile Type
	    fun{$ E}
	       {VirtualString.toString E}\=Str
	    end}
      in
	 if {Length R}==0 then
	    {Exception.raiseError qtk(custom "Internal Error" "Can't convert "#Str#" to the correct type" "")}
	    unit
	 else
	    {List.nth R 1}
	 end
      else
	 {Wait Str}
	 case Type
	 of no then Str
	 [] nswe then {String.toAtom Str}
	 [] pixel then try
			  {String.toInt Str}
		       catch _ then {String.toAtom Str} end
	 [] vs then Str
	 [] color then
	    if {List.nth Str 1}==35 then %#RRGGBB
	       BPV=({Length Str}-1) div 3
	       RR GG BB T
	       {List.takeDrop {List.drop Str 1} BPV RR T}
	       {List.takeDrop T BPV GG BB}
	       R G B
	       [R G B]={List.map [RR GG BB] fun{$ C} {List.take {VirtualString.toString C#"0"} 2} end} % takes only the two most significant bytes
	       fun{Convert V}
		  C={List.last V}


		  
		  T=if C>=48 andthen C=<57 then C-48
		    elseif C>=97 andthen C=<102 then C-87
		    elseif C>=65 andthen C=<70 then C-55
		    else C end
	       in
		  if {Length V}>1 then
		     T+16*{Convert {List.take V {Length V}-1}}
		  else
		     T
		  end
	       end
	    in
	       c({Convert R} {Convert G} {Convert B})
	    else
	       {String.toAtom Str}
	    end
	 [] colortrans then
	    if Str==nil then nil else {ConvertToType Str color} end
	 [] cursor then {String.toAtom Str}
	 [] bitmap then {String.toAtom Str}
	 [] atom then {String.toAtom Str}
	 [] anchor then {String.toAtom Str}
	 [] relief then {String.toAtom Str}
	 [] boolean then Str=="1"
	 [] natural then {TkStringToInt Str}
	 [] int then {TkStringToInt Str}
	 [] integer then {TkStringToInt Str}
	 [] float then {TkStringToFloat Str}
	 [] list then {TkStringToListString Str}
	 [] listInt then {TkStringToListInt Str}
	 [] listFloat then {TkStringToListFloat Str}
	 [] listAtom then {TkStringToListAtom Str}
	 [] font then {GetFont Str}
	 [] scrollregion then
	    if Str==nil then q else
	       fun{Split What}
		  A B in
		  {List.takeDropWhile What
		   fun{$ C}
		      C\=32
		   end
		   A B}
		  if B==nil then
		     A|nil
		  else
		     A|{Split {List.drop B 1}}
		  end
	       end
	       fun{ToNumber What}
		  R
	       in
		  try
		     R={String.toFloat What}
		  catch _ then skip end
		  try
		     R={String.toInt What}
		  catch _ then skip end
		  if {IsFree R} then
		     {Exception.raiseError qtk(custom "Type Conversion Error" "Can't convert "#What#" to number")}
		     0
		  else R end
	       end
	    in
	       {List.toRecord q {List.mapInd
				 {List.map 
				  {Split Str}
				  ToNumber}
				 fun{$ I S} I#S end}}
	    end
	 else
	    {Exception.raiseError qtk(custom "Type Error" "Target type "#Type#" is unkown")}
	    unit
	 end
      end
   end

   Lock={NewLock}
   NoArgs={NewName}
   Toplevel={NewName}
   NQTk={ByNeed fun{$} QTk end}
   Win32=({Property.get 'platform'}.os==win32)

   fun{NewLook}
      L={NewDictionary}
   in
      look(set:proc{$ P}
		  {Dictionary.put L {Label P} P}
	       end
	   get:fun{$ P}
		  {Record.adjoin
		   {Dictionary.condGet L {Label P} r}
		   P}
	       end)
   end

   DefLook=look(set:proc{$ P} skip end
		get:fun{$ P} P end)
   
   proc{ExecTk Obj Msg}
      if {Access AssertLevel}.all==none then
	 {Tk.send 'catch'(v("{") b([Obj Msg]) v("}"))}
      else
	 if {Tk.returnInt 'catch'(v("{") b([Obj Msg]) v("}"))}==1 then
	    {Exception.raiseError qtk(execFailed Obj Msg)}
	 end
      end
   end

   {Tk.send set(v 0)} % Initialize the v variable in Tcl/Tk

   proc{ReturnTk Obj M Type}
      N={LastInt M}
      if N==0 then {Exception.raiseError qtk(missingParameter 1 unknown M)} end
      Err={CheckType free M.N}
   in
      if Err==unit then skip else
	 {Exception.raiseError qtk(typeError M.N Obj.widgetType Err M)}
      end
      case {Tk.return set(v("e [catch {set v [")
			  b([Obj {Record.subtract M N}])
			  v(']}];set e "$e $v"'))}
      of 49|32|_ then {Exception.raiseError qtk(execFailed Obj M)}
      [] 48|32|R then M.N={ConvertToType R Type}
      end
   end
   
   fun{CheckType Type V}
      %
      % This function checks the type of the variable V and returns
      % either unit if type is correct or a string that is a description of the
      % required type
      %
      if {IsList Type} then
	 if {List.member V Type} then
	    unit
	 else
	    {VirtualString.toAtom {List.foldL Type
				   fun{$ I Z}
				      if I==nil then Z else
					 I#", "#Z
				      end
				   end
				   nil}}
	 end
      else
	 case Type
	 of no then
	    unit
	 [] auto(Cmd) then % the magical type : issue the command, catching the Tk error
	    Ok
	 in
	    try
	       if {Tk.returnInt 'catch'(v("{") Cmd v("}"))}==0
	       then Ok=unit end
	    catch _ then skip end
	    if {IsFree Ok} then
	       "A valid type for that parameter."
	    else
	       unit
	    end
	 [] relief then
	    {CheckType [raised sunken flat ridge solid groove] V}
	 [] anchor then
	    {CheckType [n ne e se s sw w nw center] V}
	 [] nswe then
	    if {List.all {VirtualString.toString V}
		fun{$ C}
		   {List.member C "nswe"}
		end}
	    then
	       unit
	    else
	       "Any combination of n, s, w and e"
	    end
	 [] pixel then
	    S={VirtualString.toString V}
	    Conv
	    E1 E2
	 in
	    if {List.member {List.last S} "cmip"} then
	       Conv={List.take S {Length S}-1}
	    else
	       Conv=S
	    end
	    try
	       _={String.toInt Conv}
	    catch _ then E1=unit end
	    try
	       _={String.toFloat Conv}
	    catch _ then E2=unit end
	    if {IsFree E1} orelse {IsFree E2} then unit else
	       "A screen distance (an integer or a pair int#c, int#m , int#i and int#p)"
	    end
	 [] free then
	    if {IsFree V} then unit else
	       "A free variable"
	    end
	 [] vs then
	    if {VirtualString.is V} then unit else
	       "A virtual string"
	    end
	 [] color then
	    if ({Atom.is V} andthen {Tk.returnInt 'catch'(v("{ winfo rgb . ") V v("}"))}==0)
	       orelse ({Record.is V}
		       andthen {Label V}==c
		       andthen {Record.arity V}==[1 2 3]
		       andthen {List.all [1 2 3]
				fun{$ I}
				   {HasFeature V I} andthen {Int.is V.I}
				   andthen V.I>=0 andthen V.I=<255
				end})
	    then unit
	    else "A color (either an atom that is a valid color or a record c(RR GG BB) where RR, GG and BB are integers between 0 and 255)"
	    end
	 [] cursor then
	    Ok
	 in
	    lock Lock then
	       try
		  if {Tk.returnInt 'catch'(v("{. configure -cursor") V
					   v("}"))}==0
		  then Ok=unit end
	       catch _ then skip end
	    end
	    if {IsFree Ok} then
	       "An atom that represents a valid cursor."
	    else
	       unit
	    end
	 [] font then
	    Ok
	 in
	    try
	       if {Tk.returnInt 'catch'(v("{font metrics ") V v("-fixed }"))}==0
	       then Ok=unit end
	    catch _ then skip end
	    if {IsFree Ok} then
	       "A font (a virtualstring representing a valid font or a font object)"
	    else
	       unit
	    end
	 [] bitmap then
	    Ok
	 in
	    lock Lock then
	       try
		  if {Tk.returnInt 'catch'(v("{label .testlabel -bitmap ") V
					   v(" ; destroy .testlabel }"))}==0
		  then Ok=unit end
	       catch _ then skip end
	    end
	    if {IsFree Ok} then
	       "A bitmap object or an atom that is a valid predefined bitmap"
	    else
	       unit
	    end
	 [] image then
	    Ok
	 in
	    lock Lock then
	       try
		  if {Tk.returnInt 'catch'(v("{label .testlabel -image ") V
					   v(" ; destroy .testlabel }"))}==0
		  then Ok=unit end
	       catch _ then skip end
	    end
	    if {IsFree Ok} then
	       "An image object"
	    else
	       unit
	    end
	 [] atom then
	    if {Atom.is V} then unit else
	       "An atom"
	    end
	 [] boolean then
	    if V==false orelse V==true then unit else
	       "A boolean (true or false)"
	    end
	 [] float then
	    if {Float.is V} orelse {Int.is V} then unit else
	       "A float value"
	    end
	 [] natural then
	    if {Int.is V} andthen V>=0 then unit else
	       "An integer value >= 0"
	    end
	 [] int then
	    if {Int.is V} then unit else
	       "An integer value"
	    end
	 [] integer then
	    if {Int.is V} then unit else
	       "An integer value"
	    end
	 [] action  then
	    Ok
	 in
	    if {IsFree V} then skip % can't determine type for now
	    elseif {Procedure.is V} then skip
	    elsecase V
	    of A#_ then
	       if {IsFree A} then skip % in much cases this is determined later
	       elseif A==toplevel then skip
	       elseif A==widget then skip
	       elseif {Port.is A} then skip
	       elseif {Object.is A} then skip
	       else
		  Ok=unit
	       end
	    else Ok=unit
	    end
	    if {IsFree Ok} then
	       unit
	    else
	       "A command (a procedure or a pair object#method, port#message, toplevel#method, widget#method)"
	    end
	 [] scrollregion then
	    if (V==q) orelse
	       ({Record.is V}
		andthen {Label V}==q
		andthen {Record.arity V}==[1 2 3 4]
		andthen {List.all [1 2 3 4]
			 fun{$ I}
			    {HasFeature V I} andthen ({Int.is V.I} orelse {Float.is V.I})
			 end})
	    then
	       unit
	    else
	       "A scrollregion (a record of the form q(X1 Y1 X2 Y2) where Xi and Yj are integers or floats)"
	    end
	 [] list then
	    if {List.is V} then unit else
	       "A list"
	    end
	 [] listVs then
	    if {List.is V} andthen {List.all V VirtualString.is} then unit else
	       "A list of virtual strings"
	    end
	 [] listBoolean then
	    if {List.is V} andthen {List.all V fun{$ I} I==true orelse I==false end} then unit else
	       "A list of booleans"
	    end
	 else
	    {Exception.raiseError qtk(custom "Internal Error" "Requested type "#Type#" is unkown.")}
	    unit
	 end
      end
   end
   
   
   %%
   %% Error Formatter
   %%
   
   {Error.registerFormatter qtk
    fun {$ E}
       T = 'Error: QTk module'
    in
       case E
       of qtk(badParameter P O I) then
	  error(kind:T
		msg:'Invalid parameter'
		items:[hint(l:'Parameter'
			    m:oz(P))
		       hint(l:'Widget type'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(I))
		      ])
       [] qtk(missingParameter P O I) then
	  error(kind:T
		msg:'Missing parameter'
		items:[hint(l:'Parameter'
			    m:oz(P))
		       hint(l:'Widget type'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(I))
		      ])
       [] qtk(unsettableParameter P O I) then
	  error(kind:T
		msg:'This parameter can only be set at creation time'
		items:[hint(l:'Parameter'
			    m:oz(P))
		       hint(l:'Widget type'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(I))
		      ])
       [] qtk(ungettableParameter P O I) then
	  error(kind:T
		msg:'This parameter can not be get its value'
		items:[hint(l:'Parameter'
			    m:oz(P))
		       hint(l:'Widget type'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(I))
		      ])
       [] qtk(typeError P O Error I) then
	  error(kind:T
		msg:"Incorrect Type"
		items:[hint(l:'Parameter'
			    m:oz(P))
		       hint(l:'Expected type'
			    m:Error)
		       hint(l:'Widget type'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(I))
		      ])
       [] qtk(panelObject P M) then
	  error(kind:T
		msg:"Object not in panel"
		items:[hint(l:'Object'
			    m:oz(P))
		       hint(l:'Widget type'
			    m:oz(panel))
		       hint(l:'Operation'
			    m:oz(M))
		      ])
       [] qtk(invalidAction P) then
	  error(kind:T
		msg:"An action is defined with an invalid format"
		items:[hint(l:'Action'
			    m:oz(P))])
       [] qtk(custom M W I) then
	  error(kind:T
		msg:M
		items:[hint(l:'Description'
			    m:W)
		       hint(l:'Operation'
			    m:oz(I))])
       [] qtk(custom M W) then
	  error(kind:T
		msg:M
		items:[hint(l:'Description'
			    m:W)])
       [] qtk(badWidget W) then
	  error(kind:T
		msg:"Invalid widget"
		items:if W==nil then
			 [hint(l:'Widget type'
			       m:oz({Label W}))]
		      else
			 [hint(l:'Widget type'
			       m:oz({Label W}))
			  hint(l:'Operation'
			       m:oz(W))
			 ]
		      end)
       [] qtk(execFailed O M) then
	  error(kind:T
		msg:"Error while executing a command"
		items:[hint(l:'Object'
			    m:oz(O))
		       hint(l:'Operation'
			    m:oz(M))])
       [] qtk(other) then
	  error(kind:T
		msg:"Unkown error")
       end
    end}

   %%
   %% Assertion stuff for checking parameter types
   %%
   
   AssertLevel={NewCell assert(Init:full set:full get:full all:full)}

   proc{SetAssertLevel What Level}
      if (What==all) andthen {List.member Level [full partial none]} then
	 {Assign AssertLevel assert(Init:Level set:Level get:Level all:Level)}
      else
	 if {List.member What [set get]} andthen
	    {List.member Level [full partial none]} then
	    {Assign AssertLevel {Record.adjoinAt {Access AssertLevel}
				 What Level}}
	 elseif What==init andthen
	    {List.member Level [full partial none]} then
	    {Assign AssertLevel {Record.adjoinAt {Access AssertLevel}
				 Init Level}}
	 else
	    {Exception.raiseError qtk(custom "Illegal AssertLevel" "Can only assert init, set and get to level full, partial or none" What#Level)}
	 end
      end
   end

   proc{Assert Widget TypeInfo Rec}
      if TypeInfo\=unit then % no type information => bypass test
	 Op={Label Rec}
	 Level={Access AssertLevel}.Op
      in
	 if Level==full orelse Level==partial then % checks the type
	    {Record.forAllInd Rec
	     proc{$ I V}
		if {HasFeature TypeInfo.all I} then % the type is known
		   case Op
		   of !Init then
		      if {HasFeature TypeInfo.uninit I} then % the type can't be init
			 {Exception.raiseError qtk(badParameter I Widget Rec)}
		      end
		      if Level==full then % checks the type
			 Err={CheckType TypeInfo.all.I V}
		      in
			 if Err==unit then skip else
			    {Exception.raiseError qtk(typeError I Widget Err Rec)}
			 end
		      end
		   [] set then
		      if {HasFeature TypeInfo.unset I} then % the type can't be set
			 {Exception.raiseError qtk(unsettableParameter I Widget Rec)}
		      end
		      if Level==full then % checks the type
			 Err={CheckType TypeInfo.all.I V}
		      in
			 if Err==unit then skip else
			    {Exception.raiseError qtk(typeError I Widget Err Rec)}
			 end
		      end	      
		   [] get then
		      if {HasFeature TypeInfo.unget I} then % the type can't be get
			 {Exception.raiseError qtk(ungettableParameter I Widget Rec)}
		      end
		      if Level==full then % checks the type (always free here)
			 Err={CheckType free V}
		      in
			 if Err==unit then skip else
			    {Exception.raiseError qtk(typeError I Widget Err Rec)}
			 end
		      end	      
		   end
		else
		   {Exception.raiseError qtk(badParameter I Widget Rec)}
		end
	     end}
	 end
      end
   end

   %%
   %% Class definitions
   %%

   ToolTipsDelay=1000
   ToolTipsDisappearDelay=250
   TkToolTips
   ActiveTooltips={NewCell true}

   proc{SetToolTips B}
      if B==true orelse B==false then
	 {Assign ActiveTooltips B}
      else
	 {Exception.raiseError qtk(custom "Unable to enable/disable tooltips" "The parameter must be either true or false")}
      end
   end
   
   local
      Out
      ToolTipPort={NewPort Out}
      Last={NewCell nil}
      proc{Loop L}
	 S={Access Last}\=nil
      in
	 case L of Z|Zs then
	    if {Access ActiveTooltips} then
	       case Z
	       of enter(Obj X Y) then
		  if S then
		     if {Access Last}==Obj then skip
		     else
			{{Access Last} remove}
			{Obj draw(X Y)}
			{Assign Last Obj}
		     end
		  else
		     Chrono
		  in
		     thread
			{Delay ToolTipsDelay}
			Chrono=unit
		     end
		     {WaitOr Chrono Zs}
		     if {IsFree Chrono} then
			skip
		     else
			{Assign Last Obj}
			{Obj draw(X Y)}
		     end
		  end
	       [] leave(_) then
		  if S then
		     Chrono
		  in
		     {{Access Last} remove}
		     thread
			{Delay ToolTipsDisappearDelay}
			Chrono=unit
		     end
		     {WaitOr Chrono Zs}
		     if {IsFree Chrono} then
			skip
		     else
			{Assign Last nil}
		     end
		  end
	       end
	    else
	       if {Access Last}\=nil then
		  {{Access Last} remove}
		  {Assign Last nil}
	       else skip end
	    end
	    {Loop Zs}
	 end
      end
      thread
	 {Loop Out}
      end
   in
      class TkToolTips
	 
	 feat parent
	    Toolwin:{NewCell nil}
	    Message:{NewCell nil}
	    Lock:{NewLock}
	    
	 attr text shown
	    
	 meth init(parent:P text:T)
	    lock self.Lock then
	       self.parent=P
	       text<-T
	       shown<-false
	       thread
		  {P tkBind(event:"<Enter>" args:[int(x) int(y)]
			    action:ToolTipPort#enter(self))}
		  {P tkBind(event:"<Motion>" args:[int(x) int(y)]
			    action:ToolTipPort#enter(self))}
		  {P tkBind(event:"<Leave>" action:ToolTipPort#leave(self))}
	       end
	    end
	 end
	 
	 meth unBindedInit(parent:P text:T)
	    lock self.Lock then
	       self.parent=P
	       text<-T
	       shown<-false
	    end
	 end
	 
	 meth enter(X Y)
	    {Send ToolTipPort enter(self X Y)}
	 end
	 
	 meth leave
	    {Send ToolTipPort leave(self)}
	 end
	 
	 meth set(T)
	    lock self.Lock then
	       text<-T
	       if @shown then
		  try
		     {{Access self.Message} tk(configure text:@text)}
		  catch _ then skip end
	       else skip end
	    end
	 end
	 
	 meth get(T)
	    lock self.Lock then
	       T=@text
	    end
	 end
      
	 meth draw(MX MY)
	    lock self.Lock then
	       try
		  WX={Tk.returnInt winfo(rootx self.parent)} % +{Tk.returnInt winfo(width self.parent)}
		  WY={Tk.returnInt winfo(rooty self.parent)}
		  H={Tk.returnInt winfo(height self.parent)}
		  X Y
		  M
		  T
	       in
		  if {Access self.Toolwin}==nil then
		     T={New Tk.toplevel tkInit(withdraw:true bg:black width:1 height:1
					       visual:{Tk.return winfo(visual self.parent)}
					       colormap:self.parent)}
		     M={New Tk.message tkInit(parent:T text:@text
					      bg:'#e4e2bc' aspect:800
					      font:'helvetica 8')}
		     {Tk.send pack(M padx:1 pady:1)}
		     {Assign self.Toolwin T}
		     {Assign self.Message M}
		  else
		     T={Access self.Toolwin}
		     M={Access self.Message}
		     {M tk(configure text:@text)}
		  end
		  if MX>64 then X=WX+MX else X=WX end
		  if {Abs H-MY}>64 then Y=WY+MY+16 else Y=WY+H end
		  {Tk.batch [wm(overrideredirect T true)
			     wm(geometry T '+'#{IntToString (X+4)}#'+'#{IntToString (Y+2)})
			     wm(deiconify T)
			     wm(geometry T '+'#{IntToString (X+4)}#'+'#{IntToString (Y+2)})]}
	       catch _ then skip end
	       shown<-true
	    end
	 end
	 
	 meth remove
	    lock self.Lock then
	       if {Access self.Toolwin}\=nil then
		  try
		     {Tk.send wm(withdraw {Access self.Toolwin})}
		  catch _ then skip end
	       end
	       shown<-false
	    end
	 end
	 
	 meth hide
	    lock self.Lock then
	       try
		  {{Access self.Toolwin} tkClose}
	       catch _ then skip end
	       {Assign self.Toolwin nil}
	       shown<-false
	    end
	 end
	 
      end
   end
   
   class SetGet % bare set and get functionalitites

      prop locking
	 
      meth set(...)=M
	 lock
	    {ExecTk self {Record.adjoin M configure}}
	 end
      end

      meth get(...)=M
	 lock
	    L={Record.toListInd M}
	    Type
	 in
	    {ForAll L
	     proc{$ R}
		case R
		of type#X then Type=X
		else skip end
	     end}
	    if {IsFree Type} then Type=no else skip end
	    {ForAll L
	     proc{$ R}
		case R
		of type#_ then skip
		[] X#Y then
		   {ReturnTk self cget("-"#X Y) Type}
		end
	     end}
	 end
      end
   
      meth return(...)=M
	 lock
	    L
	    Type
	    Ret
	    Return
	    InLabel
	    InList
	    ReturnNu
	 in
	    L={List.filter
	       {Record.toListInd M}
	       fun{$ R}
		  case R
		  of type#X then
		     Type=X
		     false
		  else true end
	       end}
	    if {IsFree Type} then Type=no else skip end
	    ReturnNu={List.foldL L
		      fun{$ Z R}
			 case R of Nu#F then
			    if {Int.is Nu} andthen Nu>Z andthen {IsFree F} then Nu else Z end
			 else Z end
		      end
		      0}
	    if ReturnNu<2 then raise error(incorrectReturnVariable) end else skip end
	    InList={List.filterInd L
		    fun{$ I R}
		       case R
		       of 1#X then
			  InLabel=X
			  false
		       [] !ReturnNu#V then
			  Return=V
			  false
		       else true
		       end
		    end}
	    {self tkReturn({List.toRecord InLabel InList} Ret)}
	    Return={ConvertToType Ret Type}
	    {Wait Return}
	 end
      end

      meth exec(...)=M
	 lock
	    {self {Record.adjoin M tk}}
	 end
      end

   end

   class QTkAction % QTk action class

      prop locking

      attr Action

      feat !Toplevel Parent
      
      meth init(parent:P action:A<=proc{$} skip end)
	 lock
	    self.Parent=P
	    self.Toplevel=P.toplevel
	    QTkAction,set(A)
	 end
      end

      meth action(A)
	 A=self.Toplevel.port#r(self execute)
      end
   
      meth set(A)
	 lock
	    Action<-A
	 end
      end

      meth get(A)
	 lock
	    A=@Action
	 end
      end

      meth execute(...)=M
	 lock
	    Err
	    fun{Adjoin Xs}
	       if M==execute then Xs else
		  Max={List.foldR
		       {Arity Xs} fun{$ N Old}
				     if {IsInt N} andthen N>Old then N else Old end
				  end 0}
	       in
		  {Record.adjoin
		   {List.toRecord r
		    {List.mapInd
		     {Record.toList M}
		     fun{$ I E} I+Max#E end}}
		   Xs}
	       end
	    end
	 in
	    if {Procedure.is @Action} then
	       {Procedure.apply @Action {Record.toList M}}
	    else
	       case @Action
	       of widget#Xs then {self.Parent {Adjoin Xs}}
	       [] toplevel#Xs then {self.Toplevel {Adjoin Xs}}
	       [] X#Xs then
		  if {Object.is X} then {X {Adjoin Xs}}
		  elseif {Port.is X} then
		     {Send X {Adjoin Xs}}
		  else
		     Err=unit
		  end
	       else
		  Err=unit
	       end
	    end
	    if {IsDet Err} then
	       {Exception.raiseError qtk(invalidAction @Action)}
	    end
	 end
      end
   
   end

%    fun{FindEntry DB DBL K} % find entry under key K
      
%       if {Dictionary.member DB K} then
% 	 DB.K
%       else
% 	 V
%       in
% 	 {Dictionary.put DB K V}
% 	 {Assign DBL K#V|{Access DBL}}
% 	 V
%       end
%    end
   
%    fun{FindKey DB DBL V} % find key with entry V
%       S
%       fun{Loop L R}
% 	 case R
% 	 of K#N|Xs then
% 	    if {IsDet N} andthen N==V then
% 	       %% bring this pair in front of the DBL list so that next search is fast
% 	       L=Xs
% 	       {Assign DBL K#V|S}
% 	       K
% 	    else
% 	       %% puts X on the left list and loops with next item
% 	       E
% 	    in
% 	       L=K#N|E
% 	       {Loop E Xs}
% 	    end
% 	 else
% 	    %% key not already existing : create one
% 	    K={NewName}
% 	 in
% 	    {Dictionary.put DB K V}
% 	    {Assign DBL K#V|{Access DBL}}
% 	    K
% 	 end
%       end
%    in
%       {Loop S {Access DBL}}
%    end

   class QTkClass % QTk mixin class 

      prop locking

      from SetGet

      feat
	 !Builder
	 ToolTip
	 widgetType:unknown
	 tooltipsAvailable:true
	 toplevel
	 parent
	 !EventDict
	 typeInfo:unit % different from unit means type checking is on : r(init:r unset:r unget:r)
      
      meth !Init(...)=M
	 lock
	    self.parent=M.parent
	    self.toplevel=M.parent.toplevel
	    if {IsFree self.Builder} then
	       self.Builder=self.parent.Builder
	    end
	    self.EventDict={NewDictionary}
	    {Assert self.widgetType self.typeInfo M}
	    if {HasFeature self action} then % action widget
	       self.action={New QTkAction init(parent:self action:{CondSelect M action proc{$} skip end})}
	    end
	    if self.tooltipsAvailable==true then % this widget has got a tooltips
	       {self SetToolTip(M)}
	    end
	 end
      end

      meth set(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    if {HasFeature self action} andthen {HasFeature M action} then
	       {self.action set(M.action)}
	    end
	    if self.tooltipsAvailable andthen {HasFeature M tooltips} then
	       {self SetToolTip(M)}
	    end
	    SetGet,{Subtracts M [action tooltips]}
	 end
      end

      meth get(...)=M
	 lock
	    {Assert self.widgetType self.typeInfo M}
	    if {HasFeature self action} andthen {HasFeature M action} then
	       {self.action get(M.action)}
	    end
	    if self.tooltipsAvailable andthen {HasFeature M tooltips} then
	       M.tooltips=if {IsFree self.ToolTip} then
			     ""
			  else
			     {self.ToolTip get($)}
			  end
	    end
	    if {HasFeature M tk} then M.tk=self end
	    if self.typeInfo==unit then
	       SetGet,{Subtracts M [action tooltips blackbox tk]}
	    else
	       {Record.forAllInd {Subtracts M [action tooltips blackbox]}
		proc{$ I R}
		   SetGet,get(I:R type:self.typeInfo.all.I)
		end}
	    end
	 end
      end

      meth SetToolTip(M)
	 lock
	    if {HasFeature M tooltips} then
	       if {IsFree self.ToolTip} then
		  self.ToolTip={New TkToolTips init(parent:self text:M.tooltips)}
	       else
		  {self.ToolTip set(M.tooltips)}
	       end
	    else skip end
	 end
      end
      
      meth bind(action:A<=NoArgs
		event:E
		append:P<=false
		StoreInEventDict:S<=true
		args:G<=nil)
	 lock
	    Event={VirtualString.toAtom E}
	 in
	    if A==NoArgs then
	       {self tkBind(event:Event
			    args:G)}
	       {Dictionary.remove self.EventDict Event}
	    else
	       Act={{New QTkAction init(parent:self action:A)} action($)}
	    in
	       {self tkBind(event:Event
			    action:Act
			    append:P
			    args:G)}
	       if S then
		  if P then
		     {Dictionary.put self.EventDict Event
		      Act|{Dictionary.condGet self.EventDict Event nil}}
		  else
		     {Dictionary.put self.EventDict Event
		      Act|nil}
		  end
	       end
	    end
	 end
      end

      meth getFocus(force:F<=false)
	 lock
	    {ExecTk focus if F then o("-force" self) else o(self) end}
	 end
      end

      meth setGrab(global:G<=false)
	 lock
	    {ExecTk grab if G then o("-global" self) else o(self) end}
	 end
      end
   
      meth releaseGrab
	 lock
	    {ExecTk grab o(release self)}
	 end
      end
   
      meth getGrabStatus(G)
	 lock
	    {ReturnTk grab o(status self G) atom}
	 end
      end

      meth 'raise'(1:W<=NoArgs)
	 lock
	    if W==NoArgs then
	       {ExecTk 'raise' o(self)}
	    else
	       {ExecTk 'raise' o(self W)}
	    end
	 end
      end

      meth lower(1:W<=NoArgs)
	 lock
	    if W==NoArgs then
	       {ExecTk lower o(self)}
	    else
	       {ExecTk lower o(self W)}
	    end
	 end
      end
      
      meth winfo(...)=M
	 lock
	    R=r(cells:natural
		colormapfull:boolean
		depth:natural
		fpixels:exception %%%%%%%%%
		geometry:exception %%%%%%%%%%%
		height:natural
		id:no
		ismapped:boolean
		name:no
		parent:no
		pixels:exception %%%%%%%%%
		pointerx:natural
		pointery:natural
		pointerxy:exception %%%%%%
		reqheight:natural
		reqwidth:natural
		rgb:exception %%%%%
		rootx:natural
		rooty:natural
		screen:no
		screencells:natural
		screendepth:natural
		screenheight:natural
		screenmmheight:natural
		screenmmwidth:natural
		screenvisual:atom
		screenwidth:natural
		server:no
		toplevel:exception %%%%%%%
		viewable:boolean
		visual:atom
		visualid:no
		visualsavailable:exception %%%%%%%
		vrootheight:natural
		vrootwidth:natural
		vrootx:natural
		vrooty:natural
		width:natural
		x:natural
		y:natural)
	 
	 in
	    {Record.forAllInd M
	     proc{$ I V}
		if {HasFeature R I} then % parameter is correct
		   Err={CheckType free V}
		in
		   if Err==unit then skip
		   else
		      {Exception.raiseError qtk(typeError I self.widgetType Err M)}
		   end
		   V=if R.I==exception then % special parameters
			case I
			of fpixels then % function as still one parameter is missing
			   fun{$ P}
			      Err={CheckType pixel P}
			   in
			      if Err==unit then skip
			      else
				 {Exception.raiseError qtk(typeError I self.widgetType Err M)}
			      end
			      {ConvertToType {Tk.return winfo(I self P)} float}
			   end
			[] geometry then
			   {List.toRecord geometry
			    {List.mapInd
			     {SplitGeometry {Tk.return winfo(I self)}}
			     fun{$ I V}
				case I
				of 1 then width
				[] 2 then height
				[] 3 then x
				[] 4 then y
				end#V
			     end}}
			[] pixels then
			   fun{$ P}
			      Err={CheckType pixel P}
			   in
			      if Err==unit then skip
			      else
				 {Exception.raiseError qtk(typeError I self.widgetType Err M)}
			      end
			      {ConvertToType {Tk.return winfo(I self P)} natural}
			   end
			[] pointerxy then
			   {List.toRecord pointerxy
			    {List.mapInd
			     {Split {Tk.return winfo(I self)}}
			     fun{$ I V}
				case I
				of 1 then x
				[] 2 then y
				end#{String.toInt V}
			     end}}
			[] rgb then
			   fun{$ P}
			      Err={CheckType color P}
			   in
			      if Err==unit then skip
			      else
				 {Exception.raiseError qtk(typeError I self.widgetType Err M)}
			      end
			      {List.toRecord rgb
			       {List.mapInd
				{Split {Tk.return winfo(I self P)}}
				fun{$ I V}
				   case I
				   of 1 then red
				   [] 2 then green
				   [] 3 then blue
				   end#({String.toInt V} div 256)
				end}}
			   end
			[] toplevel then self.toplevel
			[] visualsavailable then
			   {List.map
			    {Split {Tk.return winfo(I self includeids)}}
			    fun{$ Visual}
			       {List.toRecord visual
				{List.mapInd
				 {Split Visual}
				 fun{$ I V}
				    case I
				    of 1 then visual#{ConvertToType V atom}
				    [] 2 then depth#{ConvertToType V natural}
				    [] 3 then id#V
				    end
				 end}}
			    end}
			end
		     else
			{ConvertToType {Tk.return winfo(I self)} R.I}
		     end
		else
		   {Exception.raiseError qtk(badParameter I self.widgetType M)}
		end
	     end}
	 end
      end


      meth close
	 lock
	    try
	       {self destroy}
	    catch _ then skip end
	    {Tk.send destroy(self)}
	 end
      end

      meth destroy
	 lock
	    skip
	 end
      end
   end

   fun{WInfo I}
      R
      What=winfo(I)
   in
      R=case {Label I}
	of cells then {Tk.returnInt What}
	[] colormapfull then {Tk.return What}=="1"
	[] depth then {Tk.returnInt What}
	[] exist then {Tk.return What}=="1"
	[] fpixels then {Tk.returnFloat What}
	[] height then {Tk.returnInt What}
	[] ismapped then {Tk.return What}=="1"
	[] pixels then {Tk.returnInt What}
	[] pointerx then {Tk.returnInt What}
	[] pointery then {Tk.returnInt What}
	[] reqheight then {Tk.returnInt What}
	[] reqwidth then {Tk.returnInt What}
	[] rootx then {Tk.returnInt What}
	[] rooty then {Tk.returnInt What}
	[] screen then {Tk.return What}
	[] screencells then {Tk.returnInt What}
	[] screendepth then {Tk.returnInt What}
	[] screenheight then {Tk.returnInt What}
	[] screenmmheight then {Tk.returnInt What}
	[] screenmmwidth then {Tk.returnInt What}
	[] screenvisual then {Tk.returnAtom What}
	[] screenwidth then {Tk.returnInt What}
	[] server then {Tk.return What}
	[] viewable then {Tk.return What}=="1"
	[] visual then {Tk.returnAtom What}
	[] visualid then {Tk.return What}
	[] vrootwidth then {Tk.returnInt What}
	[] vrootheight then {Tk.returnInt What}
	[] vrootx then {Tk.returnInt What}
	[] vrooty then {Tk.returnInt What}
	[] width then {Tk.returnInt What}
	[] x then {Tk.returnInt What}
	[] y then {Tk.returnInt What}
	end
      {Wait R}
      R
   end
   
   
%    Widgets={NewDictionary}

%    fun{GetWidget M}
%       P={Dictionary.condGet Widgets M r(object:nil)}.object
%    in
%       if P==nil then
% 	 E
%       in
% 	 try
% 	    E={QTkRegisterWidget M $}.{VirtualString.toAtom qTk#{Majus M}}
% 	 catch _ then
% 	    {Exception.raiseError qtk(custom "Unable to register a widget" "Missing or incorrect widget name" M)}
% 	    E=nil
% 	 end
% 	 E
%       else
% 	 P
%       end
%    end

%   fun{MakeClass ClassName Description}
%      {Class.new [ClassName] q
%       {Record.map
%	{Record.filter Description
%	 fun{$ V}
%	    {IsDet V} andthen {IsRecord V} andthen {HasFeature V feature}
%	 end}
%	fun{$ V}
%	   V.feature
%	end}
%       [locking]}
%   end
      
%   fun{NewFeat Class Desc}
%      {New {MakeClass Class Desc} Desc}
%   end
   
%    fun{MakeClass ClassName Description}
%       {Class.new [ClassName] q
%        {Record.map
% 	{Record.filter Description
% 	 fun{$ V}
% 	    {IsDet V} andthen {IsRecord V} andthen {HasFeature V feature}
% 	 end}
% 	fun{$ V}
% 	   V.feature
% 	end}
%        [locking]}
%    end

%    fun{GetLook Rec}
%       {{CondSelect Rec look DefLook}.get Rec}
%    end
   
%    fun{NewFeat Class Desc}
%       {New {MakeClass Class Desc} {PropagateLook Desc}}
%    end

%    fun{PropagateLook Rec}
%       Look={CondSelect Rec look DefLook}
%    in
%       {GetLook
%        {Record.mapInd Rec
% 	fun{$ I V}
% 	   if {IsInt I} andthen {IsDet V} andthen {IsRecord V} then
% 	      L=if {HasFeature V look}==false then
% 		   {Record.adjoinAt V look Look}
% 		else
% 		   V
% 		end
% 	   in
% 	      {GetLook L}
% 	   else
% 	      V
% 	   end
% 	end}}
%    end
   

%    TkClass =
%    {List.last
%     {Arity
%      {GetClass
%       {New class $ from Tk.frame meth init skip end end init}}
%      . OoFeat}}
   
%    fun{TkLoad FileName TkName}
%       {ExecTk load FileName}
%       class $
% 	 from Tk.frame
% 	 feat !TkClass:TkName
%       end
%    end

%    fun{TkLoadPI FileName TkName}
%       P={Property.get 'platform'}.os
%    in
%       {TkLoad
%        FileName#"-"#P#if P==win32 then ".dll" else ".so" end
%        TkName}
%    end
 
%    fun{RegisterLoadTkWidget TkClass TkName}
%       QTkName={VirtualString.toAtom qTk#{Majus TkName}}
%       class Temp
% 	 feat widgetType:TkName
% 	 from QTkClass TkClass
% 	 meth !TkName(...)=M
% 	    QTkClass,{Record.adjoin M init}
% 	    TkClass,{TkInit M}
% 	 end
% 	 meth otherwise(M)
% %	    {ExecTk self M}
% 	    {self tk(M)}
% 	 end
%       end		     
%    in
%       {RegisterWidget r(widgetType:TkName
% 			feature:false
% 			QTkName:Temp)}
%       true
%    end
   
%    fun{LoadTk FileName TkName}
%       try
% 	 {RegisterLoadTkWidget {TkLoad FileName TkName} TkName}
%       catch _ then false end
%    end
   
%    fun{LoadTkPI FileName TkName}
%       try
% 	 {RegisterLoadTkWidget {TkLoadPI FileName TkName} TkName}
%       catch _ then false end
%    end

   Feature={NewName}
   ParentFeature={NewName}
   
   fun{MakeClass ClassName Description}
      fun{Loop J L}
	 case L
	 of I#V|Ls then
	    if {IsInt I} andthen {IsDet V} andthen {IsRecord V} then
	       fun{ILoop M}
		  case M of N#O|Ms then
		     N#O|{ILoop Ms}
		  else
		     if {HasFeature V feature} then
			J#V.feature|{Loop J+1 Ls}
		     else
			{Loop J Ls}
		     end
		  end
	       end
	    in
	       {ILoop {CondSelect V ParentFeature nil}}
	    else
	       {Loop J Ls}
	    end
	 else
	    nil
	 end
      end
      Features={List.toRecord q
		{List.append
		 {CondSelect Description Feature nil}
		 {Loop 1 {Record.toListInd Description}}}}
   in
      {Class.new [ClassName] q Features [locking]}
   end

   fun{ApplyLook R L}
      Look={CondSelect R look L}
   in
      {Record.adjoinAt {Look.get R} look Look}
   end
   
   class BuilderClass
      feat
	 build
	 buildMigratable
	 register
	 setAlias
	 unSetAlias
	 getAlias
	 defaultLook
	 getWidgetList
	 getAliasList
	 Widgets
	 Aliases
	 Resources
	 ResourceClass
      meth !Init(GetToplevelClass GetMigratableClass)
	 ToplevelClass    = {GetToplevelClass self}
	 MigratableClass  = {GetMigratableClass self}
      in
	 self.register    = proc{$ W} {self Register(W)} end
	 self.getWidgetList  = fun{$} {List.filter
				       {Dictionary.keys self.Widgets}
				       fun{$ K} {Atom.is K} end}
			       end
	 self.getAliasList   = fun{$} {Dictionary.keys self.Aliases} end
	 self.defaultLook = {NewLook}
	 local
	    fun{Build Class Desc}
	       fun{Loop Desc1}
		  Alias={Dictionary.condGet self.Aliases {Label Desc1} unit}
		  Desc={ApplyLook Desc1 self.defaultLook}
	       in
		  if Alias==unit then
		     Exp={self Expand(Desc $)}
		     R={self NewFeat({MakeClass Class Exp} Init(Exp) $)}
		  in
		     R
		  else
		     {Loop {Alias Desc}}
		  end
	       end
	    in
	       {Loop Desc}
	    end
	 in
	    self.build = fun{$ Desc} {Build ToplevelClass Desc} end
	    self.buildMigratable= fun{$ Desc} {Build MigratableClass Desc} end
	 end
	 self.Widgets     = {NewDictionary}
	 self.Aliases     = {NewDictionary}
	 self.setAlias    = proc{$ A R} {self SetAlias(A R)} end
	 self.unSetAlias  = proc{$ A}
			       {Dictionary.remove self.Aliases A}
			    end
	 self.getAlias    = fun{$ A}
			       self.Aliases.A
			    end
%	 self.Resources={NewDictionary}
	 self.Resources  = {WeakDictionary.new _}
	 {WeakDictionary.close self.Resources}
	 self.ResourceClass={NewDictionary}
      end
      meth Expand(R $)
	 Look={CondSelect R look self.defaultLook}
      in
	 {Record.mapInd R
	  fun{$ I V}
	     if {IsInt I} andthen {IsDet V} andthen {IsRecord V} then
		fun{Loop V}
		   L1={ApplyLook V Look}
		   Alias={Dictionary.condGet self.Aliases {Label L1} unit}
		in
		   if Alias==unit then
		      L1
		   else
		      {Loop {Alias L1}}
		   end
		end
	     in
		{Loop V}
	     else
		V
	     end
	  end}
      end
      meth SetAlias(A R)
	 if {Dictionary.member self.Widgets A} then
	    {Exception.raiseError qtk(custom "Can't set an alias to using a regular widget name." A setAlias(A R))}
	 end
	 if {Record.is R} then
	    {self.setAlias A
	     fun{$ M} {Record.adjoin {Record.adjoin R M} {Label R}} end}
	 elseif {Class.is R} then
	    {self.setAlias A
	     fun{$ M}
		UI
		%% remove all options this function automatically manages from the description
		PurgedM={Record.filterInd M
			 fun{$ I _}
			    {Not {List.member I [glue handle look parent feature Feature ParentFeature]}} end}
		%% look used by this description
		Look={CondSelect M look self.defaultLook}
		%% split children widgets descriptions from other options
		ChildrenWidget OtherOptions
		{Record.partitionInd PurgedM
		 fun{$ I V} {IsInt I} andthen {IsDet V} andthen {Record.is V} end
		 ChildrenWidget OtherOptions}
		%% apply look to children widgets
		LookedChildren={Record.map ChildrenWidget
				fun{$ C}
				   {ApplyLook C Look}
				end}
		%% add a handle option for children widgets that have a feature option
		HandledLookedChildren={Record.map LookedChildren
				       fun{$ C}
					  if {HasFeature C feature} then
					     Handle={CondSelect C handle _}
					  in
					     {Record.adjoinAt C handle Handle}
					  else
					     C
					  end
				       end}
		%% extract the feature options of all children widgets
		%% creating a list of pairs NameOfFeature#HandleOfCorrespondingWidget
		FeatList={Record.toList
			  {Record.map
			   {Record.filter HandledLookedChildren
			    fun{$ V}
			       {Record.is V} andthen {HasFeature V feature}
			    end}
			   fun{$ V} V.feature#V.handle end
			  }}
		%% supress the feature option of children widgets
		NoFeatHandledLookedChildren={Record.map HandledLookedChildren
					     fun{$ C}
						{Record.subtract C feature}
					     end}
		%% builds the description really given to the class
		Desc={Record.adjoinAt
		      {Record.adjoin NoFeatHandledLookedChildren OtherOptions}
		      QTkDesc UI}
		%% creates the class, adding required features parameters
		Cl={MakeClass R {Record.adjoinAt M Feature FeatList}}
		%% builds the object
		Obj={New Cl Desc}
		%% set handle and feature
		{CondSelect M handle _}=Obj
		%% adds what is required to UI
		Out1={Record.adjoin
		      r(glue:{CondSelect M glue ""}  % by default, passes the glue parameter as is
			ParentFeature:if {HasFeature M feature} then % feature parameter passed up to constructor widget
					 M.feature#Obj|{CondSelect M ParentFeature nil}
				      else nil end
			look:Look) % by default, passes the look parameter as is)
		      UI}
	     in
		if {HasFeature M parent} then
		   {Record.adjoinAt Out1 parent M.parent}
		else
		   Out1
		end
	     end}
	 elseif {Procedure.is R} then
	    self.Aliases.A:=fun{$ M}
			       if {HasFeature M parent} then
				  {Record.adjoinAt {R {Record.subtract M parent}} parent M.parent}
			       else
				  {R M}
			       end
			    end
	 else
	    {Exception.raiseError qtk(custom "Invalid alias format, expecting a record, a class or a procedure." A setAlias(A R))}
	 end	     
      end
      meth !Register(M)
	 try
	    WidgetName
	    Feat1={CondSelect M feature false}
	    Feat2=if (Feat1==scroll) orelse (Feat1==scrollfeat) then
		     WidgetName={VirtualString.toAtom "pure_"#M.widgetType}
		     if {Dictionary.member self.Aliases M.widgetType} orelse
			{Dictionary.member self.Widgets M.widgetType} then
			{Exception.raiseError qtk(custom "Error : widget already registered."
						  M.widgetType
						  M)}
		     end
		     self.Aliases.(M.widgetType):=fun{$ M1}
						     M={Record.adjoin M1 WidgetName}
						     Handle={CondSelect M handle _}
						     V H
						     LRFeat=if {CondSelect M lrscrollbar false} then
							       lrscrollbar#H|nil
							    else nil end
						     Feats=if {CondSelect M tdscrollbar false} then
							      tdscrollbar#V|LRFeat
							   else LRFeat end
						     fun{Format M}
							{Record.adjoin
							 r(Feature:Feats) % add tdscrollbar and lrscrollbar features
							 {Record.adjoinAt
							  {Record.adjoinAt
							   {Record.filterInd M
							    fun{$ I V}
							       {Not {List.member I [tdscrollbar lrscrollbar scrollwidth feature parent]}}
							    end}
							   glue nswe}
							  handle Handle}}
						     end
						     fun{Width R}
							{Record.adjoin
							 if {HasFeature R scrollwidth} then
							    r(width:R.scrollwidth)
							 elseif Win32 then
							    r
							 else
							    r(width:10)
							 end
							 R}
						     end
						     fun{Out R}
							Out=if {HasFeature M parent} then
							       {Record.adjoinAt R parent M.parent}
							    else R
							    end
						     in
							if {HasFeature M feature} then
							   {Record.adjoinAt Out
							    ParentFeature
							    M.feature#Handle|{CondSelect M Feature nil}}
							else
							   Out
							end
						     end
						     thread
							if {CondSelect M tdscrollbar false} then
							   {Wait V}
							   {Tk.addYScrollbar Handle V}
							end
							if {CondSelect M lrscrollbar false} then
							   {Wait H}
							   {Tk.addXScrollbar Handle H}
							end
						     end
						  in
						     if {CondSelect M tdscrollbar false} then
							if {CondSelect M lrscrollbar false} then
							   %% td & lr
							   {Out
							    lr({Format M} {Width tdscrollbar(handle:V glue:ns)} newline
							       {Width lrscrollbar(handle:H glue:we)}
							       glue:{CondSelect M glue ""})}
							else
							   %% td
							   {Out
							    lr({Format M} {Width tdscrollbar(handle:V glue:ns)} newline
							       glue:{CondSelect M glue ""})}
							end
						     else
							if {CondSelect M lrscrollbar false} then
							   %% lr
							   {Out
							    lr({Format M} newline
							       {Width lrscrollbar(handle:H glue:we)}
							       glue:{CondSelect M glue ""})}
							else
							   M
							end
						     end
						  end
		     Feat1==scrollfeat
		  else
		     if {Dictionary.member self.Widgets M.widgetType} then
			{Exception.raiseError qtk(custom "Error : widget already registered."
						  M.widgetType
						  M)}
		     end
		     WidgetName=M.widgetType
		     Feat1
		  end
	 in
	    {Dictionary.put self.Widgets WidgetName
	     r(feature:Feat2
	       object:M.widget)}
	    {ForAll {CondSelect M resource nil}
	     proc{$ Id#C}
		{Dictionary.put self.ResourceClass Id C}
	     end}
	 catch X then
	    {Exception.raiseError qtk(custom "Unable to register a widget"
				      {Error.extendedVSToVS {Error.messageToVirtualString {Error.exceptionToMessage X}}}
				      M)}
	 end
      end
      meth !GetSignature(Name $)
	 R
	 try
	    R={Dictionary.get self.Widgets Name}
	 catch _ then
	    {Browse {Dictionary.keys self.Widgets}}
	    R={Dictionary.get self.Widgets Name}
	 end
      in
	 if {HasFeature R sig} then
	    R.sig
	 else
	    Sig={GS.getClassInfo R.object}
	 in
	    {Dictionary.put self.Widgets Name {Record.adjoinAt R sig Sig}}
	    Sig
	 end
      end
      meth !FlattenLabel(R R2)
	 %% applies look to R
	 R1={ApplyLook R self.defaultLook}
	 %% alias ?
	 N={Label R1}
	 Alias={Dictionary.condGet self.Aliases N unit}
      in
	 R2=if Alias==unit then
	       %% no => gets the corresponding object
	       if {HasFeature R1 actionh} then
		  {Record.adjoinAt
		   {Record.subtract R1 actionh}
		   action proc{$}
			     if {Procedure.is R1.actionh} then
				{R1.actionh Object}
			     elsecase R1.actionh
			     of W#M then
				if {Object.is W} then
				   {W M(Object)}
				elseif {Port.is W} then
				   {Port.send W M(Object)}
				else
				{Exception.raiseError qtk(custom "Invalid actionh parameter"
							  R1.actionh R1)}
				end
			     else
				{Exception.raiseError qtk(custom "Invalid actionh parameter"
							  R1.actionh R1)}
			     end
			  end}
	       else
		  R1
	       end
	    else
	       %% yes => maps the alias and loops
	       if {HasFeature R parent} then
		  {self FlattenLabel({Record.adjoinAt {Alias R} parent R.parent} $)}
	       else
		  {self FlattenLabel({Alias R} $)}
	       end
	    end
      end
      meth !MapLabelToObject(R $)
	 {self MapFlatLabelToObject({self FlattenLabel(R $)} $)}
      end
      meth NewFeat(Class Desc $)
%	 {self NewResource({MakeClass Class Desc} {Subtracts Desc [feature Feature ParentFeature]} $)}
	 R={New {MakeClass Class Desc} {Subtracts Desc [feature Feature ParentFeature]}}
      in
	 R
      end
      meth MapFlatLabelToObject(R1 $)
	 %% pre : R has fully flattened its look and alias and is now a fully regular widget
	 Name={Label R1}
	 Object
	 R={Record.adjoin R1 Init}
	 D={Dictionary.condGet self.Widgets Name nil}
	 if D==nil then
	    {Exception.raiseError qtk(custom "Invalid Widget" "Widget name is not valid." R1)}
	 end
	 proc{SetHandle}
	    if {HasFeature R handle} then
	       R.handle=Object
	    end
	    if {HasFeature R feature} then
	       try
		  (R.parent).(R.feature)=Object
	       catch _ then
		  {Exception.raiseError qtk(custom "Invalid feature parameter" "Can't set parent widget feature :"#R.feature R1)}
	       end
	    end
	 end
	 case D.feature
	 of true then
	    %% to help build this widget, all its children widget will be already expanded their looks and aliases
	    Object={self NewFeat(D.object {self Expand(R $)} $)}
	    {SetHandle}
	 [] menu then
	    Object={self NewFeat(D.object R $)}
	    {SetHandle}
	 [] false then
%			Object={New D.object {Record.subtract R Feature}}
	    Object={self NewFeat(D.object R $)}
	    {SetHandle}
	 end
      in
	 Object
      end
      meth !Grid(...)=M
	 {ExecTk unit {Record.adjoin M grid}}
      end
      meth !GetResourceClass(Type Cl)
	 Cl={Dictionary.get self.ResourceClass Type}
      end
      meth !NewResource(_ Type Init Obj)
	 Cl={self GetResourceClass(Type $)}
	 Ob={New Cl Init}
      in
	 {WeakDictionary.put self.Resources {VirtualString.toAtom {Tk.getTclName Ob}} Ob}
	 Obj=Ob
      end
      meth !StoreResource(Ob)
	 {WeakDictionary.put self.Resources {VirtualString.toAtom {Tk.getTclName Ob}} Ob}
      end
      meth !GetObjectFromTclId(Id Obj)
	 Obj={WeakDictionary.condGet self.Resources {VirtualString.toAtom Id} unit}
      end
   end

   fun{GetBuilder GetToplevelClass}
      {New BuilderClass Init(GetToplevelClass GetMigratableClass)}
   end
      
end
