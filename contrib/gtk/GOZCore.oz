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
   System(show)
   Module(link)
   GdkNative       at 'GdkNative.so{native}'
   GtkNative       at 'GtkNative.so{native}'
   GtkCanvasNative at 'GtkCanvasNative.so{native}'
   GOZSignal       at 'GOZSignal.so{native}'
   GDK             at 'GDK.ozf'
   GTK             at 'GTK.ozf'
   GTKCANVAS       at 'GTKCANVAS.ozf'
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
   {Wait GOZSignal}
   
   %%
   %% Native Pointer Import/Export
   %%

   WrapPointer   = {NewName}
   UnwrapPointer = {NewName}
   CloseObject   = {NewName}
   
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
	 try
	    {Object ref}
	 catch _ then
	    {Object ref(_)}
	 end
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
	       {Object CloseObject}
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
	 ClassList    = {Filter
			 {Pickle.load "x-oz://system/gtk/ClassNames.ozp"}
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
	    Key    = {ForeignPointer.toInt Pointer}
	 in
	    %% Increase Reference count by one if applicable
	    try
	       {Object ref}
	    catch _ then
	       {Object ref(_)}
	    end
	    {WeakDictionary.put ObjectTable Key Object}
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
	       case Hint
	       of none  then Pointer
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
   %% Signal Handling Stubs (functional setup; used for Alice)
   %%
   
   fun {SignalConnect Object ObjSignal Handler}
      Id = {Dispatcher registerHandler(Handler $)}
   in
      {GOZSignal.signalConnect Object ObjSignal Id} Id
   end
   fun {SignalDisconnect Object SignalId}
      {Dispatcher unregisterHandler(SignalId)}
      {GOZSignal.signalConnect Object SignalId}
      unit
   end
   fun {SignalHandlerBlock Object SignalId}
      {GOZSignal.signalBlock Object SignalId}
      unit
   end
   fun {SignalHandlerUnblock Object SignalId}
      {GOZSignal.signalUnblock Object SignalId}
      unit
   end
   fun {SignalEmit Object Name}
      {GOZSignal.signalEmit Object Name}
      unit
   end

   %%
   %% Lowlevel Allocation Stubs
   %%

   fun {AllocInt InVal}
      {GOZSignal.allocInt InVal}
   end
   fun {AllocDouble InVal}
      {GOZSignal.allocDouble InVal}
   end
   fun {AllocColor Red Green Blue}
      {GOZSignal.allocColor Red Green Blue}
   end

   fun {GetInt V}
      {GOZSignal.getInt V}
   end
   fun {GetDouble V}
      {GOZSignal.getDouble V}
   end
   fun {Null}
      {GOZSignal.null}
   end
   fun {FreeData V}
      {GOZSignal.freeData V}
      unit
   end

   %%
   %% Gdk Event Import (Conversion)
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
      fun {GetGdkEvent GdkEvent}
	 GdkLabel = {Label GdkEvent}
      in
	 case GdkLabel
	 of 'GDK_EXPOSE'            then {MakeEvent GdkLabel ExposeFs GdkEvent}
	 [] 'GDK_MOTION_NOTIFY'     then {MakeEvent GdkLabel MotionFs GdkEvent}
	 [] 'GDK_BUTTON_PRESS'      then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_2BUTTON_PRESS'     then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_3BUTTON_PRESS'     then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_BUTTON_RELEASE'    then {MakeEvent GdkLabel ButtonFs GdkEvent}
	 [] 'GDK_KEY_PRESS'         then {MakeEvent GdkLabel KeyFs GdkEvent}
	 [] 'GDK_KEY_RELEASE'       then {MakeEvent GdkLabel KeyFs GdkEvent}
	 [] 'GDK_ENTER_NOTIFY'      then
	    {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_LEAVE_NOTIFY'      then
	    {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_FOCUS_CHANGE'      then {MakeEvent GdkLabel FocusFs GdkEvent}
	 [] 'GDK_CONFIGURE'         then
	    {MakeEvent GdkLabel ConfigureFs GdkEvent}
	 [] 'GDK_VISIBILITY_NOTIFY' then
	    {MakeEvent GdkLabel VisibilityFs GdkEvent}
	 [] 'GDK_NO_EXPOSE'         then {MakeEvent GdkLabel ExposeFs GdkEvent}
	 [] Name                    then Name
	 end
      end
   end
   
   %%
   %% Gtk Canvas Helper
   %%

   fun {PointsPut Points I Val}
      {GOZSignal.pointsPut Points I Val}
      unit
   end
   
   %%
   %% Gtk Oz Base Class (used for Oz Class Wrapper)
   %%
   
   class OzBase from BaseObject
      attr
	 object        %% Native Object Ptr
	 signals : nil %% Connected Signals List
      meth new
	 @object = unit
      end
      meth signalConnect(Signal ProcOrMeth $)
	 SigHandler = if {IsProcedure ProcOrMeth}
		      then
			 fun {$ Event}
			    {ProcOrMeth Event}
			    unit
			 end
		      else
			 fun {$ Event}
			    {self ProcOrMeth(Event)}
			    unit
			 end
		      end
	 SignalId   = {Dispatcher registerHandler(SigHandler $)}
      in
	 signals <- SignalId|@signals
	 {GOZSignal.signalConnect @object Signal SignalId}
	 SignalId
      end
      meth signalDisconnect(SignalId)
	 signals <- {Filter @signals fun {$ Id}
					SignalId \= Id
				     end}
	 {GOZSignal.signalDisconnect @object SignalId}
	 {Dispatcher unregisterHandler(SignalId)}
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
	 @object = Ptr
      end
      meth !UnwrapPointer($)
	 @object
      end
      meth addToObjectTable
	 {RegisterObject @object self}
      end
      meth !CloseObject
	 %% This method is called from the Finalisation
	 %% Handler to remove all registered Handlers
	 %% This is because a handler might be connected to several widgets
	 %% We only remove Handler from dictionary since the Gtk finalizer
	 %% disconnects all signals automaticly.
	 {ForAll @signals proc {$ SignalId}
			     {Dispatcher unregisterHandler(SignalId)}
			  end}
	 signals <- nil
	 {self unref}
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
   end
   
   %%
   %% Argument Conversion
   %%

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
      [] event(Val)    then {GetGdkEvent Val}
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

   proc {ConvertArgs Args ?R}
      case Args
      of Arg|Ar then
	 NewR
      in
	 R = {ConvertArgument Arg}|NewR
	 {ConvertArgs Ar NewR}
      [] nil then R = nil
      end
   end
   
   fun {GetArg Arg}
      {ConvertArgument {GOZSignal.getArg Arg}}
   end
   
   %%
   %% Core Dispatcher Class
   %%

   local
      local
	 NewSignalId = {NewName}
	 FillStream  = {NewName}
	 Dispatch    = {NewName}

	 %% Dummy Handler to circumvent problems with event caching
	 fun {EmptyHandler _}
	    unit
	 end
	 %% Maxium Polling Delay
	 PollDelay = 50 
      in
	 class DispatcherObject
	    attr
	       signalId    %% SignalId Counter
	       handlerDict %% SignalId -> Handler
	       signalPort  %% Signal Port
	       threadId    %% Thread Id of "Filler Thread"
	    meth create
	       Stream SignalPort
	    in
	       SignalPort   = {Port.new Stream}
	       @signalId    = 0
	       @handlerDict = {Dictionary.new}
	       @signalPort  = SignalPort
	       %% Tell C side about signal port
	       {GOZSignal.initializeSignalPort SignalPort}
	       %% Fetch Events
	       thread
		  @threadId = {Thread.this}
		  DispatcherObject, FillStream(PollDelay)
	       end
	       %% Call Event Handlers
	       thread
		  try DispatcherObject, Dispatch(Stream)
		  catch Ex then
		     {System.show Ex}
		     DispatcherObject, exit
		  end
	       end
	    end
	    meth !FillStream(PollInterval)
	       NewPollInterval = if {GOZSignal.handlePendingEvents}
				 then 10 %% Rapid Event testing for reactivity
				 else {Min (PollInterval + 5) PollDelay}
				 end
	    in
	       {Time.delay NewPollInterval}
	       DispatcherObject, FillStream(NewPollInterval)
	    end
	    meth !NewSignalId($)
	       signalId <- (@signalId + 1)
	    end
	    meth registerHandler(Handler $)
	       SignalId = DispatcherObject, NewSignalId($)
	    in
	       {Dictionary.put @handlerDict SignalId Handler}
	       SignalId
	    end
	    meth unregisterHandler(SignalId)
	       {Dictionary.remove @handlerDict SignalId}
	    end
	    meth !Dispatch(Stream)
	       case Stream
	       of Event|Tail then
		  Id   = Event.1
		  %% Todo: Check for optimisations
		  Data = {Record.toList Event}.2
	       in
		  case {Dictionary.condGet @handlerDict Id EmptyHandler}
		  of Handler then {Handler {ConvertArgs Data} _}
		  end
		  DispatcherObject, Dispatch(Tail)
	       [] _ then skip
	       end
	    end
	    meth exit
	       {Thread.terminate @threadId}     %% Terminate Event Fetching
	       {Thread.terminate {Thread.this}} %% Terminate Dispatch Thread
	    end
	 end
      end
   in
      fun {Exit _}
	 {GtkNative.gtkExit}
	 {Dispatcher exit}
	 unit
      end
      
      %% Create Interface
      GOZCore = 'GOZCore'(%% Native Pointer Import/Export
			  pointerToObject      : PointerToObject
			  objectToPointer      : ObjectToPointer
			  %% GList Import/Export
			  importList           : ImportList
			  exportList           : ExportList
			  %% Signal Handling
			  signalConnect        : SignalConnect
			  signalDisconnect     : SignalDisconnect
			  signalHandlerBlock   : SignalHandlerBlock
			  signalHandlerUnblock : SignalHandlerUnblock
			  signalEmit           : SignalEmit
			  %% Lowlevel Allocation/Access
			  allocInt             : AllocInt
			  allocDouble          : AllocDouble
			  allocColor           : AllocColor
			  getInt               : GetInt
			  getDouble            : GetDouble
			  null                 : Null
			  freeData             : FreeData
			  %% Gtk Arg Handling
			  makeEmptyArg         : GOZSignal.makeEmptyArg
			  makeArg              : GOZSignal.makeArg
			  getArg               : GetArg
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
			  %% GDK Event Import
			  getGdkEvent          : GetGdkEvent
			  %% GTK Canvas Helper
			  pointsPut            : PointsPut
			  %% OzBase Class
			  ozBase               : OzBase
			  %% Termination Function
			  exit                 : Exit)
      
      %% Start dispatcher
      Dispatcher = {New DispatcherObject create}
   end
end
