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
   Tk
   Module
   Error
   QTkDevel
   QTkImage(newImage:           NewImage
	    newImageLibrary:    NewImageLibrary
	    loadImageLibrary:   LoadImageLibrary
	    saveImageLibrary:   SaveImageLibrary
	    buildImageLibrary:  BuildImageLibrary)
   QTkMenu
   QTkSpace
   QTkLabel
   QTkButton
   QTkCheckbutton
   QTkRadiobutton
   QTkScale
   QTkScrollbar
   QTkEntry
   QTkCanvas
   QTkListbox
   QTkText
   QTkDropdownlistbox
   QTkNumberentry
   QTkPlaceholder
   QTkMigratable
   QTkGrid
   QTkPanel
   QTkRubberframe
   QTkScrollframe
   QTkToolbar
   QTkFrame
   QTkFont(newFont:NewFont)
   PrintCanvas
   System(show:Show)
   Browser(browse:Browse)
   
export

   build:DialogBuilder
   buildMigratable:BuildMigratable
   dialogbox:DialogBox
   Bell
   Clipboard
   NewFont
   NewImage
   NewImageLibrary
   LoadImageLibrary
   SaveImageLibrary
   BuildImageLibrary
   buildMenu:NewMenu
   newLook:NewLook
   WInfo
   SetAssertLevel
   QTkDesc
   newBuilder:GetBuilder

prepare
   NoArgs={NewName}
   CharToUpper = Char.toUpper
   fun{Majus Str}
      case {VirtualString.toString Str}
      of C|Cs then {CharToUpper C}|Cs
      [] X then X
      end
   end
   VsToString=VirtualString.toString
   
define

   SetAssertLevel	= QTkDevel.setAssertLevel
   QTkAction		= QTkDevel.qTkAction
   NewLook		= QTkDevel.newLook
   Split		= QTkDevel.split
   SplitGeometry	= QTkDevel.splitGeometry
   SplitParams		= QTkDevel.splitParams
   TkInit		= QTkDevel.tkInit
   ExecTk		= QTkDevel.execTk
   ReturnTk		= QTkDevel.returnTk
   CheckType		= QTkDevel.checkType
   Assert		= QTkDevel.assert
   SetGet		= QTkDevel.setGet
   QTkClass		= QTkDevel.qTkClass
   Subtracts		= QTkDevel.subtracts
   TkToolTips		= QTkDevel.qTkTooltips
   Builder              = QTkDevel.builder
   NewMenu              = QTkMenu.newMenu
   Init                 = QTkDevel.init
   QTkDesc              = QTkDevel.qTkDesc
   WInfo                = QTkDevel.wInfo
   GetSignature         = QTkDevel.getSignature
   FlattenLabel         = QTkDevel.flattenLabel

%    %% create a module manager with Tk and QTkDevel
%    %% so that these don't get reloaded and relinked

%    ModMan={New Module.manager init}
%    {ModMan enter(name:"QTkDevel" QTkDevel)}
%    {ModMan enter(name:"QTk" QTkDevel.qTk)}

