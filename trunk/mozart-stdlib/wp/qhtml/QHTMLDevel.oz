functor
   
import
   QUI
   QHTMLType(marshall:Marshall
	     unmarshall:Unmarshall
%	     existType:ExistType
%	     existTrans:ExistTrans
	     existEvent:ExistEvent
	     event2OZ:Event2OZ
	     checkType:CheckType)
   Params
%   System(show:Show)

export
   Quote
   QuoteSingle
   QuoteDouble
   RemoveQuote
   DefineWidget
   BaseClass
   DataVal DataInfo GetId InitDataInfo Events
   BuildInnerHTML BuildHTML Undefined
   OnConnect OnDisconnect
   Set Get Send Return Configure Cget Sync
   ProcessEvent ExecEvent ReSetCustom SetTooltips
   Join
   CheckType
   QInit
   GetWidDef

define

   {Wait CheckType}
   GetParam
   GetEvent
   local
      ParamsD={NewDictionary}
      EventsD={NewDictionary}
   in
      {ForAll Params.data
       proc{$ K#V}
	  ParamsD.K:=V
       end}
      {ForAll Params.events
       proc{$ K#V}
	  EventsD.K:=V
       end}
      fun{GetParam N}
	 {Dictionary.condGet ParamsD N nil}
      end
      fun{GetEvent N}
	 {Dictionary.condGet EventsD N nil}
      end
   end
   %% global names
   SharedName
   [DataVal DataInfo GetId Events InitDataInfo
    BuildInnerHTML BuildHTML Undefined
    OnConnect OnDisconnect
    Set Get Send Return Configure Cget Sync
    ProcessEvent ExecEvent ReSetCustom SetTooltips]=SharedName
   
   {ForAll SharedName proc{$ V} V={NewName} end}
   QInit=QUI.qInit

   Bad={NewName}
   PSend=Port.send
   %% misc procedures

   fun{Join L Sep}
      Len={Length L}
   in
      {VirtualString.toString
       {List.foldLInd L
	fun{$ I Old E}
	   Old#E#if I\=Len then Sep else "" end
	end ""}}
   end

   %% base class for html widgets to inherit from

   class BaseClass
      prop locking
      meth !QInit(...) skip end
      meth set(...) skip end
      meth get(...) skip end
      meth bind(...) skip end
      meth !BuildInnerHTML($)
	 ""
      end
      meth !OnConnect
	 skip
      end
      meth !OnDisconnect
	 skip
      end
      meth !Sync
	 skip
      end
      meth close skip end
   end

   fun{RemoveQuote Str1}
      Str={VirtualString.toString Str1}
   in
      case Str of &"|_ andthen {List.last Str}==&" then
	 {List.drop {List.take Str {Length Str}-1} 1}
      else Str end
   end

   %% define widget procedure, mixing classes and registering to QUI

   WidDef={NewDictionary}

   fun{GetWidDef T}
      WidDef.T
   end
   
   proc{DefineWidget Info MixClass}
      IsContainer={Record.some Info.desc fun{$ P} P==container end}
%      fun{GetDataVal Obj K}
%	 Obj.DataVal.K
%      end
   in
      {Dictionary.put WidDef {Label Info.desc} Info.desc}
      {QUI.buildWidget Info.desc
       fun{$ Name QUIMixClass}
	  class C
	     from MixClass QUIMixClass
	     feat
		widgetId:{Label Info.desc}
		!DataVal
		!DataInfo:{NewDictionary}
		!Events
	     meth !QInit(...)=M
		Remain Contained Init
		Top=if M.parent==unit then self else {M.parent get(toplevel:$)} end
	     in
		self.DataVal={NewDictionary}
		self.Events={NewDictionary}
%		self.DataInfo={NewDictionary}
%		{self InitDataInfo}
		% init QUI object
		Init=if IsContainer then
			{QUI.partitionContained M Contained $}
		     else
			Contained=QInit
			M
		     end
		% init default parameters
		{ForAll {Dictionary.entries self.DataInfo}
		 proc{$ K#V}
		    if {HasFeature V default} then
		       {Dictionary.put self.DataVal K V.default}
		    end
		 end}
		% define tagName
		{self Set(tagname Info.tag)}
		% define HTML id
		local
		   ID={Top GetId(self $)}
		   CPI=if M.parent==unit then "" else {M.parent get(childPrefixId:$)} end
		in
		   {self Set(id ID)}
		   {self Set(childPrefixId CPI)}
		   {self Set(refId if CPI=="" then ID else CPI#"."#ID end)}
		end
		% widget is not yet displayed
		{self Set(isdisplayed false)}
		% QUI constructor
		QUIMixClass,{Record.adjoinAt Init QUI.remainingParameters Remain}
		% check parameters
		{self Check(Remain)}
		% store user defined parameters
		{self Store(Init)}
		% give hand for other parameters
		local
		   Custom={Record.filterInd Remain
			   fun{$ K _}
			      {CondSelect {Dictionary.get self.DataInfo K} init outer}==custom(true)
			   end}
		in
		   MixClass,{Record.adjoin Custom Contained}
		end
	     end
	     meth set(...)=M
		Remain
	     in
		QUIMixClass,{Record.adjoinAt M QUI.remainingParameters Remain}
		{self Check(Remain)}
		{self Store(M)}
		local
		   Top={self get(toplevel:$)}
		   Con={self get(isdisplayed:$)}
		   Custom={Record.filterInd Remain
			   fun{$ K V}
			      I={Dictionary.get self.DataInfo K}
			      T={CondSelect I set outer}
			   in
			      if T==custom(true) then
				 true
			      else
				 if Con then
				    {Top configure(self
						   if T==style(true) then "style."#K else K end
						   {Marshall self I V})}
				 end
				 false
			      end
			   end}
		in
		   MixClass,Custom
		end
	     end
	     meth get(...)=M
		Remain
	     in
		QUIMixClass,{Record.adjoinAt M QUI.remainingParameters Remain}
		{self Check(Remain)}
		if Remain\=get then
		   Top={self get(toplevel:$)}
		   Con
		   Custom={Record.filterInd Remain
			   fun{$ K V}
			      I={Dictionary.get self.DataInfo K}
			      T={CondSelect I get internal(true)}
			   in
			      case T
			      of internal(true) then
				 V={self Get(K $)}
				 false
			      [] custom(true) then
				 true
			      else
				 if {IsFree Con} then Con={self get(isdisplayed:$)} end
				 if Con then
				    V={Unmarshall self I
				       {Top cget(self
						 if T==style(true) then "style."#K else K end
						 $)}}
				    {self Set(K V)}
				 else
				    V={self Get(K $)}
				 end
				 false
			      end
			   end}
		in
		   MixClass,Custom
		end
	     end
	     meth bind(event:E action:A args:P<=nil)=M
		%% events are a bit tricky because of their parameters
		%% qhtml's user can bind events over and over with different parameters
		%% and due to lag, the notofications from the page can be with different parameters
		%% than those requested
		%% solution used here :
		%% when a notification is received from the web page, along with paramaters specification
		%% if the notification is complete enough according to the event info on the Oz side
		%%    then the event is raised, otherwise it is dropped.
		{CheckType E atom M}
		I={Dictionary.condGet self.DataInfo E r}
	     in
		if {Not {HasFeature I bind}} orelse {Not {CondSelect I.bind 1 true}} then
		   {QUI.raiseError self.widgetId "Invalid event : "#E M}
		else
		   {CheckType A event M}
		   {CheckType P list M}
		   Port#Msg={{self get(actionManager:$)} getAction(A $)}
		   self.Events.E:=Port#Msg#P
		in
		   if {Label I.bind}==custom then
		      %% custom events
		      MixClass,bind(event:E action:Port#Msg args:P)
		   else
		      %% standard events
		      {ForAll P proc{$ E}
				   if {Not {Atom.is E}} orelse {Not {ExistEvent E}} then {QUI.raiseError self.widgetId "Invalid argument for event : "#E M} end
				end}
		      Id={self get(id:$)}
		      HTMLAct="top.catchevent(event,'"#Id#"','"#E#"','"#{Join P ","}#"')"
		      Rec={Record.adjoinAt bind E HTMLAct}
		      {self Store(Rec)}
%		      self.Events.E:=Port#Msg#P
		      Top={self get(toplevel:$)}
		   in
		      if {self Get(isdisplayed $)} then
			 %% this method was supposed to work but didn't :(
%			 {Top configure(self
%					E
%					HTMLAct)}
			 %% using another method instead
			 {self Sync}         % synchronize state
			 {self OnDisconnect} % simulate disconnection
			 {Top configure(self
					outerHTML
					'"'#{QuoteDouble {self BuildOuterHTML($)}}#'"'
				       )}
			 {self OnConnect}   % simulate connection
		      end
		   end
		end
	     end
	     meth !ProcessEvent(Str)
		L R
		{List.takeDropWhile Str fun{$ C} C\=&, end L R}
		Port#Msg#P={Dictionary.condGet self.Events {String.toAtom L} unit#unit#Bad}
	     in
		if P\=Bad then
		   fun{Loop Str1}
		      Str=case Str1 of &,|Xs then Xs else Str1 end
		   in
		      if Str==nil then nil
		      else
			 L R V Remain
			 {List.takeDropWhile Str fun{$ C} C\=&: end L R}
			 {List.takeDropWhile {List.drop R 1} fun{$ C} C\=&, end V Remain}
			 Code={String.toAtom L}
		      in
			 Code#{Event2OZ V Code}|{Loop Remain}
		      end
		   end
		   Received={List.toRecord r {Loop R}}
		   Result={List.map P
			   fun{$ R} {CondSelect Received R Bad} end}
		in
		   if {List.some Result fun{$ V} V==Bad end} then skip %% event is dropped, bad parameters received
		   else
		      {PSend Port {Record.adjoin {List.toTuple r Result} Msg}}
		   end
		end
	     end
	     meth !ExecEvent(E Params<=r)
		if {Dictionary.member self.Events E} then
		   Port#Msg#_=self.Events.E
		in
		   {PSend Port {Record.adjoin Params Msg}}
		else skip end
	     end
	     meth Store(M)
		{Record.forAllInd M
		 proc{$ P V}
		    {self Set(P V)}
		 end}
	     end
	     meth !InitDataInfo
% 		{ForAll [tagname#r(type:atom
% 				   init:custom(false)
% 				   set:custom(false)
% 				   get:internal(true))
% 			 id#r(type:atom
% 			      init:outer(false)
% 			      set:outer(false)
% 			      get:internal(true))
% 			 childPrefixId#r(type:atom
% 					 init:custom(false)
% 					 set:custom(false)
% 					 get:internal(true))
% 			 refId#r(type:atom
% 				 init:custom(false)
% 				 set:custom(false)
% 				 get:internal(true))
% 			 isdisplayed#r(type:bool
% 				       init:custom(false)
% 				       set:custom(false)
% 				       get:internal(true))
% 			 width#r(type:widthlength
% 				 default:glue
% 				 init:style(true)
% 				 set:style(true)
% 				 get:internal(true))
% 			 height#r(type:heightlength
% 				  default:glue
% 				  init:style(true)
% 				  set:style(true)
% 				  get:internal(true))
% 			 title#r(type:html
% 				 init:outer(false)
% 				 set:outer(false)
% 				 get:internal(false))
% 			]
% 		 proc{$ K#V}
% 		    self.DataInfo.K := V
% 		 end}
% 		{ForAll Info.params
% 		 proc{$ K#V}
% 		    self.DataInfo.K := V
% 		 end}
% 		{ForAll Info.events
% 		 proc{$ K#V}
% 		    self.DataInfo.K := {Record.adjoinAt V type event}
% 		 end}
% 		{ForAll {Dictionary.entries self.DataInfo}
% 		 proc{$ K#V}
% 		    T={CondSelect V type no}
% 		 in
% 		    if {Not {List.is T}} andthen {Not {ExistType T}} then
% 		       raise typeDoesNotExist(self.widgetId K V) end
% 		    end
% 		 end}
		{ForAll {GetParam {Label Info.desc}}
		 proc{$ K#V}
		    self.DataInfo.K:=V
		 end}
		{ForAll {GetEvent {Label Info.desc}}
		 proc{$ K}
		    self.DataInfo.K:=r(bind:outer(true)
				       type:event)
		 end}
		{ForAll [tagname#r(type:atom
				   init:custom(false)
				   set:custom(false)
				   get:internal(true))
			 id#r(type:atom
			      init:outer(false)
			      set:outer(false)
			      get:internal(true))
			 childPrefixId#r(type:atom
					 init:custom(false)
					 set:custom(false)
					 get:internal(true))
			 refId#r(type:atom
				 init:custom(false)
				 set:custom(false)
				 get:internal(true))
			 isdisplayed#r(type:bool
				       init:custom(false)
				       set:custom(false)
				       get:internal(true))
			 width#r(type:widthlength
				 default:glue
				 init:style(true)
				 set:style(true)
				 get:internal(true))
			 height#r(type:heightlength
				  default:glue
				  init:style(true)
				  set:style(true)
				  get:internal(true))
			 title#r(type:html
				 init:outer(false)
				 set:outer(false)
				 get:internal(false))
			]
		 proc{$ K#V}
		    self.DataInfo.K := V
		 end}
		%%%%%%%%%%%%%%%%%%%%%%%%%
 		{ForAll Info.params
 		 proc{$ K#V}
 		    self.DataInfo.K := V
 		 end}
 		{ForAll Info.events
 		 proc{$ K#V}
 		    self.DataInfo.K := {Record.adjoinAt V type event}
 		 end}
	     end
	     meth Check(M)
		L={Label M}
	     in
		{Record.forAllInd M
		 proc{$ I V}
		    if {Dictionary.member self.DataInfo I} then
		       F=self.DataInfo.I
		       A={CondSelect F L outer(true)}
		    in
		       case L
		       of get then
			  if F.type==event orelse {Not A.1} then
			     {QUI.raiseError self.widgetId "Invalid operation for parameter : "#I M}
			  end
%		       [] bind then
%			  {CheckType V event M}
		       else
			  if A.1 andthen F.type\=event then
			     {CheckType V F.type M}
			  else
			     {QUI.raiseError self.widgetId "Invalid operation for parameter : "#I M}
			  end
		       end
		    else
		       {QUI.raiseError self.widgetId "Invalid parameter name : "#I M}
		    end
		 end}
	     end
	     meth !OnConnect
		Top={self get(toplevel:$)}
	     in
		{self Set(isdisplayed true)}
		{ForAll  {Dictionary.entries self.DataVal}
		 proc{$ K#V}
		    I={Dictionary.condGet self.DataInfo K r(set:outer)}
		    T={Label {CondSelect I set outer}}
		 in
		    if T==html then
		       {Top configure(self
				      if T==style(true) then "style."#K else K end
				      {Marshall self I V})}
		    end
		 end}
		MixClass,OnConnect
	     end
	     meth !OnDisconnect
		{self Set(isdisplayed false)}
		MixClass,OnDisconnect
	     end
	     meth !Set(P V)
		if {IsDet V} andthen (V==Undefined) then
		   {Dictionary.remove self.DataVal P}
		else
		   self.DataVal.P:=V
		end
	     end
	     meth !Get(P V)
		V={Dictionary.condGet self.DataVal P Undefined}
	     end
	     meth !ReSetCustom
		%% this methods sets back all parameters that are custom AND not undefined. Usefull in OnConnect for all custom widgets
		%% that are defined inside the set method only
		{ForAll  {Dictionary.entries self.DataVal}
		 proc{$ K#V}
		    I={Dictionary.condGet self.DataInfo K r(set:outer)}
		    T={CondSelect I set outer}
		 in
		    if T==custom(true) then MixClass,set(K:V) end
		 end}
	     end
	     meth BuildOuterHTML($)
		"<"#{self get(tagname:$)}#" "#{self BuildStyle($)}#" "#
		{Join
		 {List.map
		  {List.filter
		   {List.map {Dictionary.entries self.DataVal}
		    fun{$ K#V}
		       Info={Dictionary.condGet self.DataInfo K r(set:custom)}
		    in
		       K#V#Info
		    end}
		   fun{$ _#_#I}
		      if {HasFeature I bind} then {Label I.bind}\=custom
		      else {Label {CondSelect I set outer}}==outer
		      end
		   end}
		  fun{$ K#V#M}
		     K#'='#{Marshall self M V}
		  end}
		 " "}#
		">"
	     end
	     meth BuildStyle($)
		R='style="'#{Join
			     {List.map
			      {List.filter		
			       {List.map {Dictionary.entries self.DataVal}
				fun{$ K#V}
				   Info={Dictionary.condGet self.DataInfo K r(set:custom)}
				in
				   K#V#{Label {CondSelect Info set outer}}#Info
				end}
			       fun{$ _#_#I#_}
				  I==style
			       end}
			      fun{$ K#V#_#M}
				 {CondSelect M attribute K}#":"#{RemoveQuote {Marshall self M V}}

% 				 case K
% 								of width then if V==glue then
% 										 if self.DataVal.glue.exph then "100%" else "auto" end
% 									      else {Show M#V} {Marshall self M V} end
% 								[] height then if V==glue then
% 										  if self.DataVal.glue.expv then "100%" else "auto" end
% 									       else {Marshall self M V} end
% 								else {Marshall self M V} end
			      end}
			     "; "}#'"'
	     in
		R
	     end
	     meth BuildClosureHTML($)
		"</"#{self get(tagname:$)}#">"
	     end
	     meth !BuildHTML($)
		{self BuildOuterHTML($)}#
		'\n'#
		{self BuildInnerHTML($)}#
		'\n'#
		{self BuildClosureHTML($)}
	     end
	     meth !SetTooltips(V)
		{self Set(title V)}
		if {self Get(isdisplayed $)} then
		   {{self get(toplevel:$)} configure(self
						     title
						     {Marshall self title V})}
		end
	     end
	     meth !Send(V)
		if {self Get(isdisplayed $)} then
		   {{self get(toplevel:$)} send(V)}
		end
	     end
	     meth !Return(V R)
		if {self Get(isdisplayed $)} then
		   {{self get(toplevel:$)} return(V R)}
		end
	     end
	     meth !Configure(Id V)
		if {self Get(isdisplayed $)} then
		   {{self get(toplevel:$)} configure(self Id V)}
		end
	     end
	     meth !Cget(Id V)
		if {self Get(isdisplayed $)} then
		   {{self get(toplevel:$)} cget(self Id V)}
		end
	     end
	     meth !Sync
		if {self Get(isdisplayed $)} then
%		   Top={self get(toplevel:$)}
%		in
		   {ForAll  {Dictionary.entries self.DataVal}
		    proc{$ K#_}
		       I={Dictionary.condGet self.DataInfo K r(set:outer)}
		       T={CondSelect I get outer(true)}
		    in
		       if ({Label T}\=internal) andthen T.1
			  andthen {Not {HasFeature I bind}}
		       then
			  {self get(K:_)}
		       end
		    end}
		end
		MixClass,Sync
	     end
	  end
       in
	  {New C InitDataInfo _}
	  C
       end}
   end

   %% widgets definition

   fun{QuoteDouble Str}
      fun{Loop Str}
	 case Str
	 of &\n|Cs then {Loop Cs}
	 [] &"|Cs then &\\|&\\|&"|{Loop Cs}
	 [] &'|Cs then &\\|&'|{Loop Cs}
	 [] C|Cs then C|{Loop Cs}
	 else nil end
      end
   in
      {Loop {VirtualString.toString Str}}
   end

   fun{QuoteSingle Str}
      fun{Loop Str}
	 case Str
	 of &\n|Cs then {Loop Cs}
	 [] &"|Cs then &\\|&"|{Loop Cs}
	 [] &'|Cs then &\\|&\\|&'|{Loop Cs}
	 [] C|Cs then C|{Loop Cs}
	 else nil end
      end
   in
      {Loop {VirtualString.toString Str}}
   end

   fun{Quote Str}
      fun{Loop Str}
	 case Str
	 of &\n|Cs then {Loop Cs}
	 [] &"|Cs then &\\|&"|{Loop Cs}
	 [] &'|Cs then &\\|&'|{Loop Cs}
	 [] C|Cs then C|{Loop Cs}
	 else nil end
      end
   in
      {Loop {VirtualString.toString Str}}
   end

end
   
      

   
