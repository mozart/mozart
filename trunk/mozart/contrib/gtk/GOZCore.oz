%%%
%%% Authors:
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

functor
import
   Pickle(load)
   Module(link)
   GdkNative            at 'GdkNative.so{native}'
   GtkNative            at 'GtkNative.so{native}'
   GtkCanvasNative      at 'GtkCanvasNative.so{native}'
   GdkFieldNative       at 'GdkFieldNative.so{native}'
   GtkFieldNative       at 'GtkFieldNative.so{native}'
   GtkCanvasFieldNative at 'GtkCanvasFieldNative.so{native}'
   GOZSignal            at 'GOZSignal.so{native}'
   GDK                  at 'GDK.ozf'
   GTK                  at 'GTK.ozf'
   GTKCANVAS            at 'GTKCANVAS.ozf'
\ifdef DEBUG
   System(show)
\endif
export
   'GOZCore' : GOZCore
define
   Dispatcher
   %%
   %% Force Evaluation of Modules in appropriate Order
   %%

   {Wait GdkNative}
   {Wait GtkNative}
   {Wait GtkCanvasNative}
   {Wait GdkFieldNative}
   {Wait GtkFieldNative}
   {Wait GtkCanvasFieldNative}
   {Wait GOZSignal}
   
   %%
   %% Native Pointer Import/Export
   %%

   WrapPointer   = {NewName}
   UnwrapPointer = {NewName}

   %% Obtain Pointer from Oz Object or Pointer (Easy)
   fun {ObjectToPointer Object}
      if Object == unit
      then {GOZSignal.null}
      elseif {IsObject Object}
      then {Object UnwrapPointer($)}
      else Object
      end
   end
      
   %% Create Object from incoming Pointer (Difficult)
   local
      Stream ObjectTable
   in
      ObjectTable = {WeakDictionary.new Stream}

      %% Add freshly created objects to table
      %% Used by oz-side-called object constructors
      proc {RegisterObject Pointer Object}
	 %% Increase Reference count by one if applicable
	 %% The Gdk/Gtk ref functions have been made looking
	 %% the same. The default ref is a 'skip'.
	 {Object ref}
	 {WeakDictionary.put ObjectTable {ForeignPointer.toInt Pointer} Object}
      end
      
      %% Loosing the last Oz reference
      %% possibly enables Gdk/Gtk collection
      thread
	 proc {Finalize Stream}
	    case Stream
	    of (_#Object)|Sr then
	       %% Decreases Reference count by one if applicable
	       %% and removes all registered Handlers
\ifdef DEBUG
	       {System.show 'finalizing '#{String.toAtom {Object toString($)}}}
\endif
	       {Object unref}
	       {Finalize Sr}
	    [] nil then skip
	    end
	 end
      in
	 {Finalize Stream}
      end
                  
      %% Convert Pointer to Oz Object
      %% Thread is necessary to prevent suspension
      PointerToObject =
      thread
	 %% Translate Key Information to Oz Class
	 fun {GetOzClass OzClass}
	    case OzClass
	    of 'Gdk'(Key) then
	       NewKey = if Key == '' then misc else Key end
	    in
	       GDK.NewKey
	    [] 'Gtk'(Key) then GTK.Key
	    [] 'GtkCanvas'(Key) then
	       NewKey = case Key
			of ''      then canvas
			[] 'group' then canvasGroup
			[] 'item'  then canvasItem
			end
	    in
	       GTKCANVAS.NewKey
	    end
	 end
	 %% First Upper
	 fun {FU A}
	    S = {VirtualString.toString A}
	 in
	    case S
	    of S|Sr then {Char.toUpper S}|Sr
	    [] nil then nil
	    end
	 end
	 %% Import OzClasses
	 %% Need to drop the parental information
	 %% which is used by Alice Binding
	 ClassList    = {Filter
			 {Map
			  {Pickle.load "x-oz://system/gtk/ClassNames.ozp"}
			   fun {$ 'class'(_ OzClass)}
			      OzClass
			   end}
			  fun {$ OzClass}
			     case OzClass
			     of 'Gdk'(...)    then false
				%% Not implemented under Windows
			     [] 'Gtk'(socket) then false
			     [] _             then true
			     end
			  end}
	 Classes      = {Map ClassList GetOzClass}
	 ClassKeys    = {Map ClassList
			 fun {$ OzClass}
			    RealClass RealObj RealType
			 in
			    RealClass = {GetOzClass OzClass}
			    RealObj   = {New RealClass noop}
			    RealType  = {RealObj getType($)}
			    case RealType
			    of unit then
			       ClassVS = case OzClass
					 of 'Gtk'(Key) then "Gtk"#{FU Key}
					 [] 'GtkCanvas'(Key) then
					    "GtkCanvas"#{FU Key}
					 end
			       ClassS = {VirtualString.toString ClassVS}
			    in
			       {GtkNative.gtkTypeFromName ClassS}
			    [] Type then Type
			    end
			 end}
	 %% GtkTypeKey -> OzClass
	 ClassDict = {FoldL {List.zip ClassKeys Classes fun {$ X Y} X#Y end}
		      fun {$ D X#Y}
			 {Dictionary.put D X Y} D
		      end {Dictionary.new}}
	 %% Determine necessary Oz Class
	 fun {SearchClass Pointer}
	    {Dictionary.get ClassDict {GOZSignal.getObjectType Pointer}}
	 end
	 %% Create New Wrapper and register it to ObjectTable
	 fun {CreateClass Class Pointer}
	    Object = {New Class WrapPointer(Pointer)}
	 in
	    {RegisterObject Pointer Object}
	    Object
	 end
      in
	 fun {$ Hint Pointer}
	    %% Check for already known Pointers
	    Object = {WeakDictionary.condGet ObjectTable
		      {ForeignPointer.toInt Pointer} nil}
	 in
	    case Object
	    of nil then
	       %% New Pointer occured
	       %% none: No information, import as OzBase
	       %% auto: Search for Class Type
	       %% Class: class is known
	       case Hint
	       of none  then {CreateClass OzBase Pointer}
	       [] auto  then {CreateClass {SearchClass Pointer} Pointer}
	       [] Class then {CreateClass Class Pointer}
	       end
	    else Object
	    end
	 end
      end
      %% Pointer Translation (necessary for GLists)
      %% Tries to map incoming pointer to an existing object
      fun {TranslatePointer Pointer}
	 %% TODO: Rationale for this
	 {PointerToObject none Pointer}
      end
      %% GList Import/Export
      fun {ImportList Ls}
	 {Map Ls TranslatePointer}
      end
      fun {ExportList Ls}
	 {Map Ls ObjectToPointer}
      end
   end
   
   %%
   %% Alice Canvas Helper
   %%

   local
      Canvas = {Value.byNeed fun {$}
				{New GTKCANVAS.canvas new}
			     end}

      fun {UnwrapArgument Key#Value}
	 NewKey   = {ByteString.toString Key}
	 NewValue = case Value
		    of 'INT'(V)    then V
		    [] 'DOUBLE'(V) then V
		    [] 'STRING'(V) then V
		    [] 'OBJECT'(V) then V
		    [] 'POINTS'(V) then V
		    end
      in
	 NewKey#NewValue
      end
   in
      fun {CanvasItemNew Group Type Args}
	 {Canvas newItem(Group Type {Map Args UnwrapArgument} $)}
      end
      fun {CanvasItemSet Item Type Arg}
	 case {UnwrapArgument Type#Arg}
	 of Type#Arg then
	    {Item set(Type Arg)}
	    unit
	 end
      end
   end

   %%
   %% Gtk Oz Base Class (used for Oz Class Wrapper)
   %%

   %% It is necessary to separate the finalisation of the
   %% object and its connected handlers since handlers
   %% might be executed as long as the c side object lives.
   %% In contrast, the oz object might have been finalized already.

   class OzBase from BaseObject
      attr
	 object  : unit %% Native Object Ptr
	 signals : unit %% Cell containing Connected Signals List
      meth wrapperNew
	 Signals = {Cell.new nil#nil}
      in
	 signals <- Signals
	 %% The delete event is manually connected.
	 %% This special event automatically calls all other
	 %% connected delete events in order of connection.
	 if {self isGtkObject($)}
	 then
	    {Dispatcher signalConnect(
			   proc {$ Event}
			      {Dispatcher killSignals(Event Signals)}
			   end
			   @object 'delete-event' _)}
	 end
      end
      meth signalConnect(Signal ProcOrMeth $)
	 SigHandler = if {IsProcedure ProcOrMeth}
		      then ProcOrMeth
		      else
			 proc {$ Event}
			    {self ProcOrMeth(Event)}
			 end
		      end
	 CurSignals NewSignals
      in
	 {Cell.exchange @signals CurSignals NewSignals}
	 case CurSignals
	 of DelHs#SigHs then
	    case Signal
	    of 'delete-event' then
	       NewSignals = {Append DelHs [SigHandler]}#SigHs
	       unit
	    [] Signal then
	       SignalId = {Dispatcher
			   signalConnect(SigHandler @object Signal $)}
	    in
	       NewSignals = DelHs#(SignalId|SigHs)
	       SignalId
	    end
	 end
      end
      meth signalDisconnect(SignalId)
	 CurSignals NewSignals
      in
	 {Cell.exchange @signals CurSignals NewSignals}
	 case CurSignals
	 of DelHs|SigHs then
	    NewHs = {Filter SigHs fun {$ Id}
				     SignalId \= Id
				  end}
	 in
	    {Dispatcher signalDisconnect(@object SignalId)}
	    NewSignals = DelHs#NewHs
	 end
      end
      meth signalBlock(SignalId)
	 {GOZSignal.signalBlock @object SignalId}
      end
      meth signalUnblock(SignalId)
	 {GOZSignal.signalUnblock @object SignalId}
      end
      meth signalEmit(Signal)
	 {GOZSignal.signalEmit @object Signal}
      end
      meth !WrapPointer(Ptr)
	 object <- Ptr
	 {self wrapperNew}
      end
      meth !UnwrapPointer($)
	 @object
      end
      meth addToObjectTable
	 {RegisterObject @object self}
      end
      meth ref
	 %% Presume unmanaged Object
	 skip
      end
      meth unref
	 %% Presume unmanaged Object
	 skip
      end
      meth getType($)
	 unit
      end
      meth isGtkObject($)
	 false
      end
   end

   %%
   %% Oz/Alice GdkEvent Creation
   %%
   local
      fun {Id X}
	 X
      end
      fun {RGW X}
	 {PointerToObject GDK.window X}
      end
      fun {RGT X}
	 {PointerToObject GDK.rectangle X}
      end
      fun {ITB X}
	 X == 1
      end
      
      ExposeFs     = [window#RGW send#ITB area#RGT count#Id]
      MotionFs     = [window#RGW send#ITB time#Id x#Id y#Id
		      pressure#Id xtilt#Id ytilt#Id state#Id
		      is_hint#Id source#Id deveceid#Id x_root#Id y_root#Id]
      ButtonFs     = [window#RGW send#ITB time#Id x#Id y#Id
		      pressure#Id xtilt#Id ytilt#Id state#Id
		      button#Id source#Id deveceid#Id x_root#Id y_root#Id]
      KeyFs        = [window#RGW send#ITB time#Id state#Id
		      keyval#Id length#Id string#Id]
      CrossingFs   = [window#RGW send#ITB subwindow#RGW time#Id
		      x#Id y#Id x_root#Id y_root#Id
		      mode#Id detail#Id focus#ITB state#Id]
      FocusFs      = [window#RGW send#ITB hasFocus#ITB]
      ConfigureFs  = [window#RGW send#ITB x#Id y#Id width#Id height#Id]
      VisibilityFs = [window#RGW send#ITB state#Id]
      
      fun {First X#_} X end
      
      fun {MakeEvent Label FeatS Event}
	 GdkEvent = {Record.make Label {Map FeatS First}}
      in
	 {List.forAllInd FeatS
	  proc {$ I X#F} GdkEvent.X = {F Event.I} end} GdkEvent
      end
   in
      fun {ComputeGdkEvent GdkEvent}
	 GdkLabel = {Label GdkEvent}
      in
	 case GdkLabel
	 of 'GDK_EXPOSE'         then {MakeEvent GdkLabel ExposeFs GdkEvent}
	 [] 'GDK_MOTION_NOTIFY'  then {MakeEvent GdkLabel MotionFs GdkEvent}
	 [] 'GDK_BUTTON_PRESS'   then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_2BUTTON_PRESS'  then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_3BUTTON_PRESS'  then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_BUTTON_RELEASE' then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_KEY_PRESS'      then {MakeEvent GdkLabel KeyFs GdkEvent}
	 [] 'GDK_KEY_RELEASE'    then {MakeEvent GdkLabel KeyFs GdkEvent}
	 [] 'GDK_ENTER_NOTIFY'   then {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_LEAVE_NOTIFY'   then {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_FOCUS_CHANGE'   then {MakeEvent GdkLabel FocusFs GdkEvent}
	 [] 'GDK_CONFIGURE'      then {MakeEvent GdkLabel ConfigureFs GdkEvent}
	 [] 'GDK_VISIBILITY_NOTIFY' then
	    {MakeEvent GdkLabel VisibilityFs GdkEvent}
	 [] 'GDK_NO_EXPOSE'         then {MakeEvent GdkLabel ExposeFs GdkEvent}
	 [] Name                    then Name
	 end
      end
   end
   
   %%
   %% Argument Conversion
   %%

   local
      fun {ConvertArgument Arg}
	 case Arg
	 of int(Val)      then Val
	 [] double(Val)   then Val
	 [] string(Val)   then Val
	 [] pointer(Val)  then Val
	 [] object(Val)   then {PointerToObject auto Val}
	    %% GTK Object which are not GTK Objects also need special care
	 [] accel(Val)    then {PointerToObject GTK.accelGroup Val}
	 [] style(Val)    then {PointerToObject GTK.style Val}
	    %% GDK Events need special care
	 [] event(Val)    then {ComputeGdkEvent Val}
	 [] color(Val)    then {PointerToObject GDK.color Val}
	 [] context(Val)  then {PointerToObject GDK.colorContext Val}
	 [] map(Val)      then {PointerToObject GDK.colormap Val}
	 [] cursor(Val)   then {PointerToObject GDK.cursor Val}
	 [] drag(Val)     then {PointerToObject GDK.dragContext Val}
	 [] drawable(Val) then {PointerToObject GDK.drawable Val}
	 [] font(Val)     then {PointerToObject GDK.font Val}
	 [] gc(Val)       then {PointerToObject GDK.gc Val}
	 [] image(Val)    then {PointerToObject GDK.image Val}
	 [] visual(Val)   then {PointerToObject GDK.visual Val}
	 [] window(Val)   then {PointerToObject GDK.window Val}
	 end
      end
   in
      proc {ConvertOzArgs Args ?R}
	 case Args
	 of Arg|Ar then
	    NewR
	 in
	    R = {ConvertArgument Arg}|NewR
	    {ConvertOzArgs Ar NewR}
	 [] nil then R = nil
	 end
      end
      fun {GetArg Arg}
	 {ConvertArgument {GOZSignal.getArg Arg}}
      end
   end

   %%
   %% Alice Argument Conversion
   %%

   local
      fun {ConvertArgument Arg}
	 case Arg
	 of int(Val)      then 'INT'(Val)
	 [] double(Val)   then 'DOUBLE'(Val)
	 [] string(Val)   then 'STRING'({ByteString.make Val})
	 [] pointer(Val)  then 'OBJECT'({PointerToObject none Val})
	 [] object(Val)   then 'OBJECT'({PointerToObject auto Val})
	    %% GTK Object which are not GTK Objects also need special care
	 [] accel(Val)    then 'OBJECT'({PointerToObject GTK.accelGroup Val})
	 [] style(Val)    then 'OBJECT'({PointerToObject GTK.style Val})
	    %% GDK Events need special care
	 [] event(Val)    then 'EVENT'({ComputeGdkEvent Val})
	 [] color(Val)    then 'OBJECT'({PointerToObject GDK.color Val})
	 [] context(Val)  then 'OBJECT'({PointerToObject GDK.colorContext Val})
	 [] map(Val)      then 'OBJECT'({PointerToObject GDK.colormap Val})
	 [] cursor(Val)   then 'OBJECT'({PointerToObject GDK.cursor Val})
	 [] drag(Val)     then 'OBJECT'({PointerToObject GDK.dragContext Val})
	 [] drawable(Val) then 'OBJECT'({PointerToObject GDK.drawable Val})
	 [] font(Val)     then 'OBJECT'({PointerToObject GDK.font Val})
	 [] gc(Val)       then 'OBJECT'({PointerToObject GDK.gc Val})
	 [] image(Val)    then 'OBJECT'({PointerToObject GDK.image Val})
	 [] visual(Val)   then 'OBJECT'({PointerToObject GDK.visual Val})
	 [] window(Val)   then 'OBJECT'({PointerToObject GDK.window Val})
	 end
      end
   in
      proc {ConvertAliceArgs Args ?R}
	 case Args
	 of Arg|Ar then
	    NewR
	 in
	    R = {ConvertArgument Arg}|NewR
	    {ConvertAliceArgs Ar NewR}
	 [] nil then R = nil
	 end
      end
      fun {GetAliceArg Arg}
	 {ConvertArgument {GOZSignal.getArg Arg}}
      end
   end
   
   %%
   %% Core Dispatcher Class
   %%

   local
      Start = {NewName}
      local
	 NewSignalId = {NewName}
	 FillStream  = {NewName}
	 Dispatch    = {NewName}

	 %% Dummy Handler to circumvent problems with event caching
	 proc {EmptyHandler _}
\ifdef DEBUG
	    {System.show 'Empty Handler Called'}
\endif
	    skip
	 end
	 %% Maxium Polling Delay
	 PollDelay = 50 
      in
	 class DispatcherObject
	    attr
	       stream      %% Event Stream
	       signalId    %% SignalId Counter
	       handlerDict %% SignalId -> Handler
	       signalPort  %% Signal Port
	       fillerId    %% Filler Thread Id
	       dispatchId  %% Dispatch Thread Id
	       conv        %% Argument Converter
	    meth create
	       Stream SignalPort
	    in
	       SignalPort   = {Port.new Stream}
	       @stream      = Stream
	       @signalId    = 0
	       @handlerDict = {Dictionary.new}
	       @signalPort  = SignalPort
	       %% Tell C side about signal port
	       {GOZSignal.initializeSignalPort SignalPort}
	       %% Fetch Events (Filler Thread)
	       thread
		  @fillerId = {Thread.this}
		  DispatcherObject, FillStream(PollDelay)
	       end
	    end
	    %% FillStream runs on its own. It does not access any
	    %% stateful data of the DispatcherObject.
	    %% Just placed here for convenience.
	    meth !FillStream(PollInterval)
	       NeedDispatch    = {GOZSignal.handlePendingEvents}
	       NewPollInterval = if NeedDispatch
				 then
				    %% Enable Event Processing
				    {Dispatcher Dispatch}
				    10 %% Rapid Event testing for reactivity
				 else {Min (PollInterval + 5) PollDelay}
				 end
	    in
	       {Time.delay NewPollInterval}
	       DispatcherObject, FillStream(NewPollInterval)
	    end
	    meth !NewSignalId($)
	       signalId <- (@signalId + 1)
	    end
	    meth signalConnect(Handler Object Signal $)
	       SignalId = DispatcherObject, NewSignalId($)
	    in
	       {Dictionary.put @handlerDict SignalId Handler}
	       {GOZSignal.signalConnect Object Signal SignalId}
	       SignalId
	    end
	    meth signalDisconnect(Object SignalId)
	       {GOZSignal.signalDisconnect Object SignalId}
	       {Dictionary.remove @handlerDict SignalId}
	    end
	    meth killSignals(Event Signals)
	       HandlerDict = @handlerDict
	    in
	       case {Cell.access Signals}
	       of DelHs#SigHs then
		  %% Execute any delete Event Handler connected
		  {ForAll DelHs
		   proc {$ Handler}
		      {Handler Event}
		   end}
		  %% Disconnect any Handlers
		  {ForAll SigHs
		   proc {$ SignalId}
		      {Dictionary.remove HandlerDict SignalId}
		   end}
	       end
	    end
	    meth !Start
	       @dispatchId = {Thread.this}
	    end
	    meth !Dispatch
	       Stream = @stream
	    in
	       if {IsFuture Stream}
	       then skip
	       elsecase Stream
	       of (Id|Data)|Tail then
		  case {Dictionary.condGet @handlerDict Id EmptyHandler}
		  of Handler then {Handler {@conv Data}}
		  end
		  stream <- Tail
		  {Dispatcher Dispatch}
	       [] nil then
		  DispatcherObject, exit
	       end
	    end
	    meth setMode(Mode)
	       conv <- case Mode
		       of oz    then ConvertOzArgs
		       [] alice then ConvertAliceArgs
		       end
	    end
	    meth exit
	       {GtkNative.exit}               %% Terminate C side system
	       {Thread.terminate @fillerId}   %% Terminate Event Fetching
	       {Thread.terminate @dispatchId} %% Terminate Dispatcher
	    end
	 end
      end
   in
      proc {Exit}
	 {Dispatcher exit}
      end

      proc {SetMode Mode}
	 {Dispatcher setMode(Mode)}
      end
      
      %% Create Interface
      GOZCore = 'GOZCore'(%% Native Pointer Import/Export
			  pointerToObject      : PointerToObject
			  objectToPointer      : ObjectToPointer
			  %% GList Import/Export
			  importList           : ImportList
			  exportList           : ExportList
			  %% Lowlevel Allocation/Access
			  allocInt             : GOZSignal.allocInt
			  allocDouble          : GOZSignal.allocDouble
			  allocColor           : GOZSignal.allocColor
			  getInt               : GOZSignal.getInt
			  getDouble            : GOZSignal.getDouble
			  null                 : GOZSignal.null
			  freeData             : GOZSignal.freeData
			  %% Gtk Arg Handling
			  makeEmptyArg         : GOZSignal.makeEmptyArg
			  makeArg              : GOZSignal.makeArg
			  getArg               : GetArg
			  getAliceArg          : GetAliceArg
			  %% String Handling
			  allocStr             : GOZSignal.allocStr
			  getStr               : GOZSignal.getStr
			  %% String Arr Handling
			  allocStrArr          : GOZSignal.allocStrArr
			  makeStrArr           : GOZSignal.makeStrArr
			  getStrArr            : GOZSignal.getStrArr
			  %% Color Array Handling
			  makeColorArr         : GOZSignal.makeColorArr
			  getColorList         : GOZSignal.getColorList
			  %% Alice Canvas Helper
			  canvasItemNew        : CanvasItemNew
			  canvasItemSet        : CanvasItemSet
			  %% OzBase Class
			  ozBase               : OzBase
			  %% Set Dispatcher Mode
			  setMode              : SetMode
			  %% Termination Function
			  exit                 : Exit)
      
      %% Create & Start dispatcher
      fun {NewServer O}
	 S P
      in
	 P = {NewPort S}
	 thread
	    {ForAll S O}
	 end
	 proc {$ M}
	    {Port.send P M}
	 end
      end
      Dispatcher = {NewServer {New DispatcherObject create}}
      {Dispatcher setMode(oz)}
      {Dispatcher Start}
   end
end