%    fun{QTkRegisterWidget GName}
%       FName={VsToString
% 	     "QTK" #
% 	     case {VsToString GName}
% 	     of &t|&d|X then {Majus X}
% 	     [] &l|&r|X then {Majus X}
% 	     []       X then {Majus X}
% 	     end   #
% 	     ".ozf"}
%       M
%    in
%       %% this has become simpler with the new usage of
%       %% failed futures to capture concurrent exceptions
%       {ModMan link(url:FName M)}
%       {Wait M}
%       M
%    end

   
   \insert QTkClipboard.oz
   \insert QTkDialogbox.oz

   fun{GetTopLevelClass BuilderObj}
      class $
	 
	 from QTkFrame.frame Tk.toplevel QTkClass
	    
	 feat
	    !Builder:BuilderObj
	    Inited
	    Return
	    port
	    Closed
	    Radiobuttons
	    Destroyed
	    RadiobuttonsNotify
	    widgetType:toplevel
	    WM:[title aspect client focusmodel geometry grid group
		iconbitmap iconmask iconname iconposition iconwindow
		maxsize minsize overrideredirect resizable transient]
	    typeInfo:r(all:r(look:no
			     borderwidth:pixel
			     cursor:cursor
			     highlightbackground:color
			     highlightcolor:color
			     highlightthickness:pixel
			     relief:relief
			     takefocus:boolean
			     background:color bg:color
			     'class':atom
			     colormap:no
			     container:boolean
			     height:pixel
			     %% menu:no commented as special support has to be furnished for this
			     screen:vs
			     use:vs
			     visual:no
			     width:pixel
			     %% parameters taken into account here
			     action:action  % action is called when the user tries to close the window
			     parent:no
			     return:free    % same return as for buttons
			     %% wm parameters
			     title:vs
			     aspect:no
			     client:vs
			     focusmodel:[active passive]
			     geometry:no
			     grid:no
			     group:no
			     iconbitmap:bitmap
			     iconmask:bitmap
			     iconname:vs
			     iconposition:no
			     iconwindow:no
			     maxsize:no
			     minsize:no
			     useport:no
			     overrideredirect:boolean
			     resizable:no
			     transient:no)
		       uninit:r
		       unset:r(return:unit visual:unit use:unit screen:unit container:unit
			       colormap:unit 'class':unit)
		       unget:r(return:unit group:unit iconbitmap:unit iconmask:unit
			       iconwindow:unit transient:unit))
	    action
	 
	 attr Destroyer
	 
	 
	 prop locking
      
	 meth !Init(M)
	    lock
	       if {IsFree self.Inited} then self.Inited=unit else
		  {Exception.raiseError qtk(custom "Can't build a window" "The window has already been initialized" M)}
	       end
	       if {Label M}\=td andthen {Label M}\=lr then
		  {Exception.raiseError qtk(custom "Bad toplevel widget" {Label M} M)}
	       end
	       OutP
	       proc{Listen L}
		  case L of X|Xs then
		     case X
		     of destroy then
  	             % save datas
			{ForAll {self getChildren($)}
			 proc{$ C} try {C destroy} catch _ then skip end end}
			{self destroy}
	             % close window
			self.Closed=unit
			_={New TkToolTips hide}
			{self tkClose} 
		     else
	             % apply action
			if {IsFree self.Destroyed} then
			   Rec={List.toRecord
				X.2
				{List.filter
				 {List.map
				  {Record.toListInd X}
				  fun{$ R}
				     I J
				  in
				     I#J=R
				     if I>2 then I-2#J else nil end
				  end}
				 fun{$ R} R\=nil end}}
			   proc{DoIt}
			      try
				 {X.1 Rec}
			      catch E then
				 {Error.printException E}
			      end
			   end
			in
			   if {HasFeature M useport} then
			      {Port.send M.useport DoIt}
			   else
			      {DoIt}
			   end
			else skip end % waiting for the destroy instruction => skip pending commands
			{Listen Xs}
		     end
		  else skip end
	       end
	       A B
	       Title={CondSelect M title "Oz/QTk Window"}
	       Out
	    in
	       self.toplevel=self
	       self.Radiobuttons={NewDictionary}
	       self.RadiobuttonsNotify={NewDictionary}
	       Destroyer<-nil
	       self.port={NewPort Out}
	       thread
		  {Listen Out}
	       end
	       QTkClass,{Record.adjoin {Record.filterInd M
					fun{$ I _}
					   {Int.is I}==false
					end} Init(parent:self
						  action:{CondSelect M action toplevel#close})}
	       self.Return={CondSelect M return _}
	       {SplitParams M self.WM A B}
	       Tk.toplevel,{Record.adjoin {TkInit A} tkInit(delete:self.port#r(self Execute)
							    withdraw:true
							   )}
	       {self {Record.adjoin B WM(title:Title
					 iconname:{CondSelect M iconname Title})}}
	       QTkFrame.frame,Init({Subtracts {Record.adjoinAt M parent self} [action return]})
	    end
	 end
      
	 meth set(...)=M
	    lock
	       A B
	    in
	       {SplitParams M self.WM A B}
	       QTkClass,A
	       {Assert self.widgetType self.typeInfo B}
	       {self {Record.adjoin B WM}}
	    end
	 end   

	 meth get(...)=M
	    lock
	       A B
	    in
	       {SplitParams M self.WM A B}
	       QTkClass,A
	       {Assert self.widgetType self.typeInfo B}
	       {self {Record.adjoin B WMGet}}
	    end
	 end
      
	 meth WM(...)=M
	    lock
	       {Record.forAllInd M
		proc{$ I V}
		   proc{Check Type}
		      Err={CheckType Type V}
		   in
		      if Err==unit then skip
		      else
			 {Exception.raiseError qtk(typeError I self.widgetType Err M)}
		      end
		   end
		in
		   case I
		   of title then
		      {Check vs}
		      {Tk.send wm(title self V)}
		   [] aspect then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V} andthen {Label V}==aspect then
			 if {Record.arity V}==nil then
			    {Tk.send wm(aspect self '""' '""' '""' '""')}
			 elseif {Record.arity V}==[maxDenom maxNumer minDenom minNumer]
			    andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			    {Tk.send wm(aspect self V.minNumer V.minDenom V.maxNumer V.maxDenom)}
			 else
			    Err=unit
			 end
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record aspect or aspect(minNumer:int minDenom:int maxNumer:int maxDenom:int) where int is an integer value"
						   M)}
		      end
		   [] client then
		      {Check vs}
		      {Tk.send wm(client self V)}
		   [] focusmodel then
		      {Check [active passive]}
		      {Tk.send wm(focusmodel self V)}
		   [] geometry then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V} andthen {Label V}==geometry
			 andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			 if {Record.arity V}==[height width x y] then
			    {Tk.send wm(geometry self V.width#"x"#V.height#"+"#V.x#"+"#V.y)}
			 elseif {Record.arity V}==[height width] then
			    {Tk.send wm(geometry self V.width#"x"#V.height)}
			 elseif {Record.arity V}==[x y] then
			    {Tk.send wm(geometry self "+"#V.x#"+"#V.y)}
			 else
			    Err=unit
			 end
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record geometry(x:int y:int width:int height:int) where int is an integer value"
						   M)}
		      end
		   [] grid then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V} andthen {Label V}==grid then
			 if {Record.arity V}==nil then
			    {Tk.send wm(grid self '""' '""' '""' '""')}
			 elseif {Record.arity V}==[baseHeight baseWidth heightInc widthInc]
			    andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			    {Tk.send wm(grid self V.baseHeight V.baseWidth V.widthInc V.heightInc)}
			 else
			    Err=unit
			 end
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record grid or grid(minNumer:int minDenom:int maxNumer:int maxDenom:int) where int is an integer value"
						   M)}
		      end		   
		   [] group then
		      if {IsDet V}==false orelse
			 {Tk.returnInt 'catch'(v("{wm group ") self V v("}"))}==1 then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A window"
						   M)}
		      end
		   [] iconbitmap then
		      {Check bitmap}
		      {Tk.send wm(iconbitmap self V)}
		   [] iconmask then
		      {Check bitmap}
		      {Tk.send wm(iconmask self V)}
		   [] iconname then
		      {Check vs}
		      {Tk.send wm(iconname self V)}
		   [] iconposition then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V}
			 andthen {Record.arity V}==[x y]
			 andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			 {Tk.send wm(iconposition self V.x V.y)}
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record coord(x:int y:int) where int is an integer value"
						   M)}
		      end
		   [] iconwindow then
		      if {IsDet V}==false orelse
			 {Tk.returnInt 'catch'(v("{wm iconwindow ") self V v("}"))}==1 then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A window"
						   M)}
		      end
		   [] maxsize then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V}
			 andthen {Record.arity V}==[height width]
			 andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			 {Tk.send wm(maxsize self V.width V.height)}
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record maxsize(width:int height:int) where int is an integer value"
						   M)}
		      end
		   [] minsize then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V}
			 andthen {Record.arity V}==[height width]
			 andthen {Record.all V fun{$ I} {IsDet I} andthen {Int.is I} end} then
			 {Tk.send wm(minsize self V.width V.height)}
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record minsize(width:int height:int) where int is an integer value"
						   M)}
		      end		   
		   [] overrideredirect then
		      {Check boolean}
		      {Tk.send wm(overrideredirect self V)}
		   [] resizable then
		      Err
		   in
		      if {IsDet V} andthen {IsRecord V}
			 andthen {Record.arity V}==[height width]
			 andthen {Record.all V fun{$ I} {IsDet I} andthen I==true orelse I==false end} then
			 {Tk.send wm(resizable self V.width V.height)}
		      else
			 Err=unit
		      end
		      if {IsDet Err} then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A record resizable(width:bool height:bool) where bool is either true or false"
						   M)}
		      end  
		   [] transient then
		      if {IsDet V}==false orelse
			 {Tk.returnInt 'catch'(v("{wm transient ") self V v("}"))}==1 then
			 {Exception.raiseError qtk(typeError I self.widgetType
						   "A window"
						   M)}
		      end
		   else
		      {Exception.raiseError qtk(badParameter I toplevel M)}
		   end
		end}
	    end
	 end

	 meth WMGet(...)=M
	    lock
	       {Record.forAllInd M
		proc{$ I V}
		   Str={Tk.return wm(I self)}
		in
		   V=case I
		     of title then Str
		     [] aspect then {List.toRecord aspect {List.mapInd {Split Str}
							   fun{$ I V}
							      case I
							      of 1 then minNumer
							      [] 2 then minDenom
							      [] 3 then maxNumer
							      [] 4 then maxDenom
							      end#{String.toInt V}
							   end}}
		     [] client then Str
		     [] focusmodel then {String.toAtom Str}
		     [] geometry then {List.toRecord geometry
				       {List.mapInd
					{SplitGeometry Str}
					fun{$ I V}
					   case I
					   of 1 then width
					   [] 2 then height
					   [] 3 then x
					   [] 4 then y
					   end#V
					end}}
		     [] grid then {List.toRecord grid {List.mapInd {Split Str}
						       fun{$ I V}
							  case I
							  of 1 then baseWidth
							  [] 2 then baseHeight
							  [] 3 then widthInc
							  [] 4 then heightInc
							  end#{String.toInt V}
						       end}}
		     [] iconname then Str
		     [] iconposition then {List.toRecord iconposition {List.mapInd {Split Str}
								       fun{$ I V}
									  case I
									  of 1 then x
									  [] 2 then y
									  end#{String.toInt V}
								       end}}
		     [] maxsize then {List.toRecord maxsize {List.mapInd {Split Str}
							     fun{$ I V}
								case I
								of 1 then width
								[] 2 then height
								end#{String.toInt V}
							     end}}
		     [] minsize then {List.toRecord minsize {List.mapInd {Split Str}
							     fun{$ I V}
								case I
								of 1 then width
								[] 2 then height
								end#{String.toInt V}
							     end}}
		     [] overrideredirect then Str=="1"
		     [] resizable then {List.toRecord resizable {List.mapInd {Split Str}
								 fun{$ I V}
								    case I
								    of 1 then width
								    [] 2 then height
								    end#V=="1"
								 end}}
		     end
		   {Wait V}
		end}
	    end
	 end
      
	 meth show(wait:W<=false modal:M<=false)
	    lock
	       {Tk.send wm(deiconify self)}
	       if M then
		  {Tk.send grab(self)}
	       end
	    end
	    if W then
	       {Wait self.Closed}
	    end
	 end

	 meth wait
	    {Wait self.Closed}
	 end
      
	 meth hide
	    lock
	       {Tk.send wm(withdraw self)}
	    end
	 end
      
	 meth close
	    lock
	       self.Destroyed=unit
	       {Send self.port destroy}
	    end
	 end

	 meth iconify
	    lock
	       {Tk.send wm(iconify self)}
	    end
	 end
      
	 meth deiconify
	    lock
	       {Tk.send wm(deiconify self)}
	    end
	 end

      % internal methods for the good behaviour of buttons and radiobuttons

	 meth Execute
	    lock
	       Destroyer<-self
	       {self.action execute}
	    end
	 end
      
	 meth setDestroyer(Obj)
	    lock
	       Destroyer<-Obj
	    end
	 end

	 meth getDestroyer(Obj)
	    lock
	       Obj=@Destroyer
	    end
	 end

	 meth askNotifyRadioButton(Key Obj)
	    lock
	       {Dictionary.put self.RadiobuttonsNotify Key
		{Append [Obj] {Dictionary.condGet self.RadiobuttonsNotify Key nil}}
	       }
	    end
	 end
      
	 meth notifyRadioButton(Key)
	    lock
	       {ForAll
		{Dictionary.condGet self.RadiobuttonsNotify Key nil}
		proc{$ O}
		   try {O notify} catch _ then skip end
		end}
	    end
	 end
      
	 meth putRadioDict(Key Value)
	    lock
	       {Dictionary.put self.Radiobuttons Key Value}
	    end
	 end
      
	 meth getRadioDict(Key Value)
	    lock
	       V={Dictionary.condGet self.Radiobuttons Key nil}
	    in
	       if V==nil then
		  {self putRadioDict(Key r({New Tk.variable tkInit(1)} 0))}
		  {self getRadioDict(Key Value)}
	       else
		  Value=V
	       end
	       {self putRadioDict(Key r(Value.1 Value.2+1))}
	    end
	 end
      
	 meth destroy
	    lock
	       self.Return=@Destroyer==self
	    end
	 end

	 meth newAction(Act Ret)
	    lock
	       {{New QTkAction init(parent:self action:Act)} action(Ret)}
	    end
	 end

	 meth wInfo(What $)
	    {WInfo What}
	 end
      
      end
   end

   
   proc{Bell}
      {Tk.send bell}
   end


   fun{GetBuilder}
      Builder={QTkDevel.getBuilder GetTopLevelClass}
   in
      {ForAll [QTkFrame
	       QTkMenu
	       QTkSpace
	       QTkLabel
	       QTkButton
	       QTkCheckbutton
	       QTkRadiobutton
	       QTkScale
	       QTkScrollbar
	       QTkEntry
	       QTkCanvas
	       QTkListbox
	       QTkText
	       QTkPlaceholder
	       QTkMigratable
	       QTkGrid
	       QTkRubberframe
	       QTkScrollframe
 	       QTkPanel
	       QTkToolbar]
       proc{$ V}
	  {ForAll V.register proc{$ W} {Builder.register W} end}
       end}
      {ForAll [QTkDropdownlistbox
 	       QTkNumberentry]
       proc{$ V}
 	  {ForAll V.register proc{$ W} {Builder.setAlias W.widgetType W.widget} end}
	end}
      Builder
   end

   DefaultBuilder={GetBuilder}

   DialogBuilder=DefaultBuilder.build
   BuildMigratable=DefaultBuilder.buildMigratable

   {Tk.send tk_setPalette(grey)} % to force all qtk users to have the same default palette

end
