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
   Fault
   GS at 'GetSignature.ozf'
   QTkDevel
   QTkBare
   System(show:Show)
   Browser(browse:Browse)
   Tk
   Error
   QTkFont(setCallBack:SetFontCallBack
	   removeCallBack:RemoveFontCallBack
	   getFontObj:GetFontObj
	   getFontId:GetFontId
	   isFont:IsFont
	   newSilentFont:NewFont)
   QTkImage(isImage:IsImage
	    newImage:NewImage
	    getImageId:GetImageId)
   TkBoot at 'x-oz://boot/Tk'
   Session
    
export

   GetMigratableClass
   Register
   AssertStream

prepare
   NoArgs={NewName}
   Parent={NewName}
   CharToUpper = Char.toUpper
   fun{Majus Str}
      case {VirtualString.toString Str}
      of C|Cs then {CharToUpper C}|Cs
      [] X then X
      end
   end
   VsToString=VirtualString.toString
   OBC={NewName}
   IMG={NewName}
   LNK={NewName}
   CRT={NewName}
   Disco={NewName}
   TempFail={NewName}
   Mgr={NewName}
   Name={NewName}
   Type={NewName}
   SetAction={NewName}
   Exec={NewName}
   Action={NewName}
   Run={NewName}
   UpdateState={NewName}
   SetState={NewName}
   GetState={NewName}
   TransfertState={NewName}
   GetResInfo={NewName}
   AddResource={NewName}
   DeleteResource={NewName}
   Destroy={NewName}
   
