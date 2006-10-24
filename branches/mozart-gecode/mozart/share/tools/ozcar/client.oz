%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
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
   BootName(newUnique: NewUniqueName) at 'x-oz://boot/Name'
   CompilerSupport(isBuiltin: IsBuiltin) at 'x-oz://boot/CompilerSupport'
   OS(uName getPID)
   Connection(take)
   System(printName)
   Error(printException)
   Primitives at 'OzcarPrimitives.ozf'
export
   Start
define
   CommandStream
   CommandPort = {NewPort ?CommandStream}

   SiteName = {OS.uName}.nodename#':'#{OS.getPID}

   %%
   %% Transportation of first-class threads
   %%

   local
      %--** garbage collection: when can entries be removed?
      ReflectDict   = {NewDictionary}
      UnreflectDict = {NewDictionary}

      fun {NewThread T I RemoteI}
	 Q = {Primitives.getParentId T}
	 RemoteQ = case {Dictionary.condGet ReflectDict Q unit} of unit then
		      RemoteQ = {NewName}
		   in
		      ReflectDict.Q := RemoteQ
		      RemoteQ
		   [] 'thread'(id: RemoteQ ...) then RemoteQ
		   elseof RemoteQ then RemoteQ
		   end
	 ReflectedThread =
	 'thread'(id:     RemoteI
		  parent: RemoteQ
		  name:   {Primitives.getThreadName T}#'@'#SiteName
		  port:   CommandPort)
      in
	 ReflectDict.I := ReflectedThread
	 UnreflectDict.RemoteI := T
	 ReflectedThread
      end
   in
      fun {ReflectThread T} I in
	 I = {Primitives.getThreadId T}
	 case {Dictionary.condGet ReflectDict I unit} of unit then
	    {NewThread T I {NewName}}
	 [] ReflectedThread='thread'(...) then ReflectedThread
	 elseof RemoteI then
	    {NewThread T I RemoteI}
	 end
      end

      fun {UnreflectThread 'thread'(id: I ...)}
	 UnreflectDict.I
      end
   end

   %%
   %% Forwarding events to the remote debugger
   %%

   Approximation = {NewUniqueName 'Approximation'}

   fun {ReflectValue X}   %--** maximum depth, width, sharing?
      case {Value.status X}
      of det(int)        then X
      [] det(float)      then X
      [] det(tuple)      then {Record.map X ReflectValue}
      [] det(record)     then {Record.map X ReflectValue}
      [] det(atom)       then X
      [] det(name)       then X
      [] det(procedure)  then if {IsBuiltin X} then X
			      else
				 Approximation(procedure {System.printName X})
			      end
      [] det(cell)       then Approximation(cell {ReflectValue {Access X}})
      [] det(dictionary) then Approximation(dictionary
					    {Record.map
					     {Dictionary.toRecord unit X}
					     ReflectValue})
      [] det(Y)          then Approximation(Y)
      [] kinded(int)     then Approximation(fdvar)
      [] kinded(fset)    then Approximation(fsvar)
      [] kinded(record)  then Approximation(recordc)
      [] future          then Approximation(future)
      [] failed          then Approximation(failed)
      [] free            then Approximation(free)
      else                    Approximation
      end
   end

   fun {ReflectExn E}
      if {IsDet E} andthen {IsRecord E} andthen {HasFeature E debug}
	 andthen {IsDet E.debug} andthen {IsRecord E.debug}
	 andthen {HasFeature E.debug stack}
      then
	 {AdjoinAt {ReflectValue {Record.subtract E debug}} debug
	  {AdjoinAt {ReflectValue {Record.subtract E.debug stack}} stack
	   {Map E.debug.stack ReflectMessage}}}
      else
	 {ReflectValue E}
      end
   end

   fun {ReflectMessage M}
      Args =
      [if {HasFeature M thr}     then thr#{ReflectThread M.thr}  else unit end
       if {HasFeature M data}    then data#{ReflectValue M.data} else unit end
       if {HasFeature M kind}    then kind#M.kind                else unit end
       if {HasFeature M file}    then file#M.file                else unit end
       if {HasFeature M line}    then line#M.line                else unit end
       if {HasFeature M column}  then column#M.column            else unit end
       if {HasFeature M args}    then args#{ReflectValue M.args} else unit end
       if {HasFeature M frameID} then frameID#M.frameID          else unit end
       if {HasFeature M vars}    then vars#{ReflectValue M.vars} else unit end
       if {HasFeature M origin}  then origin#reflect             else unit end
       if {HasFeature M exc}     then exc#{ReflectExn M.exc}     else unit end]
   in
      {List.toRecord {Label M} {Filter Args fun {$ X} X \= unit end}}
   end

   %%
   %% Executing remote queries as local operations
   %%

   proc {Start Ticket}
      case {{Connection.take Ticket} SiteName CommandPort}
      of EventPort#Mode then
	 thread
	    for Event in {Primitives.getEventStream} do
	       {Send EventPort {ReflectMessage Event}}
	    end
	 end
	 {Wait Mode}
	 {Primitives.setMode Mode}
	 thread
	    for Command in CommandStream do
	       try
		  case Command of setMode(OnOff) then
		     {Primitives.setMode OnOff}
		  [] breakpointAt(File Line Enabled ?Succeeded) then
		     Succeeded = {Primitives.breakpointAt File Line Enabled}
		  [] getStack(Thr Count Verbose ?Frames) then
		     Frames = {Map {Primitives.getStack
				    {UnreflectThread Thr} Count Verbose}
			       ReflectMessage}
		  [] getEnvironment(Thr FrameID ?Vars) then
		     Vars = {ReflectValue {Primitives.getEnvironment
					   {UnreflectThread Thr} FrameID}}
		  [] threadState(Thr ?State) then
		     State = {Primitives.threadState {UnreflectThread Thr}}
		  [] suspend(Thr) then
		     {Primitives.suspend {UnreflectThread Thr}}
		  [] resume(Thr) then
		     {Primitives.resume {UnreflectThread Thr}}
		  [] unleash(Thr FrameID DoStep) then
		     {Primitives.unleash {UnreflectThread Thr} FrameID DoStep}
		  [] detach(Thr) then
		     {Primitives.detach {UnreflectThread Thr}}
		  [] terminate(Thr) then
		     {Primitives.terminate {UnreflectThread Thr}}
		  end
	       catch E then
		  {Error.printException E}
	       end
	    end
	 end
      end
   end
end
