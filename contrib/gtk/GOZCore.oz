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
   System(show)
   GdkNative       at 'GdkNative.so{native}'
   GtkNative       at 'GtkNative.so{native}'
   GtkCanvasNative at 'GtkCanvasNative.so{native}'
   GOZSignal       at 'GOZSignal.so{native}'
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

   local
      ObjectTable = {Dictionary.new}
   in
      %% Obtain Pointer from Oz Object/Pointer
      fun {ObjectToPointer Object}
	 if {IsObject Object}
	 then {Object UnwrapPointer($)}
	 else Object
	 end
      end
      %% Convert Pointer to Oz Object
      fun {PointerToObject Class Pointer}
	 Object = {New Class WrapPointer(Pointer)}
      in
	 {Dictionary.put ObjectTable {ForeignPointer.toInt Pointer} Object}
	 Object
      end
      %% Pointer Tranlation (necessary for GDK Events)
      fun {TranslatePointer Pointer}
	 {Dictionary.condGet ObjectTable {ForeignPointer.toInt Pointer} Pointer}
      end
      %% Release Object Ptr (necessary for GC)
      proc {RemoveObject Pointer}
	 {Dictionary.remove ObjectTable {ForeignPointer.toInt Pointer}}
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
   %% Lowlevel Allocation Stubs (functional setup)
   %%

   fun {AllocInt _}
      {GOZSignal.allocInt}
   end
   fun {AllocDouble _}
      {GOZSignal.allocDouble}
   end
   fun {AllocColor Red Blue Green}
      {GOZSignal.allocColor Red Blue Green}
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
      fun {RGP X}
	 {TranslatePointer X}
      end
      fun {ITB X}
	  X == 1
      end
      
      ExposeFs     = [window#RGP send#ITB area#RGP count#Id]
      MotionFs     = [window#RGP send#ITB time#Id x#Id y#Id pressure#Id xtilt#Id ytilt#Id state#Id
		      is_hint#Id source#Id deveceid#Id x_root#Id y_root#Id]
      ButtonFs     = [window#RGP send#ITB time#Id x#Id y#Id pressure#Id xtilt#Id ytilt#Id state#Id
		      button#Id source#Id deveceid#Id x_root#Id y_root#Id]
      KeyFs        = [window#RGP send#ITB time#Id state#Id keyval#Id length#Id string#Id]
      CrossingFs   = [window#RGP send#ITB subwindow#RGP time#Id x#Id y#Id x_root#Id y_root#Id
		      mode#Id detail#Id focus#ITB state#Id]
      FocusFs      = [window#RGP send#ITB hasFocus#ITB]
      ConfigureFs  = [window#RGP send#ITB x#Id y#Id width#Id height#Id]
      VisibilityFs = [window#RGP send#ITB state#Id]

      fun {MakeEvent Label FeatS Event}
	 GdkEvent = {Record.make Label {Map FeatS fun {$ X#_} X end}}
      in
	 {List.forAllInd FeatS proc {$ I X#F} GdkEvent.X = {F Event.I} end} GdkEvent
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
	 [] 'GDK_ENTER_NOTIFY'      then {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_LEAVE_NOTIFY'      then {MakeEvent GdkLabel CrossingFs GdkEvent}
	 [] 'GDK_FOCUS_CHANGE'      then {MakeEvent GdkLabel FocusFs GdkEvent}
	 [] 'GDK_CONFIGURE'         then {MakeEvent GdkLabel ConfigureFs GdkEvent}
	 [] 'GDK_VISIBILITY_NOTIFY' then {MakeEvent GdkLabel VisibilityFs GdkEvent}
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
   
   local
      CloseObject = {NewName}
   in
      class OzBase from BaseObject
	 attr
	    object         %% Native Object Ptr
	    signals  : nil %% Connected Signals List
	    children : nil %% All Children Objects
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
	 meth close
	    Children = @children
	 in
	    children <- nil
	    OzBase, CloseObject(Children)
	 end
	 meth !CloseObject(Childs)
	    case Childs
	    of Child|Cr then
	       {Child close}
	       OzBase, CloseObject(Cr)
	    [] nil then
	       Object = @object
	    in
	       {ForAll @signals proc {$ SignalId}
				   {Dispatcher unregisterHandler(SignalId)}
				end}
	       {RemoveObject Object}
	       {GOZSignal.freeData Object}
	       object <- unit
	    end
	 end
      end
   end
   
   %%
   %% Core Dispatcher Class (functional setup (used by Oz and Alice))
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
      in
	 class DispatcherObject
	    attr
	       signalId    %% SignalId Counter
	       handlerDict %% SignalId -> Handler
	       signalPort  %% Signal Port
	       threadId    %% Thread Id of "Filler Thread"
	    meth create
	       Stream
	       SignalPort = {Port.new Stream}
	    in
	       @signalId    = 0
	       @handlerDict = {Dictionary.new}
	       @signalPort  = SignalPort
	       %% Tell C side about signal port
	       {GOZSignal.initializeSignalPort SignalPort}
	       %% Fetch Events
	       %% Initial Polling Interval is 50ms
	       thread
		  @threadId = {Thread.this}
		  DispatcherObject, FillStream(50)
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
				 else {Min (PollInterval + 5) 50}
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
	       of event(Id Data)|Tail then
		  _ = {{Dictionary.condGet @handlerDict Id EmptyHandler} Data}
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
      GOZCore = 'GOZCore'(%% Native Pointer Import/Translation
			  pointerToObject      : PointerToObject
			  objectToPointer      : ObjectToPointer
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
			  %% GDK Event Import
			  getGdkEvent          : GetGdkEvent
			  %% GTK Canvas Helper
			  pointsPut            : PointsPut
			  %% Gtk OzBase Class
			  ozBase               : OzBase
			  %% Termination Function
			  exit                 : Exit)
      
      %% Start dispatcher
      Dispatcher = {New DispatcherObject create}
   end
end
