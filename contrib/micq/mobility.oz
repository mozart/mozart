%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
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

%%% Large part of this code will be replaced with the SafeSend(Point2Point)-abstraction, when available

functor
import
   Fault
%   System
export
   stationaryClass:StationaryObject
   newStationary:NewStationaryObject
define
   {Wait Fault}
   class StationaryObject
      feat this
   end

%    DBGS DBGP={NewPort DBGS}
%     thread
%        {ForAll DBGS proc{$ X}
%  		      {System.show X}
%  		   end}
%     end
   
   %% Creates a new instance of Class and encapsulates it in a thread (=stationary)
   fun{NewStationaryObject ?Class ?Init}
      S P={NewPort S}
      Obj={New Class Init}

      proc{RemoteInvokeMeth M}
	 if {Label M}=='GETPORT' then
	    M.1=P
	 else
	    Sync
	    proc{Watcher _ _}
	       try Sync=exception(networkFailure(remoteObjectGone(P))) catch _ then skip end
	    end
	 in

 	    
	    %% Failure handling
%	    {Send DBGP tryInstallWatcher(Watcher)}
	    {Fault.installWatcher P [permFail] Watcher _}

	    %% Try to send to remote object
	    try
%	       {Send DBGP send}
	       {Send P M#Sync}
	    catch _ then
	       skip
	    end

	    %% Wait for outcome
%	    {Send DBGP wait}
	    {Wait Sync}
	    
	    %% Unistall watcher
%	    {Send DBGP deInstallWatcher}
	    {Fault.deInstallWatcher P Watcher _}

	    %% Ok or not?
	    if {Label Sync}==exception then raise Sync.1 end end
	 end
      end
   
      proc{ServeObj}
	 {ForAll S proc{$ X} thread 
				try
				   {Obj X.1} %% Invoke method on object
				   X.2=unit  %% Signal back that everything went ok
				catch E then try X.2=exception(E) catch _ then skip end end
			     end
		   end}
      end
   in
      thread {ServeObj} end
      Obj.this=RemoteInvokeMeth
   end
end










