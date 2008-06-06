%%%
%%% Authors:
%%%   Raphael Collet (raphael.collet@uclouvain.be)
%%%
%%% Copyright:
%%%   Raphael Collet, 2008
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%% WARNING: these test might not work on all systems.  We make use of
%% the program 'kill' to suspend and resume system processes.  The
%% test should work on all Unix-like systems (Linux, Solaris, MacOSX).
%% It might not work on Win32...

functor

import
   DP
   Remote(manager)
   OS System Finalize
   TestMisc(localHost)
export
   Return

define
   %% create a remote process that exports a port, a cell, and its process id
   proc {StartServer ?S ?E}
      functor F
      import Property OS DP
      export port:P cell:C pid:Pid
      define
	 {Property.put 'close.time' 0}
	 P={NewPort _}
	 C1={NewCell 0} {DP.annotate C1 [stationary]}
	 C=C1
	 Pid={OS.getPID}
      end
   in
      S={New Remote.manager init(host:TestMisc.localHost)}
      {S ping}
      {S apply(F E)}
      {S ping}
   end

   %% put the site with the given process id in the given state
   proc {MakeSite PID State}
      if {OS.system 'kill -'#Signal.State#' '#PID} \= 0 then
	 raise remote('failed to make site in state '#State pid:PID) end
      end
   end
   Signal=map(ok:'CONT' tempFail:'STOP' permFail:'KILL')

   %% check whether the entity reaches the given fault state (within 30 sec)
   proc {CheckEntity E State}
      B = thread {List.member State {DP.getFaultStream E}} end
   in
      if {Record.waitOr B#{Time.alarm 30000}}==1 andthen B then skip else
	 raise 'expected entity state'(E State) end
      end
   end

   %% spawn a new thread
   proc {Spawn P ?T}
      thread {Thread.this T} {P} end
   end
   %% check the thread's state after some delay
   proc {CheckThread T After State}
      {Delay After} {Thread.state T}=State
   end



   %% tempFail with synchronous operation
   proc {TempFailCell} S E T in
      {StartServer S E}
      {MakeSite E.pid tempFail}
      {CheckEntity E.cell tempFail}
      T={Spawn proc {$} {Assign E.cell foo} end}
      {CheckThread T 300 blocked}
      {MakeSite E.pid ok}
      {CheckEntity E.cell ok}
      {CheckThread T 300 terminated}
      {S close}
   end

   %% localFail with synchronous operation
   proc {BreakCell} S E T in
      {StartServer S E}
      {DP.break E.cell}
      {CheckEntity E.cell localFail}
      T={Spawn proc {$} {Assign E.cell foo} end}
      {CheckThread T 300 blocked}
      {S close}
   end

   %% permFail (with Kill) with synchronous operation
   proc {KillCell} S E T in
      {StartServer S E}
      {DP.kill E.cell}
      {CheckEntity E.cell permFail}
      T={Spawn proc {$} {Assign E.cell foo} end}
      {CheckThread T 300 blocked}
      {S close}
   end

   %% permFail with synchronous operation
   proc {PermFailCell} S E T in
      {StartServer S E}
      {MakeSite E.pid permFail}
      {CheckEntity E.cell permFail}
      T={Spawn proc {$} {Assign E.cell foo} end}
      {CheckThread T 300 blocked}
   end

   %% tempFail with asynchronous operation
   proc {TempFailPort} S E T1 T2 in
      {StartServer S E}
      {MakeSite E.pid tempFail}
      {CheckEntity E.port tempFail}
      T1={Spawn proc {$} {Send E.port foo} end}
      {CheckThread T1 300 terminated}
      {MakeSite E.pid ok}
      {CheckEntity E.port ok}
      T2={Spawn proc {$} {Send E.port foo} end}
      {CheckThread T2 300 terminated}
      {S close}
   end

   %% localFail with asynchronous operation
   proc {BreakPort} S E T in
      {StartServer S E}
      {DP.break E.port}
      {CheckEntity E.port localFail}
      T={Spawn proc {$} {Send E.port foo} end}
      {CheckThread T 300 terminated}
      {S close}
   end

   %% permFail (with Kill) with asynchronous operation
   proc {KillPort} S E T in
      {StartServer S E}
      {DP.kill E.port}
      {CheckEntity E.port permFail}
      T={Spawn proc {$} {Send E.port foo} end}
      {CheckThread T 300 terminated}
      {S close}
   end

   %% permFail with asynchronous operation
   proc {PermFailPort} S E T in
      {StartServer S E}
      {MakeSite E.pid permFail}
      {CheckEntity E.port permFail}
      T={Spawn proc {$} {Send E.port foo} end}
      {CheckThread T 300 terminated}
   end

   Return=
   dp([fault_synchronous_tempFail(TempFailCell keys:[fault])
       fault_synchronous_permFail(PermFailCell keys:[fault])
       fault_synchronous_break(BreakCell keys:[fault])
       fault_synchronous_kill(KillCell keys:[fault])
       fault_asynchronous_tempFail(TempFailPort keys:[fault])
       fault_asynchronous_permFail(PermFailPort keys:[fault])
       fault_asynchronous_break(BreakPort keys:[fault])
       fault_asynchronous_kill(KillPort keys:[fault])
      ])
end










