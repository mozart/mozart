functor

import
   Error
   
export
   NoGlue
   GetGlue
   ToArray
   CalcLR
   ContinueLength
   RaiseError
   CheckFree
   AddLook
   NewLook
   SetAlias
   GetAlias
   UnSetAlias
   ToDescGlue
   actionManager:ActionManagerClass
   Widget
   RegisterWidget
   GetWidget
   GetWidgetList
   MissingWidgetError
   OnUnknownWidget
   Build
   FilterContained
   PartitionContained
%   SubtractAutoParams
%   SplitAutoParams
%   AutoParams
   AddParameterSupport
   AddActionSupport
   AddReturnSupport
   AddContainerSupport
   RemainingParameters
   ParameterNotSpecified
   AddTooltipsSupport
   BuildBaseWidget
   BuildWidget
   IsEvent
   QInit
   MDestroy MBuildChild MDestroyChild MGetChildren MSetState MGetState
   
define
   ML
   [MDestroy MBuildChild MDestroyChild MGetChildren MSetState MGetState]=ML
   
   {ForAll ML
    proc{$ V} V={NewName} end}

   NoGlue=r(w:false e:false s:false n:false exph:false expv:false)
   QInit={NewName}

   fun{GetGlue D}
      %% transforms a glue record parameter into a record
      %% (see what function returns)
      CG={CondSelect D glue ''}
   in
      if {Atom.is CG} then
	 St={VirtualString.toString CG}
	 W={List.member &w St}
	 E={List.member &e St}
	 N={List.member &n St}
	 S={List.member &s St}
      in
	 r(w:W e:E n:N s:S exph:W andthen E expv:N andthen S)
      else
	 CG
      end
   end

   fun{ToArray Rec}
      %% splits a container record into a list of lists
      %% splitting at newline .
      fun{ParseLine Line Remaining}
	 case Line
	 of newline(...)|Xs then
	    Remaining=Xs nil
	 [] X1|Xs then
	    X={Record.adjoinAt X1 glue {GetGlue X1}}
	 in
	    X|{ParseLine Xs Remaining}
	 else
	    Remaining=nil nil
	 end
      end
      fun{Loop L}
	 Rs
	 R={ParseLine L Rs}
      in
	 if R==nil then nil
	 else R|{Loop Rs}
	 end
      end
      Raw={Loop {List.map
		 {List.filter
		  {Record.toListInd Rec}
		  fun{$ I#_} {IsInt I} end}
		 fun{$ _#E} E end}}
      %% Make all lines the same length
      Len={List.foldL Raw fun{$ Old Line} {Max Old {Length Line}} end 0}
   in
      {List.map Raw
       fun{$ Line}
	  {List.append Line
	   {List.map {List.make Len-{Length Line}}
	    fun{$ E} E=empty(glue:NoGlue) end}}
       end}
   end

   fun{CalcLR Data Horiz}
      %% calculate rows and columns weight
      %% (i.e. when container is resized : either if a row/column must be resized or not)
      if Data==nil then r(h:nil v:nil horiz:Horiz)
      else
	 XD#YD=if Horiz then exph#expv else expv#exph end
	 %% if at least a widget exapnds itselfs in a direction :
	 %% all row/column containing at least a widget expanding itself
	 %% => row/column expand too, otherwise don't expand
	 fun{GetXExpand Data}
	    X1={List.make {Length {List.nth Data 1}}}
	    proc{ParseLine Line X}
	       case Line of E|Es then
		  case X of XX|XXs then
		     if E.glue.XD then XX=true end
		     {ParseLine Es XXs}
		  end
	       else skip end
	    end
	 in
	    {ForAll Data proc{$ Line} {ParseLine Line X1} end}
	    {ForAll X1 proc{$ XX} if {IsFree XX} then XX=false end end}
	    X1
	 end
	 fun{GetYExpand Data}
	    {List.map Data
	     fun{$ Line}
		{List.some Line
		 fun{$ E} E.glue.YD end}
	     end}
	 end
	 %% no widget expands itself. If there are constraints on both edges
	 %% of a row/column, it must NOT expand itself (it would be unable to satisfy constraints)
	 %% in other cases, it must expands itself to take all available space of the window.
	 fun{CalcXExpand Data}
	    X1={List.make {Length {List.nth Data 1}}+1}
	    proc{Loop1 Line X}
	       case Line of E|Es then
		  XL|XR|_=X
		  _|XXs=X
	       in
		  if (Horiz andthen (E.glue).w)
		     orelse ({Not Horiz} andthen (E.glue).n) then
		     XL=unit
		  end
		  if (Horiz andthen E.glue.e)
		     orelse ({Not Horiz} andthen E.glue.s) then
		     XR=unit
		  end
		  {Loop1 Es XXs}
		  skip
	       else skip end
	    end
	    fun{Loop2 L}
	       case L
	       of X1|X2|Xs then
		  {Not ({IsDet X1} andthen {IsDet X2})}|{Loop2 X2|Xs}
	       else nil end
	    end
	 in
	    {ForAll Data proc{$ Line} {Loop1 Line X1} end}
	    {Loop2 X1}
	 end
	 fun{CalcYExpand Data}
	    Y1={List.make {Length Data}+1}
	    proc{Loop1 Lines Y}
	       case Lines of Line|Ls then
		  YT|YB|_=Y
		  _|YYR=Y
	       in
		  if Horiz then
		     if {List.some Line fun{$ E} E.glue.n end} then YT=unit end
		     if {List.some Line fun{$ E} E.glue.s end} then YB=unit end
		  else
		     if {List.some Line fun{$ E} E.glue.w end} then YT=unit end
		     if {List.some Line fun{$ E} E.glue.e end} then YB=unit end
		  end
		  {Loop1 Ls YYR}
	       else skip end
	    end
	    fun{Loop2 L}
	       case L
	       of X1|X2|Xs then
		  {Not ({IsDet X1} andthen {IsDet X2})}|{Loop2 X2|Xs}
	       else nil end
	    end
	 in
	    {Loop1 Data Y1}
	    {Loop2 Y1}
	 end	
	 X1={GetXExpand Data}
	 Y1={GetYExpand Data}
	 X=if {List.all X1 Not} then
	      {CalcXExpand Data} else X1 end
	 Y=if {List.all Y1 Not} then
	      {CalcYExpand Data} else Y1 end
	 Result=if Horiz then r(h:X
				v:Y
				horiz:Horiz)
		else r(h:Y
		       v:X
		       horiz:Horiz)
		end
      in
	 Result
      end
   end

   fun{ContinueLength R}
      %% returns the number of continue in the R list
      case R of continue(...)|Rs then {ContinueLength Rs}+1 else 0 end
   end

   {Error.registerFormatter mbt
    fun {$ E}
       T = 'Error: QUI module'
    in
       case E
       of mbt(generalError Widget Msg Rec) then
	  error(kind:T
		msg:Msg
		items:[hint(l:'Entity'
			    m:oz(Widget))
		       hint(l:'Operation'
			    m:oz(Rec))
		      ])
       [] mbt(unknownWidget Widget) then
	  error(kind:T
		msg:"Unknown widget"
		items:[hint(l:'Widget'
			    m:oz(Widget))
		      ])
       end
    end}
   
   proc{RaiseError Widget Msg Rec}
      {Exception.raiseError
       mbt(generalError Widget Msg {Record.subtract Rec RemainingParameters})}
   end

   proc{CheckFree Widget Rec}
      if {Record.some Rec IsDet} then
	 {RaiseError Widget "Method get requires free variables only" Rec}
      end
   end

   proc{CheckFreeParam Widget P Rec}
      if {IsDet P} then
	 {RaiseError Widget "Method get requires free variables only" Rec}
      end
   end
   
   fun{AddLook Widget M}
      if {CondSelect M look unit}==unit then
	 M
      else
	 {Record.adjoin {M.look.get Widget} M}
      end
   end
   
   fun{NewLook}
      Look={NewDictionary}
      proc{Set P}
	 if {Record.is P} then
	    {Dictionary.put Look {Label P} P}
	 else
	    {RaiseError look "Can define look for records only" P}
	 end
      end
      fun{Get P}
	 {Dictionary.condGet Look {Label P} nolook}
      end
   in
      look(set:Set get:Get)
   end

   fun{ToDescGlue G}
      {VirtualString.toAtom
       if G.n then "n" else "" end#
       if G.s then "s" else "" end#
       if G.w then "w" else "" end#
       if G.e then "e" else "" end}
   end

   ActionFeat={NewName}

   fun{IsEvent A}
      if {Procedure.is A} then true
      elsecase A
      of toplevel#_ then true
      [] X#_ then
	 if {Object.is X} then true
	 elseif {Port.is X} then true
	 else false
	 end
      else false end
   end

   class ActionManagerClass
      feat ActionPort Stop Toplevel
      prop locking
      meth init(T)
	 Out
	 proc{Loop L}
	    {WaitOr L self.Stop}
	    if {IsDet self.Stop} then skip else
	       case L
	       of X|Xs then
		  {self Exec(X)}
		  {Loop Xs}
	       end
	    end
	 end
      in
	 lock
	    self.Toplevel=T
	    self.ActionPort={NewPort Out}
	 end
	 thread
	    {Loop Out}
	 end
      end
      meth getAction(M $)
	 self.ActionPort#action(ActionFeat:M)
      end
      meth exec(Act)
	 {self Exec(action(ActionFeat:Act))}
      end
      meth Exec(Act)
	 lock
	    fun{Filter Xs}
	       {Record.adjoin {Record.subtract Act ActionFeat} Xs}
	    end
	 in
	    if {Procedure.is Act.ActionFeat} then
	       {Procedure.apply Act.ActionFeat {Record.toList {Record.subtract Act ActionFeat}}}
	    elsecase Act.ActionFeat
	    of toplevel#Xs then {self.Toplevel {Filter Xs}}
	    [] X#Xs then
	       if {Object.is X} then {X {Filter Xs}}
	       elseif {Port.is X} then {Send X {Filter Xs}}
	       else {RaiseError action "Bad action format" Act.ActionFeat}
	       end
	    else {RaiseError action "Bad action format" Act.ActionFeat} end
	 end
      end
      meth execSync(M)
	 {Send self.ActionPort action(ActionFeat:M)}
      end
      meth stopService
	 lock
	    self.Stop=unit
	 end
      end
   end

   class GroupManagerClass
      feat
	 Toplevel
	 Groups
      prop locking
      meth init(T)
	 self.Toplevel=T
	 self.Groups={NewDictionary}
      end
      meth addMembers(Id Obj)
	 lock
	    E={Dictionary.condGet self.Groups Id
	       r(items:nil active:nil)}
	 in
	    {Dictionary.put self.Groups Id
	     r(items:Obj|E.items
	       active:E.active)}
	 end
      end
      meth getMembers(Id $)
	 lock
	    {Dictionary.condGet self.Groups Id r(items:nil)}.items
	 end
      end
      meth getActive(Id $)
	 lock
	    {Dictionary.condGet self.Groups Id r(active:nil)}.active
	 end
       end
       meth autoSetActive(Id Obj)
	  lock
	     E={Dictionary.condGet self.Groups Id r(items:nil active:nil)}
	  in
	     {Dictionary.put self.Groups Id {Record.adjoinAt E active Obj}}
	     if E.active\=nil andthen E.active\=Obj then
		{E.active setState(false)}
	     end
	  end
       end
       meth setActive(Id Obj)
	  lock
	     {Dictionary.put self.Groups Id
	      {Record.adjoinAt
	       {Dictionary.condGet self.Groups Id r(items:nil active:nil)}
	       active Obj}}
	  end
       end
   end

   RemainingParameters={NewName}
   ToplevelWidget={NewName}
   
   class Widget
      feat
	 Parent
	 Desc
	 widgetId:undefined
	 ActionManager
	 GroupManager
	 Toplevel
	 !ToplevelWidget:false
      attr
	 destroy:unit
      meth !QInit(...)=M
	 if {IsDet self.Desc} then
	    {RaiseError self.widgetId
	     "Widget already initialized"
	     {Record.adjoin M self.widgetId}}
	 end
	 if {Not {HasFeature M parent}} then
	    {RaiseError self.widgetId
	     "Internal error : Missing parent parameter"
	     {Record.adjoin M self.widgetId}}
	 else
	    self.Parent=M.parent
	    if self.Parent==unit then %% toplevel widget
	       if self.ToplevelWidget==false then
		  {RaiseError self.widgetId
		   "Widget is not a toplevel widget"
		   {Record.adjoin M self.widgetId}}
	       end
	       self.Toplevel=self
	       self.ActionManager={New ActionManagerClass init(self)}
	       self.GroupManager={New GroupManagerClass init(self)}
	    else
	       if self.ToplevelWidget then
		  {RaiseError self.widgetId
		   "Widget is a toplevel widget and can't be put into a container"
		   {Record.adjoin M self.widgetId}}
	       end
	       {self.Parent get(toplevel:self.Toplevel
				groupManager:self.GroupManager
				actionManager:self.ActionManager)}
	    end
	 end
	 R={Record.filterInd M
	    fun{$ P V}
	       D in
	       case P
	       of parent then skip
	       [] handle then V=self
	       [] glue then skip
	       [] padx then skip
	       [] pady then skip
	       [] feature then (self.Parent).V=self
	       [] look then skip
	       [] !RemainingParameters then skip
	       else D=unit end
	       {IsDet D}
	    end}
      in
	 if {HasFeature M RemainingParameters} then
	    M.RemainingParameters=R
	 else
	    {Record.forAllInd R
	     proc{$ P V}
		{RaiseError self.widgetId "Unknown parameter "#P {Record.adjoin M self.widgetId}}
	     end}
	 end
	 self.Desc=M
      end
      meth set(...)=M
	 if {HasFeature M RemainingParameters} then
	    M.RemainingParameters={Record.subtract M RemainingParameters}
	 else
	    {Record.forAllInd M
	     proc{$ P V}
		{RaiseError self.widgetId "Unknown or creation time only parameter "#P M}
	     end}
	 end
      end
      meth get(...)=M
	 {CheckFree self.widgetId M}
	 R
      in
	 R={Record.filterInd M
	    fun{$ P V}
	       D in
	       V=case P
		 of parent then self.Parent
		 [] handle then self
		 [] glue then {ToDescGlue self.Desc.glue}
		 [] padx then {CondSelect self.Desc padx 0}
		 [] pady then {CondSelect self.Desc pady 0}
		 [] feature then {CondSelect self.Desc feature unit}
		 [] look then {CondSelect self.Desc look unit}
		 [] toplevel then self.Toplevel
		 [] actionManager then self.ActionManager
		 [] groupManager then self.GroupManager
		 [] !RemainingParameters then _
		 else D=unit _ end
	       {IsDet D}
	    end}
	 if {HasFeature M RemainingParameters} then
	    M.RemainingParameters=R
	 else
	    {Record.forAllInd R
	     proc{$ P V}
		{RaiseError self.widgetId "Unknown parameter "#P M}
	     end}
	 end
      end
      meth bind(event:E action:A ...)=M
	 case E
	 of destroy then
	    destroy<-A
	 else
	    {RaiseError self.widgetId "Unknown event "#E M}
	 end
      end
      meth !MDestroy
	 %% raise destroy event
	 if @destroy\=unit then
	    {self.ActionManager exec(@destroy)}
	 end
      end
	 
   end

   %% composite functions adding features to base widget

   ParameterNotSpecified={NewName}
   
   fun{AddParameterSupport
       WidgetClass ParameterName
       Features
       InitProc SetProc GetProc DestroyProc}
      FromClass={Class.new [WidgetClass] q Features [locking]}
   in
      class $
	 from FromClass
	 meth !QInit(...)=M
	    Remain in
	    WidgetClass,{Record.adjoinAt
			 {Record.subtract M ParameterName} RemainingParameters Remain}
	    {InitProc self {CondSelect M ParameterName ParameterNotSpecified}}
	    {self CheckRemain(M Remain)}
	 end
	 meth CheckRemain(M Remain)
	    if {HasFeature M RemainingParameters} then
	       if {IsFree Remain} then
		  {RaiseError self.widgetId "Internal error : remain not defined" M}
	       end
	       M.RemainingParameters=Remain
	    else {Record.forAllInd Remain
		  proc{$ P V}
		     {RaiseError self.widgetId "Unknown or creation time only parameter "#P M}
		  end}
	    end
	 end	    
	 meth set(...)=M
	    Remain
	    Rec={Record.adjoinAt
			 {Record.subtract M ParameterName} RemainingParameters Remain}
	 in
	    WidgetClass,Rec
	    if {HasFeature M ParameterName} then
	       {SetProc self M.ParameterName}
	    end
	    {self CheckRemain(M Remain)}
	 end
	 meth get(...)=M
	    Remain in
	    WidgetClass,{Record.adjoinAt
			 {Record.subtract M ParameterName} RemainingParameters Remain}
	    if {HasFeature M ParameterName} then
	       {CheckFreeParam self M.ParameterName M}
	       M.ParameterName={GetProc self}
	    end
	    {self CheckRemain(M Remain)}
	 end
	 meth !MDestroy
	    {DestroyProc self}
	    WidgetClass,destroy
	 end
      end
   end

   fun{AddReturnSupport WidgetClass ReturnValue}
      Return={NewName}
   in
      {AddParameterSupport
       WidgetClass return
       q(Return)
       proc{$ Obj Value} %% init
	  if {IsDet Value} then
	     if Value==ParameterNotSpecified then skip
	     else
		{RaiseError Obj.widgetId "Parameter return requires a free variable" Value}
	     end
	  else
	     Value=Obj.Return
	  end
       end
       proc{$ Obj V} %% set
	  {RaiseError Obj.widgetId "Parameter return can't be set" set(return:V)}
       end
       proc{$ Obj V} %% get
	  V=Obj.Return
       end
       proc{$ Obj} %% destroy
	  Obj.Return={ReturnValue Obj}
       end
      }
   end
       
   fun{AddActionSupport WidgetClass SetActionP}
      Action={NewName}
   in
      {AddParameterSupport
       WidgetClass action
       q(Action)
       proc{$ Obj Value} %% init
	  Obj.Action={NewCell proc{$} skip end}
	  if Value==ParameterNotSpecified then skip
	  else {Assign Obj.Action Value} {SetActionP Obj Value}
	  end
       end
       proc{$ Obj Value} %% set procedure
	  {Assign Obj.Action Value} {SetActionP Obj Value}
       end
       fun{$ Obj}
	  {Access Obj.Action}
       end
       proc{$ _} skip end}
   end
       
   fun{AddTooltipsSupport WidgetClass SetTooltipsP}
      Tooltips={NewName}
   in
      {AddParameterSupport
       WidgetClass tooltips
       q(Tooltips)
       proc{$ Obj Value} %% init
	  Obj.Tooltips={NewCell
			if Value==ParameterNotSpecified then nil
			else Value end}
	  {SetTooltipsP Obj {Access Obj.Tooltips}}
       end
       proc{$ Obj Value} %% set procedure
	  {Assign Obj.Tooltips Value} {SetTooltipsP Obj Value}
       end
       fun{$ Obj}
	  {Access Obj.Tooltips}
       end
       proc{$ _} skip end}
   end

   fun{AddToplevelSupport WidgetClass}
      class $
	 from WidgetClass
	 feat !ToplevelWidget:true
      end
   end

   fun{AddContainerSupport WidgetClass}
      class $
	 from WidgetClass
	 attr Children:nil
	 meth !MBuildChild(D $)
	    Obj={Build D self}
	 in
	    Children<-Obj|@Children
	    Obj
	 end
	 meth !MDestroyChild(D)
	    if {List.member D @Children} then
	       Children<-{List.subtract @Children D}
	       {D destroy}
	    else
	       {RaiseError self.widgetId "Not a child of this container" D}
	    end
	 end
	 meth !MGetChildren(D)=M
	    {CheckFreeParam self D M}
	    D=@Children
	 end	    
	 meth !MDestroy
	    {ForAll @Children
	     proc{$ O} {O destroy} end}
	 end
      end
   end
   
   fun{AddGroupSupport WidgetClass SetState}
      class $
	 from WidgetClass
	 feat Group GroupManager
	 meth !QInit(...)=M
	    WidgetClass,{Record.subtract M group}
	    if {HasFeature M group} then
	       self.Group=M.group
	    else
	       {RaiseError self.widgetId "Parameter group is missing" M}
	    end
	    self.GroupManager={self get(groupManager:$)}
	    {self.GroupManager addMembers(M.group self)}
	 end
	 meth get(...)=M
	    WidgetClass,{Record.subtract M group}
	    if {HasFeature M group}
	    then
	       {CheckFreeParam self M.group M}
	       self.Group=M.group
	    end
	 end
	 meth setState(B)
	    if B then {self.GroupManager setActive(self.Group self)} end
	    {SetState self B}
	 end
	 meth getState($)
	    {self.GroupManager getActive(self.Group $)}==self
	 end
      end
   end

   WidgetDict={NewDictionary}

   proc{RegisterWidget Widget Class Container}
      if {Dictionary.member WidgetDict Widget} then
	 raise error(widgetAlreadyExist Widget) end
      end
      {Dictionary.put WidgetDict Widget r('class':Class container:Container)}
   end
   
   proc{MissingWidgetError Widget}
      {Exception.raiseError
       mbt(unknownWidget Widget)}
   end
   
   AliasDict={NewDictionary}

   proc{SetAlias N R}
      if {Not {Atom.is N}} then
	 {RaiseError alias "Invalid alias name" N}
      elseif {Dictionary.member WidgetDict N} then
	 {RaiseError alias "Error : alias can't have a widget's name" N}
      elseif {Not {Record.is R}} then
	 {RaiseError alias "Invalid alias definition" R}
      elseif {Not {Dictionary.member WidgetDict {Label R}}} then
	 {RaiseError alias "Error : alias only to regular widget" R}
      else
	 AliasDict.N := R
      end
   end

   fun{GetAlias R}
      if {Dictionary.member AliasDict {Label R}} then
	 Alias=AliasDict.{Label R}
      in
	 {Record.adjoin {Record.adjoin Alias R} {Label Alias}}
      else
	 {RaiseError alias "Unknown alias" R}
	 R
      end
   end

   proc{UnSetAlias N}
      {Dictionary.remove AliasDict N}
   end
   
   UnknownWidget={NewCell MissingWidgetError}

   proc{OnUnknownWidget P}
      {Assign UnknownWidget P}
   end
   
   fun{GetWidget Widget}
      R={Dictionary.condGet WidgetDict Widget unit}
   in
      if R==unit then
	 %% if a dynamic widget creator is defined, call it
	 {{Access UnknownWidget} Widget}
	 R={Dictionary.condGet WidgetDict Widget unit}
      in
	 %% widget is really missing
	 if R==unit then {MissingWidgetError Widget} unit else R.'class' end
      else R.'class' end
   end

   fun{GetWidgetList}
      {List.filter
       {Dictionary.keys WidgetDict}
       fun{$ C} {Not {Name.is C}} end}
   end

   fun{IsContainer Widget}
      R={Dictionary.condGet WidgetDict Widget unit}
   in
      if R==unit then
	 %% if a dynamic widget creator is defined, call it
	 {{Access UnknownWidget} Widget}
	 R={Dictionary.condGet WidgetDict Widget unit}
      in
	 %% widget is really missing
	 if R==unit then {MissingWidgetError Widget} false else R.container end
      else R.container end
   end

   fun{FilterContained Desc}
      {Record.filterInd Desc
       fun{$ I V}
	  {Not ({IsInt I} andthen {IsDet V} andthen {IsRecord V})}
       end}
   end

   proc{PartitionContained Desc A B}
      {Record.partitionInd Desc
       fun{$ I V}
	  ({IsInt I} andthen {IsDet V} andthen {IsRecord V})
       end A B}
   end

%   AutoParams=[handle parent toplevel glue padx pady feature look toplevel actionManager]
   
%   fun{SubtractAutoParams Desc}
%      {Record.filterInd Desc
%       fun{$ P _}
%	  {Not
%	   {List.member P AutoParams}}
%       end}
%   end

%   proc{SplitAutoParams Desc A B}
%      {Record.partitionInd Desc
%       fun{$ P _}
%	  {List.member P AutoParams}
%       end B A}
%   end

   fun{Build Desc1 Parent}
      %% builds widgets corresponding to Description and return the object built
      Alias={Dictionary.condGet AliasDict {Label Desc1} unit}
      Desc=if Alias==unit then Desc1
	   else
	      {Record.adjoin {Record.adjoin Alias Desc1} {Label Alias}}
	   end
      Widget={Label Desc}
      Init={Record.adjoin Desc QInit(parent:Parent glue:{GetGlue Desc})}
      WidgetClass={GetWidget Widget}
      Container={IsContainer Widget}
   in
      if Container then
	 %% create a new object, adding features for subwidgets
	 NewClass={Class.new [WidgetClass] q
		   {Record.map
		    {Record.filterInd Init
		     fun{$ I V}
			{IsInt I} andthen {IsDet V} andthen {IsRecord V} andthen {HasFeature V feature}
		     end}
		    fun{$ V}
		       V.feature
		    end}
		   [locking]}
	 %% propagate look to contained widgets
	 Look={CondSelect Desc look unit}
	 NewInit=if Look==unit then Init
		 else
		    {Record.mapInd Init
		     fun{$ I V}
			if {IsInt I} andthen {IsDet V} andthen {IsRecord V} then
			   {AddLook {Label V} {Record.adjoin r(look:Look) V}}
			else
			   V
			end
		     end}
		 end
      in
	 {New NewClass NewInit}
      else
	 {New WidgetClass Init}
      end
   end

   fun{BuildBaseWidget R}
      {List.foldL
       {Record.toList R}
       fun{$ BaseClass Def}
	  case Def
	  of return(P) then {AddReturnSupport BaseClass P}
	  [] container then {AddContainerSupport BaseClass}
	  [] tooltips(P) then {AddTooltipsSupport BaseClass P}
	  [] action(P) then {AddActionSupport BaseClass P}
	  [] toplevel then {AddToplevelSupport BaseClass}
	  [] group(P) then {AddGroupSupport BaseClass P}
	  else {RaiseError {Label Def} "Invalid parameter support "#Def R} nil end
       end
       Widget}
   end
   
   proc{BuildWidget D Linker}
      %% takes a widget definition (including parameter supports)
      %% and creates the corresponding widget
      WidgetClass
      IsContainer={Record.some D
		   fun{$ V} {Label V}==container end}
   in
      WidgetClass={Linker {Label D} {BuildBaseWidget D}}
      {RegisterWidget {Label D} WidgetClass IsContainer}
   end
   
end