define

   %% debug stuff
   AssertStream
   AP={NewPort AssertStream}
   proc{AssertRes R}
      fun{Loop R}
	 if {Record.is R} then
	    fun{Loop2 L}
	       case L of X|Xs then
		  T={Loop X}
	       in
		  if T==unit then {Loop2 Xs} else T end
	       else unit end
	    end
	 in
	    {Loop2 {Record.toList R}}
	 elseif {Object.is R} then
	    R
	 else
	    unit
	 end
      end
   in
      thread
	 T={Loop R}
      in
	 if T\=unit then {Port.send AP T#R} end
      end
   end

   %%

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
   Init                 = QTkDevel.init
   QTkDesc              = QTkDevel.qTkDesc
   WInfo                = QTkDevel.wInfo
   GetSignature         = QTkDevel.getSignature
   FlattenLabel         = QTkDevel.flattenLabel
   GlobalInitType       = QTkDevel.globalInitType
   GlobalUnsetType      = QTkDevel.globalUnsetType
   GlobalUngetType      = QTkDevel.globalUngetType
   MapLabelToObject     = QTkDevel.mapLabelToObject
   Grid                 = QTkDevel.grid
   NotifyResource       = QTkDevel.notifyResource
   NewResource          = QTkDevel.newResource
   GetObjectFromTclId   = QTkDevel.getObjectFromTclId
   GetResourceClass     = QTkDevel.getResourceClass
   WInfo                = QTkDevel.wInfo
   TclName
   {TkBoot.getNames _ _ TclName}

   proc{DefPop Widget Ret}
      fun{GetParams}
	 P={Record.filterInd Widget.typeInfo.all
	    fun{$ I V}
	       {Not {HasFeature Widget.typeInfo.unset I}} andthen
	       {Not {HasFeature Widget.typeInfo.unget I}} andthen
	       {Not {HasFeature GlobalInitType I}} andthen
	       {Not {List.member V [action]}}
	    end}
	 R={Record.adjoin {Record.map P
			   fun{$ _} _ end}
	    get}
      in
	 {Widget R}
	 R
      end
   in
      case Widget.widgetType
      of canvas then
	 Dump={Widget dump($ handles:true tags:true)}
	 Params={GetParams}
% 	 Dump={List.map Dump
% 	       fun{$ E}
% 		  if {HasFeature E tags} then
% 		     {Record.adjoinAt E tags
% 		      {List.toTuple q
% 		       {List.filter
% 			{Record.toList E.tags}
% 			fun{$ T} {HasFeature T Mgr} end}}
% 		     }
% 		  else
% 		     E
% 		  end
% 	       end}
      in
	 Ret=Dump#if Params.scrollregion==q then
		     {Record.subtract Params scrollregion}
		  else
		     Params
		  end
      else
	 Ret={GetParams}
      end
   end
   
   proc{DefPush Widget S Resources AssignResource}
      case Widget.widgetType
      of canvas then
	 H T
	 {List.partition Resources fun{$ C#_} C==ct end T H}
	 Tags={List.map T fun{$ _#O} O end}
	 Handles={List.map T fun{$ _#O} O end}
	 D#P=S
      in
	 {ForAll Tags
	  proc{$ H} {Widget newTag({AssignResource H})} end}
	 {Widget {Record.adjoin P set}}
	 {ForAll D
	  proc{$ Cmd}
	     try
		if {HasFeature Cmd handle} then
		   {Widget {Record.adjoinAt Cmd handle
			    {AssignResource Cmd.handle}}}
		else
		   {Widget Cmd}
		end
	     catch X then {Error.printException X} end
	  end}
      [] text then
	 M T
	 {List.partition Resources fun{$ C#_} C==tt end T M}
	 Tags={List.map T fun{$ _#O} O end}
	 Marks={List.map T fun{$ _#O} O end}
      in
	 {ForAll Tags
	  proc{$ H} {Widget newTag({AssignResource H})} end}
	 {ForAll Marks
	  proc{$ H} {Widget newMark({AssignResource H})} end}
	 {Widget {Record.adjoin S set}}
      else
	 {Widget {Record.adjoin S set}}
      end
   end
   
   class ProxyClass
      feat
	 !Mgr
	 !Name
	 ResDict
	 ActionDict
	 EventDict
	 bad:unit
      attr
	 State
	 Pop:unit
	 Push:unit
      meth !Init(M N)
	 self.Mgr=M
	 self.Name=N
	 self.EventDict={NewDictionary}
	 self.ActionDict={NewDictionary}
	 self.ResDict={NewDictionary}
      end
      meth bind(...)=M
	 R
	 proc{SetIt}
	    Event={VirtualString.toAtom M.event}
	    Add={CondSelect M append false} 
	 in
	    if {HasFeature M action} then
	       {self SetAction(M.action R)}
	       if Add then
		  {Dictionary.put self.EventDict Event
		   {Record.adjoinAt M action R}|{Dictionary.condGet self.EventDict Event nil}}
	       else
		  {ForAll {Dictionary.condGet self.EventDict Event nil}
		   proc{$ E}
		      {Dictionary.remove self.ActionDict E.action}
		   end}
		  {Dictionary.put self.EventDict Event {Record.adjoinAt M action R}|nil}
	       end
	    else
	       {ForAll {Dictionary.condGet self.EventDict Event nil}
		proc{$ E}
		   {Dictionary.remove self.ActionDict E.action}
		end}			
	    end
	 end
	 Err
      in
	 if {HasFeature M action} then
	    R={NewName}
	    {self.Mgr strongSend(self.Name {Record.adjoinAt M action R} Err)}
	 else
	    {self.Mgr strongSend(self.Name M Err)}
	 end
	 if Err==ok then
	    {SetIt}
	 else
	    raise Err end
	 end
      end
      meth destroy
	 {self.Mgr destroy(self.Name)}
      end
      meth otherwise(Msg)
	 Err
	 Msg2={self.Mgr translate(Msg $)}
      in
	 {self.Mgr strongSend(self.Name Msg2 Err)}
	 if Err\=ok then
	    raise Err end
	 end
      end
      meth !Exec(V)
	 P#R={Dictionary.get self.ActionDict V.2}
      in
	 {Port.send P
	  {Record.adjoin V R}}
      end
      meth !SetAction(A R)
	 if {IsFree R} then R={NewName} end
	 {Dictionary.put self.ActionDict R
	  {{New QTkAction init(parent:self.Mgr action:A)} action($)}}
      end
      meth !SetState(S)
	 Cmd=if @Push==unit then
		{self.Mgr getPush(self.widgetType $)}
	     else
		@Push
	     end
	 fun{AssignResource Obj}
	    !LNK#Obj.Name
	 end
	 fun{Loop L}
	    case L of I#T|Ls then
	       Ob={self.Mgr getObj(I $)}
	    in
	       if Ob==I then {Loop Ls}
	       else
		  T#Ob|{Loop Ls}
	       end
	    else nil end
	 end
      in
	 {Cmd self S {Loop {Dictionary.entries self.ResDict}} AssignResource}
      end
      meth !GetState($)
	 if {IsDet @State} then
	    r(params:@State
	      events:{Dictionary.entries self.EventDict})
	 else
	    r(events:{Dictionary.entries self.EventDict})
	 end
      end
      meth !UpdateState(1:S<=_)
	 Cmd=if @Pop==unit then %use default pop function for this type of widget
		{self.Mgr getPop(self.widgetType $)}
	     else
		@Pop
	     end
	 Err
      in
	 try
	    {Cmd self S}
	 catch _ then 
	    Err=unit
	 end
	 if {IsFree Err} then
	    State<-S
	 end
      end
      meth !TransfertState(Pop1 Push1)
	 Pop<-Pop1
	 Push<-Push1
      end
      meth !AddResource(Name Type)
	 {Dictionary.put self.ResDict Name Type}
      end
      meth !DeleteResource(Name)
	 {Dictionary.remove self.ResDict Name}
      end
   end

   class ResourceProxyClass

      from ProxyClass
      feat
	 !Parent
	 !Type
	 
      meth !Init(M N P T)
	 ProxyClass,Init(M N)
	 self.Parent=P
	 self.Type=T
	 {P AddResource(N T)}
      end

      meth !Destroy
	 {self.Parent DeleteResource(self.Name)}
      end
   end
   
   class MigratableManager
      feat
	 widgets    % Dictionary containing all widgets
	 resources  % WeakDictionary containing the ressources
	 toplevel
	 Desc       % Initial description record
	 Supp       % Supplementary descriptions records : built on the fly, while the window is existing
	 DefaultST
	 fonts
	 flock
	 session
	 sendl
      attr
	 GridInfo
	 fcb
	 alwaysAccept
	 bufhead
	 buftail
	 priority
	 sendth
      prop locking
      meth !Init(Top D)
	 buftail<-@bufhead
	 self.session={Session.new}
	 self.toplevel=Top
	 self.widgets={NewDictionary}
	 self.sendl={NewLock}
	 sendth<-unit
	 priority<-unit
	 thread
	    S
	    self.resources={WeakDictionary.new S}
	 in
	    {ForAll S
	     proc{$ K#O}
		lock
		   if {Session.isConnected self.session} then
		      if @priority==unit then
			 %% connected to another site
			 {Session.aSend self.session deleteResource(K)}
		      else
			 %% connecting to another site
			 %% => will remove the resource later
			 {WeakDictionary.put self.resources K O}
		      end
		   end
		   {O.Parent DeleteResource(K)}
		end
	     end}
	 end
	 {Wait self.resources}
	 self.DefaultST={NewDictionary}
	 self.Desc={self mapLabelToObject(D $)}
	 self.Supp={NewCell nil}
	 fcb<-unit
	 GridInfo<-nil
	 alwaysAccept<-true
	 {Session.bind self.session permFail proc{$} {Session.lSend self.session disco} end}
%	 {Session.bind self.session disconnect proc{$} {Session.lSend self.session disco} end}
%	 {Session.bind self.session connect proc{$} {Session.lSend self.session connect} end}
	 thread
	    {ForAll {Session.getSideStream self.session}
	     proc{$ Id#M}
%		{Session.lSend self.session connectTo(Id)}
		{Fault.enable M site nil _}
		if {Not {Session.isConnected self.session}} then
		   thread M=true end
		elseif @alwaysAccept then
		   thread
		      {Session.waitDisconnect self.session}
		      M=true
		   end
		   {Session.lSend self.session disco}
		else
		   thread M=false end
		end
	     end}
	 end
	 {Session.configure self.session autoBreakAfter 30000}
	 thread
	    {ForAll {Session.getStream self.session}
	     proc{$ M}
		{self M}
	     end}
	 end
% 	 thread
% 	    %% buffer management thread
% 	    proc{Loop}
% 	       M
% 	    in
% 	       {Wait @bufhead.1.4}
% 	       _|M=@bufhead
% 	       bufhead<-M
% 	    end
% 	 in
% 	    {Loop}
% 	 end
      end
      meth startSendMessages
	 lock
	    sendth<-_
	    thread
	       @sendth={Thread.this}
	       proc{Loop L}
		  case L
		  of To#Msg#Err#Read|Ls then
		     if {IsFree Read} then
			Err1
		     in
			try
			   {Session.aSend self.session rec(To Msg Err1)}
			catch _ then
			   {Wait _} %% this thread is going to be killed, it should suspend
			end
			thread
			   {Session.waitDisconnectOr self.session Err1}
			   lock self.sendl then
			      if {IsDet Err1} andthen {IsFree Err} then
				 Err=Err1
				 Read=unit
			      end
			   end
			end
		     end
		     {Loop Ls}
		  end
	       end
	    in
	       {Loop @bufhead}
	    end
	    {Wait @sendth}
	 end
      end
      meth stopSendMessages
	 lock
	    try
	       {Thread.terminate @sendth}
	    catch _ then skip end
	    sendth<-unit
	 end
      end
      meth !Grid(...)=M
% 	 if {List.member M.1 [configure forget]} then
% 	    L={Record.toList {Record.filterInd M fun{$ I _} {Int.is I} end}}
% 	    fun{Loop S}
% 	       case S of X|Xs then
% 		  A B AL
% 	       in
% 		  {Record.partitionInd X fun{$ I _} {Int.is I} end A B}
% 		  AL={Record.toList A}
% 		  if {List.some AL fun{$ E} {List.member E L} end} then
		     
% 	       else nil
% 	       end
% 	    end
% 	 in
% 	    GridInfo<-M|{Loop @GridInfo}
% 	 else
	 GridInfo<-M|@GridInfo %% this is just logged (no optimization for now)
%	 end
%	 {Browse @GridInfo}
      end
      meth remoteBuild(Desc Ret)
	 Local={self mapLabelToObject(Desc $)}
      in
	 Ret={Record.adjoinAt Local parent Desc.parent}
      end
      meth reBind(Id R)
	 R={{self getObj(Id $)} GetState($)}.events
      end
%      meth deleteResource(K)
%	 try
%	    {Session.aSend self.session deleteResource(K)}
%	 catch _ then skip end
%      end
      meth dynaBuild(To Msg L)
	 {Assign self.Supp To#Msg#L|{Access self.Supp}}
      end
      meth getImage(Id D)
	 Obj={WeakDictionary.condGet self.resources Id nil}
      in
	 if Obj==nil then
	    D=bitmap
	 else
	    fun{GetParams L}
	       {List.map L
		fun{$ P}
		   P#{Obj get(P:$)}
		end}
	    end
%	    Params={GetParams [background foreground channel format gamma height width palette]}
	    Params=nil
	    Data={Obj get(data:$)}
	    MData={Obj get(maskdata:$)}
	    Type={Obj get(type:$)}
	 in
	    D={Record.adjoin
	       {Record.adjoin
		{List.toRecord r Params}
		if MData==nil then r(data:Data) else r(data:Data maskdata:MData) end}
	       Type}
	 end
      end
      meth mapLabelToObject(Desc Ret)
	 fun{Loop M0 Prt}
	    M={self.toplevel.Builder FlattenLabel(M0 $)}
	    Type={Label M}
	 in
	    if {List.member Type [newline continue]} then
	       M
	    else
	       fun{FLoop L}
		  case L
		  of I#V|Xs then
		     if {Int.is I} andthen {IsDet V} andthen {Record.is V} andthen {HasFeature V feature} then
			V.feature|{FLoop Xs}
		     else
			{FLoop Xs}
		     end
		  else
		     nil
		  end
	       end
	       Sig={self.toplevel.Builder GetSignature(Type $)}
	       FeatList={FLoop {Record.toListInd M}}
	       Name={NewName}
	       M1={Record.subtract M feature}      %% this is no more needed
	       M2={Record.subtract M1 look}        %% looks have already been expanded
	       M3={Record.adjoinAt M2 handle Name} %% store this widget's id
	       D
	       Proxy={New {Class.new [ProxyClass] r {List.toTuple r
						     {List.append FeatList
						      {List.map {Record.toListInd Sig.'feat'}
						       fun{$ I#_} I end}
						     }} nil}
		      Init(self Name)}
	       {Record.forAllInd Sig.'feat'
		proc{$ I V}
		   if I\=parent andthen I\=toplevel then
		      Proxy.I=V
		   end
		end}
	       Proxy.toplevel=self
	       Proxy.parent=Prt
	       {CondSelect M handle _}=Proxy
	       if {HasFeature M feature} then
		  Prt.(M.feature)=Proxy
	       end
	    in
	       {Dictionary.put self.widgets Name Proxy}
	       {Record.mapInd M3
		fun{$ I V}
		   case I
		   of action then
		      {Proxy SetAction(V $)}
		   elseif {Int.is I} andthen {IsDet V} andthen {Record.is V} then
		      {Loop V Proxy}
		   else
		      V
		   end
		end}
	    end
	 end
	 Parent=if {HasFeature Desc parent} then
		   {Dictionary.get self.widgets Desc.parent}
		else
		   self
		end
      in
	 Ret={Loop {Record.subtract Desc parent} Parent}
      end
      meth connect(V)
	 lock
	    fun{Loop L}
	       case L of N#W|Xs then
		  S={W GetState($)}
	       in
		  if {IsDet S} then N#S|{Loop Xs}
		  else {Loop Xs} end
	       else nil end
	    end
	    States={Loop {Dictionary.entries self.widgets}}
	    Sync
	    Fonts
	    fcb<-proc{$ M}
		    try
		       {Session.aSend self.session recFont(M)}
		    catch _ then skip end
		 end
	    {SetFontCallBack @fcb Fonts}
	 in
	    V=self.Desc#{Reverse {Access self.Supp}}#{List.map States fun{$ Id#S} Id#S.events end}#{Reverse @GridInfo}#Fonts#Sync
	    {Session.waitDisconnectOr self.session Sync}
	    thread
	       priority<-{Thread.this}
	       if {IsDet Sync} then
		  {ForAll States
		   proc{$ Id#S}
		      if {HasFeature S params} then
			 W={Dictionary.get self.widgets Id}
		      in
			 {W SetState(S.params)}
		      end
		   end}
	       end
	       try
		  {Session.aSend self.session recall(start)}
	       catch _ then skip end
	    end
	       
%	    {self startSendMessages}
%	    {Show starting}
	 end	 
      end
      meth start
	 lock
	    priority<-unit
	    {self startSendMessages}
	 end
      end
      meth disco
	 lock
	    if {Session.isConnected self.session} then
	       try
		  {Thread.terminate @priority}
	       catch _ then skip end
	       if @sendth\=unit then
		  {self stopSendMessages}
		  priority<-{Thread.this}
		  {self updateState}
	       end
	       priority<-unit
	       {Session.break self.session}
	       {RemoveFontCallBack @fcb}
	    end
	 end
      end
      meth destroy(...)=M
	 {self disco}
      end
      meth translate(Msg $)
	 fun{Parse R}
	    if {IsDet R} then
	       case R
	       of !OBC#Id then
		  {self getObj(Id $)}
	       [] !CRT#(Id#Type#PId) then
		  Parent={self getObj(PId $)}
		  Cl={self.toplevel.Builder GetResourceClass(Type $)}
		  Sig={GS.getClassInfo Cl}
		  Proxy={New {Class.new [ResourceProxyClass]
			      r {List.toTuple r
				 {List.map {Record.toListInd Sig.'feat'}
				  fun{$ I#_} I end}
				} nil}
			 Init(self Id Parent Type)}
	       in
		  {WeakDictionary.put self.resources Id Proxy}
		  Proxy
	       [] !IMG#Id then
		  {WeakDictionary.condGet self.resources Id nil}
	       elseif {Record.is R} then
		  {Record.filter
		   {Record.map R Parse}
		   fun{$ E}
		      if E\=OBC then true else
			 {Show removing#R}
			 false
		      end
		   end}
	       else
		  R
	       end
	    else
	       N
	    in
	       thread
		  {WaitOr N R}
		  if {IsFree R} then
		     R={ParseOut N}
		  else N={Parse R} end
	       end
	       N
	    end
	 end
	 fun{ParseOut R}
	    if {IsDet R} then
	       if {Object.is R} then
		  {self getId(R $)}
	       elseif {List.is R} then
		  {List.map R ParseOut}
	       elseif {Record.is R} then
		  {Record.map R ParseOut}
	       else
		  R
	       end
	    else
	       N
	    in
	       thread
		  {WaitOr N R}
		  if {IsFree R} then
		     R={Parse N}
		  else N={ParseOut R} end
	       end
	       N
	    end
	 end
      in
	 {ParseOut Msg}
      end
      meth getId(Obj $)
	 if {HasFeature Obj Name} then
	    OBC#(Obj.Name)
	 elseif {IsFont Obj} then
	    OBC#{GetFontId Obj}
	 elseif {IsImage Obj} then
	    %% images are stored in the widget dictionary when exported
	    {WeakDictionary.put self.resources {GetImageId Obj} Obj}
	    IMG#{GetImageId Obj}
	 else
	    Obj
	 end
      end
      meth getObj(Id $)
	 R={Dictionary.condGet self.widgets Id
	    {WeakDictionary.condGet self.resources Id
	     {GetFontObj Id}}}
      in
	 if R==unit orelse R==Id then
	    {Show unableToMarshall#Id}
	    OBC
	 else
	    R
	 end
      end
%       meth send(To Msg Err<=_)
% 	 {Session.waitConnect self.session}
% 	 lock
% 	    {Session.aSend self.session rec(To Msg Err)}
% 	 end
%       end
      meth weakSend(To Msg Err)
	 S
      in
	 try
	    {Session.aSend self.session rec(To Msg Err)}
	 catch _ then Err=disconnected end
      end
      meth strongSend(To Msg Err<=_)=M
	 if @priority\=unit andthen {Thread.this}==@priority then
	    G
	 in
	    {self weakSend(To Msg Err)}
	    thread
	       {Session.waitDisconnectOr self.session Err}
	       if {IsFree Err} then Err=disconnected end
	    end
	 else
	    lock self.sendl then
	       N
	    in
	       @buftail=To#Msg#Err#_|N
	       buftail<-N
	    end
	 end
      end
      meth isConnected($)
	 {Session.isConnected self.session}
      end
      meth exec(...)=V
	 Widget={Dictionary.condGet self.widgets V.1
		 {WeakDictionary.condGet self.resources V.1 unit}}
      in
	 if Widget\=unit then
	    {Widget Exec(V)}
	 else
	    {Show ignored#V}
	 end
      end
      meth dump($)
	 self.Desc#{Dictionary.entries self.widgets}
      end
      meth getRef($)
	 {Session.getRef self.session}
      end
      meth stop
	 {Thread.terminate {Thread.this}}
      end
      meth transfertState(Widget Pop1 Push1)
	 if {Atom.is Widget} then
	    Old={Dictionary.condGet self.DefaultST Widget r(unit unit)}
	 in
	    if Pop1==unit andthen Push1==unit then
	       {Dictionary.remove self.DefaultST Widget}
	    else
	       Pop=if Pop1==unit then DefPop
		   elseif {Procedure.arity Pop1}==2 then
		      Pop1
		   elseif Old.1==unit then
		      proc{$ Widget Ret}
			 {Pop1 DefPop Widget Ret}
		      end
		   else %% arity\=2 andthen Old.1\=unit
		      proc{$ Widget Ret}
			 {Pop1 Old.1 Widget Ret}
		      end
		   end
	       Push=if Push1==unit then DefPush
		    elseif {Procedure.arity Push1}==2 then
		       Push1
		    elseif Old.2==unit then
		       proc{$ Widget Ret}
			  {Push1 DefPush Widget Ret}
		       end
		    else %% arity\=2 andthen Old.1\=unit
		       proc{$ Widget Ret}
			  {Push1 Old.2 Widget Ret}
		       end
		    end
	    in
	       {Dictionary.put self.DefaultST Widget r(Pop Push)}
	    end
	 else {Widget TransfertState(Pop1 Push1)}
	 end
      end
      meth getPop(Widget $)
	 {Dictionary.condGet self.DefaultST Widget r(DefPop DefPush)}.1
      end
      meth getPush(Widget $)
	 {Dictionary.condGet self.DefaultST Widget r(DefPop DefPush)}.2
      end
      meth updateState
	 {ForAll {Dictionary.items self.widgets}
	  proc{$ W} {W UpdateState} end}
      end
      meth getResource(Id Type $)
	 Id#Type
      end
   end
   
   fun{GetMigratableClass BuilderObj}
      BO
      class GMC
	 prop
	    locking
	 attr
	    AutoWindow
	    OnConnect
	    OnDisconnect
	    Destroyer
	    
	 feat
	    !Builder:BuilderObj
	    DefWindow
	    Inited
	    port
	    Closed
	    Radiobuttons
	    Destroyed
	    RadiobuttonsNotify
	    widgetType:migratable
	    Mgr
	    WM:[title aspect client focusmodel geometry grid group
		iconbitmap iconmask iconname iconposition iconwindow
		maxsize minsize overrideredirect resizable transient]
	    typeInfo:r(all:{Record.adjoin GlobalInitType
			    r(look:no
			      borderwidth:pixel
			      cursor:cursor
			      highlightbackground:color
			      highlightcolor:color
			      highlightthickness:pixel
			      relief:relief
			      takefocus:boolean
			      background:color bg:color
			      autowindow:boolean
			      height:pixel
			      width:pixel
			      useport:no
			      ref:no
			      visual:no
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
			      overrideredirect:boolean
			      resizable:no
			      transient:no)}
		       uninit:r(ref:unit)
		       unset:{Record.adjoin GlobalUnsetType
			      r(container:unit
				ref:unit
				use:unit
				screen:unit
				visual:unit)}
		       unget:{Record.adjoin GlobalUngetType
			      r(group:unit
				iconbitmap:unit
				iconmask:unit
				iconwindow:unit
				transient:unit
				container:unit
				visual:unit)})
	 
	 
	 from QTkClass

	 meth !Init(M1)
	    lock
	       M={Record.subtract M1 autowindow}
	       if {IsFree self.Inited} then self.Inited=unit else
		  {Exception.raiseError qtk(custom "Can't build a migratable window" "The window has already been initialized" M)}
	       end
	       if {Label M}\=td andthen {Label M}\=lr then
		  {Exception.raiseError qtk(custom "Bad toplevel widget" {Label M} M)}
	       end
	       OutP
	    in
	       self.toplevel=self
	       self.Radiobuttons={NewDictionary}
	       self.RadiobuttonsNotify={NewDictionary}
	       Destroyer<-nil
	       thread
		  proc{Listen L}
		     case L of X|Xs then
			case X
			of destroy then
			   {ForAll {self getChildren($)}
			    proc{$ C} try {C destroy} catch _ then skip end end}
			   {self destroy}
			   self.Closed=unit
			   _={New TkToolTips hide}
			   {self tkClose} 
			else
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
		  Out
	       in
		  self.port={NewPort Out}
		  {Listen Out}
	       end
	       {Wait self.port}
	       QTkClass,{Record.adjoin {Record.filterInd {Record.subtract M autowindow}
					fun{$ I _}
					   {Int.is I}==false
					end} Init(parent:self)}
	       AutoWindow<-{CondSelect M autowindow true}
	       self.Mgr={New MigratableManager Init(self M)}
	       {self bind(event:disconnect)}
	       {self bind(event:connect)}
	       {self Disco}
	    end
	 end

	 meth !Disco
	    lock
	       {@OnDisconnect}
	       {self SetAutoWindow}
	    end
	 end

	 meth SetAutoWindow
	    lock
	       if @AutoWindow then
		  if {IsFree self.DefWindow} then
		     {QTkBare.build td(receiver(handle:self.DefWindow)) _}
		  end
		  {self.DefWindow set({self get(ref:$)})}
	       end
	    end
	 end
	    
	 meth set(...)=M
	    A B
	 in
	    {SplitParams M [autowindow] B A}
	    if A\=set then
	       {Assert self.widgetType self.typeInfo B}
	       AutoWindow<-A.autowindow
	       %% reconnect if necessary
	       if {Not {self.Mgr isConnected($)}} then
		  {self SetAutoWindow}
	       end
	    end
	    if B\=set then
	       Err
	    in
	       {self.Mgr send(unit B Err)}
	       if Err\=ok then raise Err end end
	    end
	 end

	 meth get(...)=M
	    A B
	 in
	    {SplitParams M [ref autowindow] B A}
	    {Assert self.widgetType self.typeInfo B}
	    {Record.forAllInd A
	     proc{$ I V}
		V=case I
		  of ref then {self.Mgr getRef($)}
		  [] autowindow then @AutoWindow
		  end
	     end}
	    if B\=get then
	       Err
	    in
	       {self.Mgr send(unit B Err)}
	       if Err\=ok then raise Err end end
	    end
	 end

	 meth bind(event:E
		   action:P<=NoArgs)=M
	    fun{Get}
	       if P==NoArgs then
		  proc{$} skip end
	       else
		  Err={CheckType action P}
	       in
		  if Err==unit then skip else
		     {Exception.raiseError qtk(typeError P self.widgetType Err M)}
		  end
		  case P
		  of toplevel#R then
		     proc{$} {self R} end
		  [] widget#R then
		     proc{$} {self R} end
		  [] L#R then
		     if {Port.is L} then
			proc{$} {Port.send L R} end
		     else
			proc{$} {L R} end
		     end
		  else P end
	       end
	    end
	 in
	    case {VirtualString.toAtom E}
	    of connect then
	       OnConnect<-{Get}
	    [] disconnect then
	       OnDisconnect<-{Get}
	    else
	       {Exception.raiseError qtk(custom "Invalid event" "Migratable window support only connect and disconnect events" M)}
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
	       {self.Mgr destroy}
	    end
	 end

	 meth transfertState(Widget Pop Push)
	    {self.Mgr transfertState(Widget Pop Push)}
	 end

	 meth updateState(1:W<=NoArgs 2:R<=_)
	    if W==NoArgs then
	       {self.Mgr updateState}
	    else
	       {W UpdateState(R)}
	    end
	 end
      end
   in
      GMC
   end

   
   WidgetType=receiver
   Feature=true

   class ConClass
      feat
	 main
	 widgets
	 resdict
	 tkmap
	 fonts
	 res
	 session
	 block
      attr
	 child
	 mode:n
	 rec
	 con
	 alwaysConnect
	 timeout
      prop locking
      meth init(O)
	 con<-0
	 self.main=O
	 self.widgets={NewDictionary}
	 {Dictionary.put self.widgets unit self}
	 self.tkmap={NewDictionary}
	 self.resdict={NewDictionary}
	 self.block={NewLock}
	 alwaysConnect<-true
	 child<-unit
	 rec<-nil
	 timeout<-30000
	 self.session={Session.new}
%	 {Session.bind self.session disconnect proc{$} {self disco} end}
	 {Session.bind self.session permFail proc{$} {self disco} end}
	 {Session.bind self.session tempFail proc{$} {self.main TempFail} end}
%	 {Session.bind self.session connect proc{$} {Show 'connected!'} end}
	 thread
	    {ForAll {Session.getStream self.session}
	     proc{$ M}
%		{AssertRes M}
%		{Show cr#M}
		{self M}
%		{Show cd#M}
	     end}
	 end
	 local
	    Self=self
	    fun{Mode}
	       @mode
	    end
	    proc{SMode R} mode<-R end
	    proc{PushRec V}
	       rec<-V|@rec
	    end
	    class MyBuilder
	       meth !Init skip end
	       meth !MapLabelToObject(Desc Ob)
		  lock Self.block then
		     case {Mode}
		     of n then  %% normal mode
			{O.parent.Builder MapLabelToObject(Desc Ob)}
		     [] r then  %% redirect mode
			%% the parent feature refers to a local ressource => detach it
			B={Record.adjoinAt Desc parent {Self getId(Desc.parent $)}}
			T
		     in
			{PushRec T}
			try
			   {Session.aSend Self.session remoteBuild(B T)}
			catch _ then skip end
			{Session.waitDisconnectOr Self.session T}
			if {IsFree T} then
			   {Self mapLabelToObject(td Ob)}
			else
			   {Self mapLabelToObject(T Ob)}
			end
			{Wait Ob}
		     [] s(LDesc) then %% special mode to reconnect dynamically built widgets
			Desc=LDesc.1
		     in
			{SMode r}
			{Self mapLabelToObject(Desc Ob)}
			{Self storeWidget(Desc.handle Ob)}
			{SMode s({List.drop LDesc 1})}
		     end
		  end
	       end
	       meth !NewResource(Constructor % the object trying to create the resource
				 Type        % the type of the created resource
				 Init
				 Obj)        % object to register as a resource
		  lock Self.block then
		     case {Mode}
		     of n then %% normal mode
			{O.parent.Builder NewResource(Constructor Type Init Obj)}
		     [] r then
			%% is it already in self.resdict ?
			fun{Find L}
			   case L
			   of I#!Obj|Ls then I
			   [] _|Ls then {Find Ls}
			   else unit end
			end
			Idl={Find {Dictionary.entries Self.resdict}}
		     in
			if Idl==unit then
			   %% create this new resource and mark it for the other site to build it also
			   Id={NewName}
			in
			   {Dictionary.put Self.resdict Id
			    Type#{Self translate(Constructor $)}#Obj}
			   Obj={O.parent.Builder NewResource(Constructor Type Init $)}
			else
			   %% create this resource, reconnecting it to the other site corresponding object
			   R
			in
			   {Dictionary.remove Self.resdict Idl}
			   Obj={O.parent.Builder NewResource(Constructor Type Init $)}
			   {Self storeWidget(Idl Obj)}
			   try
			      {Session.aSend Self.session reBind(Idl R)}
			   catch _ then skip end
			   {Session.waitDisconnectOr Self.session R}
			   if {IsDet R} then
			      {ForAll R
			       proc{$ E#L}
				  {ForAll {Reverse L}
				   proc{$ Evt}
				      {Obj {Record.adjoinAt Evt action
					    Self#exec(Idl Evt.action)}}
				   end}
			       end}
			   end
			end
		     end
		  end
	       end
	       meth !Grid(...)=M
		  % send this information to the controlling site so that the geometry can be restored
		  % when the ui migrates
		  if {Mode}==r then
		     try
			{Session.aSend Self.session {Self translate(M $)}}
		     catch _ then skip end
		  end
%		   {Record.map M
%		    fun{$ P} if {Object.is P} then
%				OBC#{Self getId(P $)}
%			     else P end end}}
		  {O.parent.Builder M}
	       end
	       meth otherwise(M)
		  {O.parent.Builder M}
	       end
	    end
	 in
	    O.Builder={New MyBuilder Init}
	 end
      end
      meth recall(Msg)
	 try
	    {Session.aSend self.session Msg}
	 catch _ then skip end
      end
      meth mapLabelToObject(Desc Ob)
	 lock self.block then
	    Om=@mode
	    %% parse the description, replacing handles by
	    %% an entry into self.widgets
	    Map={NewCell nil}
	    fun{Loop D}
	       {Record.mapInd D
		fun{$ I V}
		   case I
		   of action then self#exec(D.handle V)
		   [] handle then
		      E
		   in
		      {Assign Map V#E|{Access Map}}
		      E
		   elseif {Int.is I} andthen {IsDet V} andthen {Record.is V} then
		      {Loop V}
		   else
		      V
		   end
		end}
	    end
	    B={Loop Desc}
	 in
	    mode<-n
	    Ob={self.main.toplevel.Builder
		MapLabelToObject({Record.adjoinAt B parent
				  if {HasFeature Desc parent} then
				     {self getObj(Desc.parent $)}
				  else
				     self.main
				  end
				 } $)}
	    {ForAll {Access Map}
	     proc{$ V#E}
		{self storeWidget(V E)}
	     end}
	    mode<-Om
	 end
      end
      meth set(P)
	 Con
      in
	 lock self.block then
	    TooLate
	 in
	    con<-@con+1
	    if {Session.isConnected self.session} then
	       TooLate
	    in
	       thread
		  {Delay @timeout}
		  TooLate=unit
	       end
	       try
		  {Session.aSend self.session disco}
	       catch _ then skip end
	       {Session.waitNotOkOr self.session TooLate} % not ok means disconnected, tempFail or permFail
	       if {Session.getState self.session}==tempFail orelse {IsDet TooLate} then
		  %% the other site had the opportunity to disconnect nicely
		  %% but the link is down => break it now
		  {Session.break self.session}
	       end
	    end
	    {self disco(false)}
	    if P\=empty then
	       %% first step : ask if it can connect to the other site
	       RCon
	    in
	       {Fault.enable RCon site nil _}
	       thread
		  {Delay @timeout}
		  TooLate=unit
	       end
	       {Session.sideSend self.session P RCon}
	       {WaitOr RCon TooLate}
	       if {IsDet RCon} andthen RCon==true then
		  %% the other side is now supposed to accept a new incoming connection
		  %% second step : connect and ask for the initial data
		  Init
		  TooLate
		  {Session.connect self.session P}
		  Data
	       in
		  {Fault.enable Init site nil _}
		  try
		     {Session.aSend self.session connect(Data)}
		  catch _ then skip end
		  thread
		     {Delay @timeout}
		     TooLate=unit
		  end
		  {Session.waitDisconnectOrs self.session [Data TooLate]}
		  if {Session.getPeer self.session}\=unit then
		     %% connection succeeded
		     if {IsFree Data} then
			%% but didn't answer in the given time
			%% => cancel the connection
			{Session.break self.session}
		     else
			thread
			   N=@con
			in
			   {Session.waitDisconnect self.session}
			   %% in case of disconnection (other site disconnected or permfail)
			   %% clears data
			   lock
			      if @con==N then
				 {self disco(false)}
			      end
			   end
			end
			{self connectTo(Data)}
		     end
		  else
		     %% connection failed, nothing to do
		     skip
		  end
	       end
	    end
	 end
      end
      meth get($)
	 lock self.block then
	    R={Session.getPeer self.session}
	 in
	    if R==unit then empty
	    else
	       R
	    end
	 end
      end
      meth connectTo(Q)
	 lock
	    mode<-n
	    NC NCS
	    Desc#Supp#States#GridInfo#Fonts#Sync=Q
	 in
	    %% builds the fonts
	    
	    {Show step0}
	    {ForAll Fonts
	     proc{$ F}
		{self recFont(F)}
	     end}
	    
	    %% builds the initial state of the GUI from its description
	    
	    {Show step1}
	    NC={self mapLabelToObject(Desc $)}
	    
	    %% for all widgets that were dynamically created in that GUI
	    %% builds it again by repeating the command that originally created it
	    
	    {Show step2}
	    local
	       proc{Loop L R}
		  case L
		  of To#Msg#LDesc|Ls then
		     if {Dictionary.member self.widgets To} then
			W={self getObj(To $)}
			Ob
		     in
			mode<-s(LDesc)
			{W Msg}
%			   {self mapLabelToObject(Desc Ob)}
%			   {Dictionary.put self.widgets Desc.handle Ob}
			{Loop {List.append Ls R} nil}
		     else
			{Loop Ls To#Msg#LDesc|R}
		     end
		  else
		     if R\=nil then {Show incompleteTransfert} end
		  end
	       end
	    in
	       {Loop Supp nil}
	    end
	    
	    %% resources management
	    
	    {Show step3}
% 	       {ForAll Resources
% 		proc{$ R}
% 		   Id#Constructor#Type#InitCall={self translate(R $)}
% 		in
% 		   {self storeWidget(Id
% 				     if Type==ch then _ else
% 					{self.main.Builder NewResource(Constructor Type InitCall $)}
% 				     end)}
% 		end}

	    %% restore geometry of the dynamically built widgets

	    {Show step4}
	    {ForAll GridInfo
	     proc{$ M}
		{ExecTk unit {self translate({Record.adjoin M grid} $)}}
	     end}
	    
	    %% restore the event binding of the widgets
	    
	    {Show step5}
	    {ForAll States
	     proc{$ N#S}
		W={self getObj(N $)}
	     in
		{ForAll S
		 proc{$ E#L}
		    {ForAll {Reverse L}
		     proc{$ Evt}
			{W {Record.adjoinAt Evt action
			    self#exec(N Evt.action)}}
		     end}
		 end}
	     end}
	    
	    {Show step6}
	    child<-NC
	    {ExecTk unit grid(NC row:0 column:0
%				 sticky:{CondSelect Desc glue ""}
			      sticky:nswe
			      padx:{CondSelect Desc padx 0}
			      pady:{CondSelect Desc pady 0})}
	    {Show step7}
	    mode<-r
	    Sync=unit
	 end
      end
      meth exec(...)=M
	 try
	    {Session.aSend self.session M}
	 catch _ then skip end
      end
      meth storeWidget(Id Ob)
	 {Dictionary.put self.widgets Id Ob}
	 {Dictionary.put self.tkmap {VirtualString.toAtom {Tk.getTclName Ob}} Id}
      end
      meth getId(Obj R)
	 T
      in
	 try
	    T=Obj.TclName
	 catch _ then skip end
	 if {IsFree T} then
	    fun{Find L}
	       case L
	       of Id#!Obj|Xs then Id
	       [] _|Xs then {Find Xs}
	       else Obj end
	    end
	 in
	    R={Find {Dictionary.entries self.widgets}}
	 else
	    R={Dictionary.condGet self.tkmap {VirtualString.toAtom T} Obj}
	 end
      end
      meth getObj(Id $)
	 {Dictionary.get self.widgets Id}
      end
      meth removeWidget(Id)
	 Ob={self getObj(Id $)}
      in
	 {Dictionary.remove self.widgets Id}
	 {Dictionary.remove self.tkmap {VirtualString.toAtom {Tk.getTclName Ob}}}
      end
      meth deleteResource(Id)
	 {self removeWidget(Id)}
      end
      meth getImage(Id R)
	 D
	 try
	    {Session.aSend self.session getImage(Id D)}
	 catch _ then skip end
	 {Session.waitDisconnectOr self.session D}
      in
	 if {IsFree D} then
	    {Show 'BAD'}
	    R=gray
	 else
	    R={NewImage D}
	    {self storeWidget(Id R)}
	 end
      end
      meth translate(Msg $)
	 fun{Parse R}
	    if {IsDet R} then
	       case R
	       of !OBC#Id then
		  {self getObj(Id $)}
	       [] !IMG#Id then
		  R
	       in
		  try {self getObj(Id R)} catch _ then
		     {self getImage(Id R)}
		  end
		  R
	       [] !LNK#Id then
		  %% this tag its a new resource that should have the global identifier Id
		  %% the other site already has an object controlling this Id
		  N
	       in
		  {Dictionary.put self.resdict Id N}
		  N
	       else
		  if {Record.is R} then
		     {Record.map R
		      fun{$ T}
			 {Parse T}
		      end}
		  else
		     R
		  end
	       end
	    else
	       N
	    in
	       thread
		  {WaitOr N R}
		  if {IsFree R} then
		     R={ParseOut N}
		  else N={Parse R} end
	       end
	       N
	    end
	 end
	 fun{ParseOut R}
	    if {IsDet R} then
	       if {Object.is R} then
		  T={self getId(R $)}
	       in
		  if R==T then
%		     {Show potentialError#T#T.widgetType#{T get($)}}
%		     {Exception.raiseError qtk(custom "potentialError" T.widgetType#{T get($)} Msg)}
		     if {IsFont R} then
			OBC#{GetFontId R}
		     else
			fun{Find L}
			   case L
			   of Id#(Type#Parent#!R)|Ls then
			      {Dictionary.remove self.resdict Id} % this arrives only once
			      {self storeWidget(Id R)}
			      CRT#(Id#Type#{self getId(Parent $)})
			   [] _|Ls then {Find Ls}
			   else OBC end
			end
		     in
			%% is it a new resource that should be created on the other site ?
			{Find {Dictionary.entries self.resdict}}
		     end
		  else OBC#T end
	       elseif {List.is R} then
		  {List.filter
		   {List.map R
		    fun{$ T}
		       {ParseOut T}
		    end}
		   fun{$ E} E\=OBC end}
	       elseif {Record.is R} then
		  {Record.filter
		   {Record.map R
		    fun{$ T}
		       {ParseOut T}
		    end}
		   fun{$ E} E\=OBC end}
	       else
		  R
	       end
	    else
	       N
	    in
	       thread
		  {WaitOr N R}
		  if {IsFree R} then
		     R={ParseOut N}
		  else N={Parse R} end
	       end
	       N
	    end
	 end
      in
	 {Parse Msg}
      end
      meth recFont(F)
	 case F
	 of add(Id V) then
	    if {Dictionary.member self.widgets Id} then
	       {self recFont(chg(Id {Record.adjoin V set}))}
	    else
	       {self storeWidget(Id {NewFont V})}
	    end
	 [] del(Id) then
	    {self removeWidget(Id)}
	 [] chg(Id V) then
	    Ob={self getObj(Id $)}
	 in
	    if Ob==Id then
	       {self recFont(add(Id {Record.adjoin V font}))}
	    else
	       {Ob V}
	    end
	 end
      end
      meth rec(To Msg Err)
	 W={self getObj(To $)}
	 try
	    case Msg
	    of bind(...) then
	       {W {Record.adjoinAt Msg action self#exec(To Msg.action)}}
	    [] destroy then
	       {self removeWidget(To)}
	       {W destroy}
	    else
	       Msg2={self translate(Msg $)}
	    in
%	       rec<-rec(To Msg2 1)
	       {W Msg2}
	       if @rec\=nil then
		  try
		     {Session.aSend self.session dynaBuild(To Msg2 {Reverse @rec})}
		  catch _ then skip end
		  rec<-nil
	       end
	    end
	 catch X then Err=X end
      in
	 if {IsFree Err} then Err=ok else {Browse Msg#Err} end
      end
      meth disco(CC<=true)
	 lock self.block then
	    if CC andthen {Session.isConnected self.session} then
	       try
		  {Session.aSend self.session disco}
	       catch _ then skip end
	       {Session.disconnect self.session}
	    end
	    if {Not {Session.isConnected self.session}} then
	       if @child\=unit then
		  {Tk.send grid(forget @child)}
		  mode<-n
		  {@child destroy}
	       end
	       child<-unit
	       {Dictionary.removeAll self.widgets}
	       {Dictionary.removeAll self.tkmap}
	       {self.main Disco}
	    end
	 end
      end
%       meth discoL
% 	 if {Session.isConnected self.session} then
% 	    {Session.sSend self.session disco}
% 	    {Session.waitDisconnect self.session}
% 	 end
%       end
   end
   
   class QTkReceiver
   
      from Tk.frame QTkClass

      prop locking

      attr
	 OnDisconnect
	 TempFailure

      feat
	 !Builder
	 widgetType:WidgetType
	 typeInfo:r(all:{Record.adjoin GlobalInitType
			 r(1:no
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
			   height:pixel
			   width:pixel
			   visual:no)}
		    uninit:r
		    unset:{Record.adjoin GlobalUnsetType
			   r('class':unit
			     colormap:unit
			     container:unit
			     visual:unit)}
		    unget:{Record.adjoin GlobalUngetType
			   r(bitmap:unit)})
	 ConObj
	 
      meth !Init(...)=M
	 lock
	    A B
	 in
	    self.ConObj={New ConClass init(self)}
	    {SplitParams M [1] A B}
	    QTkClass,A
	    Tk.frame,{TkInit A}
	    %% B contains the structure of
	    %% creates the children
	    {Tk.batch [grid(rowconfigure self 0 weight:1)
		       grid(columnconfigure self 0 weight:1)]}
	    if {HasFeature B 1} andthen B.1\=empty then
	       {self set(B.1)}
	    end
	    {self bind(event:disconnect)}
	    {self bind(event:tempFailure)}
	 end
      end

      meth bind(event:E ...)=M
	 lock
	    case E
	    of disconnect then
	       OnDisconnect<-{{New QTkAction init(parent:self
						  action:{CondSelect M action proc{$} skip end})} action($)}
	    [] tempFail then
	       TempFailure<-{{New QTkAction init(parent:self
						 action:{CondSelect M action proc{$} skip end})} action($)}
	    else
	       QTkClass,M
	    end
	 end
      end

      meth !TempFail
	 P#M=@TempFailure
      in
	 {Port.send P M}
      end

      meth !Disco
	 P#M=@OnDisconnect
      in
	 {Port.send P M}
      end
      
      meth set(...)=M
	 lock
	    A C
	 in
	    {SplitParams M [1] A C}
	    QTkClass,A
	    if {HasFeature C 1} then
	       {self.ConObj set(C.1)}
	    end
	 end
      end

      meth get(...)=M
	 lock
	    A B
	 in
	    {SplitParams M [1] A B}
	    QTkClass,A
	    {Assert self.widgetType self.typeInfo B}
	    {CondSelect B 1 _}={self.ConObj get($)}
	 end
      end
      
      meth destroy
	 lock
	    {self.ConObj set(empty)}
	 end
      end

   end

   Register=[r(widgetType:WidgetType
	       feature:Feature
	       widget:QTkReceiver)]
   
end
