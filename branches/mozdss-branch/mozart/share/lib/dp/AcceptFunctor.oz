%%%
%%% Authors:
%%%   Erik Klintskog (erik@sics.se)
%%%   Anna Neiderud (annan@sics.se)
%%%
%%% Copyright:
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

\define DBG
functor
export
   Accept
import
   OS
\ifdef DBG
   System
\endif
   Glue at 'x-oz://boot/Glue'
define
\ifdef DBG
   ShowInfo  = System.showInfo
   Show = System.show
\else
   Show = proc{$ _} skip end
   ShowInfo = proc{$ _} skip end
\endif

   PID % Used for dbg to avoid going to the OS all the time
   
   class ResourceHandler
      prop
	 locking
      attr
	 r
	 q
      meth init(I)
	 r<-I
	 q<-nil
      end
      meth getResource
	 skip/*
	 lock W in
	    if @r>0 then
	       r<-@r-1
	       W=unit
	    else
	       q<-{Append @q [W]}
	    end
	    {Wait W}
	 end*/
      end
      meth returnResource
	 skip/*
	 lock
	    if @q==nil then
	       r<-@r+1
	    else
	       Q1|QR=@q
	    in
	       @r=0 % Check
	       q<-QR
	       Q1=unit
	    end
	 end*/
      end
   end

   MaxRead = 1000

   FDHandler = {New ResourceHandler init(5)}
   fun{BindSocket FD PortNum}
      try 
	 {ShowInfo 'BindSocket '#FD#' '#PortNum}
	 {OS.bind FD PortNum}
	 PortNum
      catch _ then 
	 {BindSocket FD PortNum + 1}
      end
   end
   
   proc{AcceptSelect FD}
      NewFD in
      try
	 {ShowInfo 'AcceptedSelect on '#FD#' '#{OS.getPID}}
	 {FDHandler getResource}
	 {ShowInfo 'Got resource'#' '#{OS.getPID}}
	 {OS.acceptSelect FD}
	 {ShowInfo 'After acceptSelect '#FD#' '#{OS.getPID}}
	 {OS.acceptNonblocking_noDnsLookup FD _ _ NewFD} %InAddress InIPPort NewFD}
	 {ShowInfo 'Accepted channel (old '#FD#' new '#NewFD#')'#' '#{OS.getPID}}
	 thread
	    {AcceptProc NewFD}
	    {FDHandler returnResource}
	 end
      % If there is an exception here we can't do much but return the
      % resources and close the socket. The most likely exception is
      % a EPIPE on the new FD.
      catch X then
	 {Show exception_AcceptSelect(X {OS.getPID})}
	 {FDHandler returnResource}
	 try {OS.close NewFD} catch _ then skip end
      end
      {AcceptSelect FD}
   end
   
   proc{Accept ListenPortNum CPortNum NodeName}
%      InAddress InIPPort
      FD
   in
      /* Create socket */
      PID = {OS.getPID}

      FD={OS.socket 'PF_INET' 'SOCK_STREAM' "tcp"}
      if ListenPortNum==default then
	 CPortNum = {BindSocket FD 9000}
      else
	 try
	    {OS.bind FD ListenPortNum}
	    CPortNum=ListenPortNum
	 catch _ then raise unable_to_listen_to(ListenPortNum) end end
      end
      {ShowInfo 'Bound '#CPortNum}
      {OS.listen FD 5}
      NodeName = {OS.uName}.nodename
      {ShowInfo 'Listening on port '#CPortNum#' using fd '#FD#' '#{OS.getPID}}
      thread
	 {AcceptSelect FD}
	 % This should never be reached
	 {Show accept_loop_finished}
	 raise accept_loop_finished end
      end 
   end
   
   proc{AcceptProc FD}
      Read InString
   in
      try
	 {Show accepting}
	 {OS.readSelect FD}
	 {OS.read FD MaxRead ?InString nil ?Read}
	 if Read>0 then
	    case InString of "tcp" then
	       {ShowInfo accepted#' '#PID}
	       _={OS.write FD "ok"}
	       {Glue.acceptConnection FD}
	    [] "give_up" then
	       {OS.close FD}
	    else
	       {OS.close FD}
	    end
	 else
	    % AN! can this happen or will there allways be an exception?
	    {OS.close FD}
	 end
      catch X then
	 {Show acceptProc_caught(X PID)}
	 case X of system(os(_ _ _ "Try again") ...) then % EAGAIN => try again
	    {AcceptProc FD}
	 else % Other fault conditions AN! should some others be treated?
	    {Show acceptProc_caught}
	    try {OS.close FD} catch _ then skip end
	 end
      end
   end
end

