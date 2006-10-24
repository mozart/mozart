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

functor
    
require
   Meths(getapplication:S_getapplication
	 logout:S_logout
	 login:S_login
	 inviteUser:S_inviteUser
	 removeApplication:S_removeApplication
	 getHistory: S_getHistory
	 setStatus:S_setStatus
	 clearHistory: S_clearHistory) at 'methods.ozf'
import
   Browser(browse:Browse)
%   System
   OS(uName)
   Connection(take)
   Pickle(load)
   Module
   DisplayMess at 'messagedisplay.ozf'
   Remote
   MManager(newSecureManager:NewSecureManager) at 'manager.ozf'
   Mobility(newStationary:NewStationary stationaryClass:StationaryClass) at 'mobility.ozf'
   NewAccountGui(start) at 'newaccountgui.ozf'
   CGUI(start:StartGUI
	shutdown:StopGUI
	addapp:AddApp
	dlgBox:DlgBox
	startapp:AddAppGUI
	invite:InviteClient
	friends:Friends
	removeFriend:RemoveFriend
	updateUser:UpdateUser
	setInfo:SetInfo
	messageAck:MessageAck
	receiveMessage:ReceiveMessage
	changeStatus:ChangeStatus
	insertMess:InsertMessFromFile
	errorBox:ErrorBox
	removeApp:RemoveApp
	updateApp:UpdateApp) at 'clientgui.ozf'
   MozartPower(showLogo:ShowLogo)
export
   start: StartClient

