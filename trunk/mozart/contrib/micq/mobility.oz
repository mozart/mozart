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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%% Large part of this code will be replaced with the SafeSend(Point2Point)-abstraction, when available

functor

export
   stationaryClass:StationaryObject
   newStationary:NewStationaryObject
   
define
   class StationaryObject
      feat this
   end
   
   %% Creates a new instance of Class and encapsulates it in a thread (=stationary)
   fun{NewStationaryObject ?Class ?Init}
      S P={NewPort S}
      Obj={New Class Init}

      proc{RemoteInvokeMeth M}
	 if {Label M}=='GETPORT' then
	    M.1=P
	 else Sync in
	    %% Initialize timeout thread
	    thread
	       {Delay 120000}
	       try Sync=exception(networkFailure(timeout(P))) catch _ then skip end
	    end

	    %% Try to send to remote object
	    try
	       {Send P M#Sync}
	    catch _ then
	       try Sync=exception(newtworkFailure(remoteObjectGone(P))) catch _ then skip end
	    end
	    
	    %% Wait for outcome
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