define
   StopLogo StopLogoDelay

   WaitQuit
   
   class ClientClass from StationaryClass
      prop
	 locking
      feat
	 args  ticketDB connecting
      attr
	 id  server  args  name  ServerRef GUIStarted status
	 
      meth init(server:S args:A)
	 proc{MyServer M}
	    try {@ServerRef M} 
	    catch X then
	       case X of networkFailure(...) then
		  {self reConnect(M)}
	       elseof idAllreadyInUse(ID) then
		  raise idAllreadyInUse(ID)  end
	       elseof noSuchMethodInServer(...) then
		  {ErrorBox "You are running a client that is not compliant with the server!\n"#
		   "Please make sure that you are running a client compiled for this particular server, or make "#
		   "sure that you have typed the right ticket URL!\n\n"#
		   "If you have any questions, please send a mail to: nilsf@sics.se or simon@sics.se"}
		  unit=WaitQuit
	       else
		  {Browse client(X M)}
		  raise X end
	       end
	    end
	 end
      in
	 GUIStarted <- false
	 status<-online
	 self.connecting = {NewCell false}
	 ServerRef<-S
	 server<-MyServer
	 args<-A
	 self.ticketDB={Dictionary.new}
	 thread {self pingServer()} end
      end

      meth reConnect(Msg)
	 proc{ConnectLoop}
	    try S in
	       S={Connection.take {Pickle.load @args.ticketURL}}
	       ServerRef <- S
	       {self registerclient(id:@args.login passwd:@args.passwd)}
	       {UnSetCon}
	    catch _ then
	       {Delay 15000}
	       {ConnectLoop}
	    end
	 end
	 fun{SetCon} O in
	    {Exchange self.connecting O true}
	    O
	 end
	 proc {UnSetCon}
	    {Exchange self.connecting _ false}
	 end
      in
	 if {SetCon} then skip
	 else
	    D = {DlgBox "Lost connection with server...\nRetrying..."}
	 in
	    thread if {D wait($)} then
		      {self serverLogout()}
		   end
	    end
	    {ConnectLoop}
	    {D close()}
	    {self setStatus}
	    {@server Msg}
	 end
      end
      
      meth registerclient(id:L passwd:PW)
	 id<-L
	 {@server S_login(id:L passwd:PW client:self.this host:{String.toAtom {OS.uName}.nodename})}
      end

      meth newAccount(A)
	 {NewAccountGui.start {Adjoin A newaccount(id:A.login server:@server client:self.this)}}
      end

      meth clearHistory( friend: F )
	 {@server S_clearHistory( id: @id friend: F)}
      end

      meth pingServer()
	 {Delay 60*1000}
	 {@server ping}
	 {self pingServer()}
      end
      
      meth startgui(settings:S<=nil)
	 if @GUIStarted then skip
	 else
	    try	H={self load($)} in
	       {Wait StopLogoDelay}
	       StopLogo=1
	       {Delay 500}
	       {StartGUI self.this @server @id H S}
	       GUIStarted <- true
	    catch X then {Browse exception(startgui X)} end
	 end
      end
      
      meth load($)
	 thread
	    try
	       {@server S_getHistory(id:@id history:$)}
	    catch _ then nil end
	 end
      end
      
      meth haltApplication( application: Instance )
	 if {Dictionary.member self.ticketDB Instance} then
	    thread
	       M={Dictionary.get self.ticketDB Instance}.module
	       R={Dictionary.get self.ticketDB Instance}.remote
	    in
	       try
		  {M.stop}
	       catch _ then skip end
	       {Delay 10000}
	       if R \= unit then
		  try
		     {R close}
		  catch _ then skip end 
	       end
	       {Dictionary.remove self.ticketDB Instance}
	    end
	 end
      end

      meth messageAck(id:_ mid:_)=M
	 {MessageAck M}
      end
      
      meth setStatus(online: S<=nil)
	 if S\=nil then status <- S end
	 {@server S_setStatus(id: @id online: @status)}
      end
      
      meth startapplication(id:I) A T F R in
	 {@server S_getapplication(id:I application:A)}
	 case {Label A} of url then
	    try ID=@id Name=@name in
	       {self remoteLink(A.serverurl
				execute:proc{$ M} {M.start user(id:ID name:Name) T} end
				module:F
				remote:R)}
	    catch X then {Browse application(name:A.name exception:X)} end
	 elseof functors then
	    T=ticket
	 end
	 {Dictionary.put self.ticketDB A.instance {Adjoin app(ticket:T
							      fullname:A.name#" ("#A.instance#")"
							      module:F remote:R) A}} 
	 {AddAppGUI app(id:A.instance name:A.name#" ("#A.instance#")")}
      end
      
      meth invite(id:UID application:AID)=M
	 try A={Dictionary.get self.ticketDB AID} in
	    {@server S_inviteUser(id:UID client:url(A.clienturl) ticket:A.ticket name:A.fullname
				  sender:@id aid:A.id)}
	 catch X then {Browse inviteUserToJoinFailed(AID X)} end
      end

      %% Start a client that have we have been invited to
      meth inviteUser(id:ID name:N client:U ticket:T sender:S description: D)
	 Ok in
	 thread
	    {InviteClient S N D Ok}
	    if Ok==true then
	       try
		  case {Label U} of url then ID=@id Name=@name in
		     {self remoteLink(U.1 execute:proc{$ M} {M.start user(name:Name id:ID) T} end)}
		  end
	       catch X then {Browse exception(startAppClientFailed X)} end
	    end
	 end
      end

      %% Start a client to a server we are running
      meth startClient(application:AID) 
	 try A={Dictionary.get self.ticketDB AID} ID=@id Name=@name in 
	    {self remoteLink(A.clienturl execute:proc{$ M} {M.start user(name:Name id:ID) A.ticket} end)}
	 catch X then {Browse inviteUserToJoinFailed(AID X)} end
      end

      meth appRemoved(aid:Id) {RemoveApp Id} end
      meth updateapplication(A) {UpdateApp A} end
      meth addapplication(As) thread {AddApp As} end end
      meth removeApplication(aid:Id) {@server S_removeApplication(aid:Id id:@id)} end
      meth friends(online:On offline:Off)=M thread {Friends M} {InsertMessFromFile} end end
      meth addfriends(online:On offline:Off)=M thread {Friends M} end end
      meth removeFriend(friend:F) thread {RemoveFriend F} end end
      
      meth receiveMessage(...)=R thread {ReceiveMessage R} end end
      meth updateUser(...)=M thread {UpdateUser M} end end
      
      meth logout()
	 {@server S_logout(id:@id)}
	 thread {StopGUI} end
	 unit=WaitQuit
      end

      meth serverLogout(Msg<=nil)
	 thread
	    if Msg\=nil then
	       {ErrorBox "Logout message from server: "#Msg}
	    end
	    thread {StopGUI} end
	    unit=WaitQuit
	 end
      end

      meth apply(F execute:E<=proc{$ _} skip end)
	 %{System.printError "*** Applying a functor\n"}
	 thread
	    M1 M={New Module.manager init}
	 in
	    {M enter(name:'DisplayMess' DisplayMess)}  
	    M1={M apply(url:'' F $)}
	    {E M1}
	 end
      end

      meth remoteLink(U execute:E<=proc{$ _} skip end module:F<=_ remote: R<=_)
	 Starter = functor $
		   import
		      Module
		      Browser(browse:Browse)
		   export
		      stop:Stop
		   define
		      SecureFunctor = 
		      \insert './manager.oz'
		      proc {Stop}
			 {F stop}
			 {Delay 3000}
		      end
		      
		      MM = {New Module.manager init}
		      SM M
		      
		   in
		      SM={MM apply(url:'' SecureFunctor $)}
		      
		      M={SM.newSecureManager}
		      try
			 F={M link(url:U $)}
			 {E F}
		      catch X then {Browse applicationExecption#X} end       
		   end  
      in
	 %{System.printError "*** Linking a remote functor\n"}
	 
	 R = {New Remote.manager init(host: localhost fork:sh)}
	 {R apply( Starter)} 
      end
      
      meth link(U execute:E<=proc{$ _} skip end module:F<=_ remote:R<=_)
	 R=unit
	 thread
	    M={NewSecureManager} %{New Module.manager init}
	 in
	    try
	       F={M link(url:U $)}
	       {E F}
	    catch X then {Browse applicationExecption#X} end
	 end
      end

      meth getInfo(info:I)
	 name<-I.name
	 thread {SetInfo I} end
      end
      
      meth notify(id:ID online:O)=M thread {ChangeStatus M} end end
   end

   
   proc{StartClient A}
      Start
   in
      {ShowLogo Start StopLogo}
      thread
	 try
	    S={Connection.take {Pickle.load A.ticketURL}}
	    C={NewStationary ClientClass init(server:S args:A)}
	 in
	    Start=500
	    thread {Delay 3000} StopLogoDelay=unit end
	    if A.newuser==false then
	       {C registerclient(id:A.login passwd:A.passwd)}
	    else
	       {C newAccount(A)}
	    end
	 catch X then
	    case X of error(1:connection(ticketToDeadSite _) ...) then
	       {ErrorBox "Server ("#A.ticketURL#") is down!"}
	       unit=WaitQuit
	    else
	    %{Browse X}
	       thread
		  {ErrorBox "Unknown exception: "#{Label X}}
		  unit=WaitQuit
	       end
	       raise X end
	    end
	 end
      end
      {Wait WaitQuit}
      {Delay 3000}
   end
in
   skip
end
