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
   Meths(getApplicationInfo:S_getApplicationInfo
	 getapplication:S_getapplication
	 message:S_message
	 logout:S_logout
	 addFriends:S_addFriends
	 removeFriend:S_removeFriend
	 getFriends:S_getFriends
	 setStatus:S_setStatus
	 searchFriends:S_searchFriends
	 login:S_login
	 updateSettings:S_updateSettings
	 addApplication:S_addApplication
	 editApplication:S_editApplication
	 inviteUser:S_inviteUser
	 addUser:S_addUser
	 removeUser:S_removeUser
	 getInfo:S_getInfo
	 updateUser:S_updateUser
	 messageAck:S_messageAck
	 removeMessage:S_removeMessage
	 getUserName:S_getUserName
	 removeApplication:S_removeApplication
	 getHistory: S_getHistory
	 clearHistory: S_clearHistory
	 getUserInfo: S_getUserInfo
	 getFAQ:S_getFAQ
	 dumpDB:S_dumpDB
	 updateFAQ:S_updateFAQ) at 'methods.ozf'
import
   Tk
   System(show showInfo)
   Connection(gate)
   Pickle(save load)
   DP(open) at 'x-oz://contrib/tools/DistPanel'
   Panel
   Fault
   Browser(browse:Browse)
   Database(db:DB getID:GetID) at 'database.ozf'
   Mobility(stationaryClass:StationaryClass newStationary:NewStationary) at 'mobility.ozf'
   Log at 'log.ozf'
   OS(uName localTime)
   Sysadm(start) at 'sysadm.ozf'
   Property
export
   start:StartServer
    
define
   Logger  Gate
   proc{WriteLog M}
      {Logger log(M)}
      {System.showInfo M}
   end
   InitServer = {NewName}
   HaltServer = {NewName}
   WaitQuit
   
   fun{GetDate}
      D={OS.localTime}
      fun{F X} if X<10 then "0"#X else X end end
   in
      date(year:if D.year<50 then 2000 else 1900 end + D.year 
	   date:{F D.mDay}#"/"#({F D.mon+1})
	   time:{F D.hour}#":"#{F D.min})
   end

   class ServerClass from StationaryClass
      prop locking final
      feat dbdir watchers

      meth ping()
	 skip
      end
	 
      meth !InitServer(dbdir:D)
	 self.dbdir=D
	 self.watchers={Dictionary.new}
	 {DB setDBdir( dir :D )}
      end

      %% Methods called from editapplicationgui.oz
      meth !S_getApplicationInfo(id:Id info:?I)
	 try {DB getApplicationInfo(id:Id info:I)}
	 catch _ then {WriteLog "Could not return application info about "#Id} end
      end
      
      %% Methods called from clients
      meth !S_getapplication(id:I application:?A)
	 try {DB getApplication(id:I application:A)}
	 catch _ then {WriteLog "Could not return applicaion "#I} end
      end
      
      meth !S_message(receiver:ID message:M sender:SID reply_to:R mid:Mid date:Date faq:FAQ<=unit)
	 D={GetDate} GlobalMID={GetID} MM
      in
	 {WriteLog "Received and stored message "#GlobalMID#" from "#SID#" to: "#{Value.toVirtualString ID 30 30}}
	 {DB storeMessage(receiver:ID id:GlobalMID sender:SID message:M date:D reply_to:R)}
	 if FAQ\=unit then %E={DB get(id:SID entry:$)} in
	    {WriteLog "Message "#GlobalMID#" is added to FAQ by "#SID}
	    {DB storeX(id:GlobalMID data:faq(poster:SID answer:M date:Date question:FAQ))}
	    MM="-- This message is added to the FAQ --\n"#M#"\n\n-- Answer ----------------------------\n"#FAQ
	 else
	    MM=M
	 end
	 {ForAll ID proc {$ I}
		       thread E in
			  try E={DB get(id:I entry:$)}
			  catch _ then
			     {WriteLog "Canceling message to "#I#", unknown receiver"}
			     {self S_messageAck( id: I mid: GlobalMID )} E=nil
			  end
			  if E\=nil then
			     try C={DB getClient(id:I client:$)} in
				{WriteLog "Forward message "#GlobalMID#" to "#E.name}
				{C receiveMessage(mid:GlobalMID message:MM sender:SID date:D reply_to:R)
				 "Message ("#GlobalMID#") forwarding to "#E.name#" failed"}
			     catch networkFailure(...) then skip end
			  end
		       end
		    end}
	 try
	    Mid = GlobalMID % Bind the message id back to sender
	    Date = D
	 catch _ then {WriteLog "Could not return messageID("#GlobalMID#") and date to "#SID} end
      end

      meth !S_clearHistory( id: ID friend: F)
	 {DB clearHistory( id: ID friend: F)}
      end
      
      meth !S_getHistory( id:ID history:History )
	 H={DB getHistory( id: ID history: $ )}
	 Hs={Map H fun{$ X} _#X end}
	 H1={Map Hs fun{$ X} X.1 end}
      in
	 try
%	    {Browse H}
	    History=H1
	    {ForAll Hs proc{$ X} X.1=X.2 {Delay 100} end}
	 catch _ then {WriteLog "Could not return complete history to "#ID} end
      end
 
      meth !S_logout(id:ID client:C<=nil)
	 lock P={Dictionary.condGet self.watchers ID nil} in
	    if P\=nil then
	       {Dictionary.remove self.watchers ID}
	       {Fault.deInstallWatcher P.port P.procedure _}
	    end
	    {self Notify(id:ID online:offline)}
	    {WriteLog "Logged out "#ID}
	    try 
	       {DB logout(id:ID)}
	    catch _ then {WriteLog ID#" is not online"} end
	       
	    if C\=nil then
	       {C serverLogout() "Can not logout "#ID }
	    end
	 end
      end

      meth !S_addFriends(id:ID friends:Fs) On Off On1 Off1 in
	 {WriteLog ID#" add friends: "#{Value.toVirtualString Fs 30 30}}
	 thread
	    try C={DB getClient(id:ID client:$)} in
	       {DB addFriends(id:ID friends:Fs)}
	       {DB getFriendsStatus(id:ID online:On offline:Off)}
	       On1={Filter On fun{$ X} {List.member X.id Fs} end}
	       Off1={Filter Off fun{$ X} {List.member X.id Fs} end}
	       {C addfriends(online:On1 offline:Off1) "Add friends Failed"}
	    catch networkFailure(...) then skip end
	 end
      end
      
      meth !S_removeFriend(id:ID friend:F)
	 {WriteLog ID#" removes "#F}
	 {DB removeFriend(id:ID friend:F)}
      end

      meth !S_getFriends(id:ID friends:$) {DB getFriends(id:ID friends:$)} end
      
      meth !S_setStatus(id:ID online:O)
	 try C={DB getClient(id:ID client:$)} in
	    {self Notify(id:ID online:O)}
	    {WriteLog ID#" went "#O}
	    {DB setOnline(id:ID online:O client:C)}
	 catch networkFailure(...) then skip end
      end
      
      meth !S_searchFriends(id:ID<=nil firstname:FName<=nil lastname:LName<=nil
			 organization:Org<=nil email:Email<=nil hits: H)
	 try
	    H = {DB search(id:ID firstname:FName lastname:LName organization:Org
			   email:Email hits:$)}
	 catch _ then {WriteLog "Could not return value in 'search for friends'"} end
      end
      
      meth !S_login(passwd:PW id:ID client:C1 host:H)
	 proc{Autologout ID}
	    {Delay 1000}
	    if {DB isOnline(id:ID online:$)}\=false then
	       try
		  {self S_logout(id:ID)}
		  {WriteLog ID#" on "#H#" is autologged out"}
	       catch X then
		  case X of noSuchEntry(...) then
		     {WriteLog "Autologout failed for "#ID}
		  else {Browse autologoutException(X)} end 
	       end
	    end
	 end
	 
	 proc{C M Log}
	    try {C1 M}
	    catch networkFailure(X) then 
	       {WriteLog "* Networkfailure ["#{Label X}#"]: "#Log#" ("#ID#" on host "#H#")"}
	       {Autologout ID}
	       raise networkFailure(X) end
	    end
	 end
	 fun{NewFun Mess}
	    functor $
	    import
	       Tk
	       DisplayMess(display)
	    define
	       T={New Tk.toplevel tkInit(title:"Login failed")}
	       L={New Tk.label tkInit(parent:T text:Mess)}
	       B0={New Tk.button tkInit(parent:T text:"Help"
					action:proc{$} {DisplayMess.display "Help"
							"If this is your first time you connect, you have no account!\n"#
							"If you don't have an account, you must create one.\n\n"#
							" - Press the button 'Create Account' to create a new account.\n"#
							" - Use <Tab> to traverse between the fields,\n"#
							"   press <Return> when done\n\n"#
							"This help will improve someday!"
							"Close Help"}
					       end)}
	       B1={New Tk.button
		   tkInit(parent:T text:"New Login"
			  action:proc{$}
				    {T tkClose}
				    {C apply(functor $
					     import Tk
					     define
						GO
						T={New Tk.toplevel tkInit(title:"Login")}
						V1={New Tk.variable tkInit(ID)}
						V2={New Tk.variable tkInit(PW)}
						L1={New Tk.label tkInit(parent:T text:"Login")}
						L2={New Tk.label tkInit(parent:T text:"Password")}
						E1={New Tk.entry tkInit(parent:T width:50 textvariable:V1)}
						E2={New Tk.entry tkInit(parent:T width:50 textvariable:V2)}
						{Tk.batch [grid(L1 row:3 column:0 sticky:e)
							   grid(E1 row:3 column:1 sticky:w)
							   grid(L2 row:6 column:0 sticky:e)
							   grid(E2 row:6 column:1 sticky:w)
							   focus(E1)]}
						{E1 tkBind(event:'<Return>' action:proc{$} GO=unit end)}
						{E2 tkBind(event:'<Return>' action:proc{$} GO=unit end)}
						proc{Start}
						   ID1={V1 tkReturnAtom($)}
						   PW1={V2 tkReturnAtom($)}
						in
						   {Wait ID1} {Wait PW1}
						   {T tkClose}
						   {C1 registerclient(id:ID1 passwd:PW1)}
						end
					     in
						thread
						   {Wait GO}
						   {Start}
						end
					     end) "Could not apply functor 'newlogin'"}
				 end)}
	       B2={New Tk.button tkInit(parent:T text:"New User (Create Account)"
					action:proc{$}
						  {T tkClose}
						  try
						     {C1 newAccount(args(login:ID passwd:PW))} 
						  catch X then raise X end end
					       end)}
	       B3={New Tk.button tkInit(parent:T text:"Quit" action:proc{$}
								       {T tkClose}
								       {C1 serverLogout}
								    end)}
	    in
	       {Tk.batch [grid(L row:0 column:0 columnspan:4 sticky:we)
			  grid(B0 row:5 column:0 sticky:we)
			  grid(B1 row:5 column:1 sticky:we)
			  grid(B2 row:5 column:2 sticky:we)
			  grid(B3 row:5 column:3 sticky:we)
			  focus(B0)]}
	    end
	 end
	 proc{WatcherProc X E}
	    {WriteLog "System detected client crash for "#ID#" on "#H#". Initialize autologgout..."} 
	    {Autologout ID}
	 end
      in
	 {WriteLog ID#" connects from "#H#"..."}
	 try CP E={DB get(id:ID entry:$)} in

	    if PW == E.passwd then
	       %% Add handlers and watchers on the client
	       {C1 'GETPORT'(CP)}
	       {Fault.installWatcher CP [permFail] WatcherProc _}

	       %% Check if client is allready logged in...
	       if {DB isOnline(id:ID online:$)} \= false then OldC in
		  %% Some nice error message to the client
		  {WriteLog E.name#" ("#ID#") is already logged in!"}
		  {DB getClient(id:ID client:OldC)}
		  if OldC\=false then
		     thread
			{WriteLog "Logout old client ("#ID#")"}
			{self S_logout(id:ID client:OldC)}
		     end
		  end
		  {Delay 2500}
	       end
	       
	       %% So that we can uninstall watcher
	       {Dictionary.put self.watchers ID entry(port: CP procedure: WatcherProc )}
	       
	       lock
		  {DB login(id:ID client:C)}
		  local On Off in
		     {DB getFriendsStatus(id:ID online:On offline:Off)}
		     {C friends(online:On offline:Off) "Sending friends information failed"}
		  end
		  {self Notify(id:ID online:online)}
		  {WriteLog E.name#" ("#ID#") logged on!"}
	       end

	       %% Initialize the client
	       {C startgui(settings:E.settings) "Could not start client"}
	       	       
	       {C getInfo(info:{DB get(id:ID entry:$)}) "Send personal info to client failed"}

	       %% Send stored messages to client
	       M = {DB getMessages(id:ID messages:$)} in
	       thread
		  {ForAll M proc {$ X}
			       {WriteLog "Forward stored message ("#X.id#") to "#E.name}
			       try 
				  {C receiveMessage(mid:X.id
						    message:X.message
						    sender:X.sender
						    date:X.date
						    reply_to:X.reply_to) "Message delivery failed for "#X.id}
			       catch _ then skip end
			    end}
	       end

	       %% Send out all registered applications
	       {C addapplication({DB getApps(entries:$)}) "Can't send registered applications to client"}
	    else % User entered wrong password
	       {WriteLog E.name#" ("#ID#") uses wrong password: "#PW}
	       {C apply({NewFun "Wrong password for '"#ID#"', login failed!"})
		"Send 'newlogin' functor to client failed"}
	    end
	    
	 catch noSuchEntry(X) then
	    {WriteLog "Unknown user ("#X#") is refused access..."}
	    {C apply({NewFun "Login as '"#ID#"' failed!"}) "Send 'newlogin' functor to client failed"}
	 [] networkFailure(...) then {System.show '-'} skip
	 end
      end

      meth !S_updateSettings(id:Id settings:S) {DB updateSettings(id:Id settings:S)} end
      
      meth !S_addApplication(name:N serverurl:S clienturl:C author:A description:D) ID in
	 {DB addApplication(id:ID name:N serverurl:S clienturl:C author:A description:D)}
	 {WriteLog A#" adds application "#N#" ("#ID#")"}
	 thread
	    {ForAll {DB getAllOnline(entries:$)} proc{$ X}
						    try
						       {X.client addapplication([app(id:ID name:N author:A)])
							"Can not add application "#N#" to client("#X.id#")"}
						    catch _ then skip end
						 end}
	 end
      end
      
      meth !S_editApplication(name:N serverurl:S clienturl:C author:A id:ID description:D)
	 {DB updateApplication(id:ID name:N serverurl:S clienturl:C author:A description:D)}
	 {WriteLog A#" updates application "#N#" ("#ID#")"}
	 thread
	    {ForAll {DB getAllOnline(entries:$)} proc{$ X}
						    try
						       {X.client updateapplication(app(id:ID name:N author:A))
							"Can not update application "#N#" to client "#X.id}
						    catch _ then skip end
						 end}
	 end
      end

      meth !S_inviteUser(id:Id sender:S ticket:T client:C name:N aid: Aid) 
	 {WriteLog S#" invites "#Id#" to join "#N#" [instance: "#Aid#"]"}
	 if {DB isOnline(id:Id online:$)}\=false then Cl={DB getClient(id:Id client:$)} in
	    try
	       {Cl inviteUser(sender:S ticket:T client:C
			      description: {DB getApplicationInfo( id:Aid info:$ )}.description
			      id:Id name:N) "Could not invite "#Id#" to "#N#" from "#S}
	    catch _ then skip end
	 end
      end

      
      meth !S_addUser(firstname:F lastname:L friends:Fr<=nil organization:O
		      email:E passwd:P id:U userlevel:UL extra:Xtra<=nil)
	 {WriteLog "Create account for "#U#" ("#F#" "#L#", "#O#")"}
	 try
	    {DB addUser(id:U
			firstname:F
			lastname:L
			organization:O
			friends:Fr
			email:E
			passwd:P
			userlevel: UL
			extra:Xtra)}
	 catch idAllreadyInUse(M) then
	    {WriteLog "Id '"#U#"' is allready taken!"}
	    raise idAllreadyInUse(M) end
	 end
      end
      
      meth !S_removeUser(id:Id)
	 {WriteLog "User "#Id#" is being removed!"}
	 if {DB isOnline( id:Id online:$ )}\=false then
	    try C={DB getClient(id:Id client:$)} in
	       {C serverLogout() "Can't logout "#Id}
	    catch networkFailure(...) then skip
	    end
	    {self S_logout( id: Id )}
	 end

	 try
	    %% Remove from others friendslist
	    {ForAll {DB getNotify( id: Id notify: $)} proc {$ X}
							 if {DB isOnline(id:X online:$)}\=false then
							    try
							       C={DB getClient(id:X client:$)}
							    in
							       {C removeFriend(friend: Id)
								"Can't remove friend ("#Id#") from client ("#X#")"}
							    catch networkFailure(...) then skip end
							 end
							 {self S_removeFriend( id: X friend: Id )}
						      end}
	 catch _ then {WriteLog "Id '"#Id#"' can't be removed from friendlist!"} end	 
	 %% Remove friends - include removing from others notifylists.
	 try
	    {ForAll {DB getFriends( id: Id friends: $)} proc {$ X}
							   {self S_removeFriend(id:Id friend:X)}
							end}
	 catch _ then {WriteLog "Id '"#Id#"' can't remove friends!"} end	 
	 %% Remove all messages send by removed dude.
	 try
	    {DB removeMessagesFrom( id: Id )}
	 catch _ then {WriteLog "Id '"#Id#"' can't remove his messages!"} end
	 %% Mark all messages to removed dude as read. 
	 try
	    {ForAll {DB getMessages( id: Id messages: $ )} proc {$ X}
							      {self S_messageAck(id:Id
										 mid:X.id )}
							   end}
	 catch _ then {WriteLog "Id '"#Id#"' can't ack his messages!"} end

	 try
	    {DB removeUser( id: Id)}
	 catch _ then {WriteLog "Id '"#Id#"' can't be removed!"} end
      end

      meth !S_getUserInfo( id:Id info:I)
	  try 
	    I = {DB getUserInfo(id:Id entry:$)}
	 catch _ then {WriteLog "Can't return info about "#Id} end
      end
      
      
      meth !S_getInfo(id:Id info:I)
	 try 
	    I = {DB get(id:Id entry:$)}
	 catch _ then {WriteLog "Can't return info about "#Id} end
      end
      
      meth !S_updateUser(firstname:F lastname:L organization:O email:E
			 passwd:P id:U userlevel:UL extra:Xtra<=nil) ON I in
	 %% Update the databse
	 {DB updateUser(id:U firstname:F lastname:L organization:O email:E passwd:P
		        userlevel: UL extra:Xtra)}

	 %% Update the client
	 try 
	    {{DB getClient(id:U client:$)} getInfo(info:{DB get(id:U entry:$)})
	     "Updating personal information failed"}
	 catch networkFailure(...) then skip end
	 
	 %% Notify all friends
	 {DB getNotifyStatus(id:U online:ON offline:_)}
	 {DB get(id:U entry:I)}
	 {ForAll ON proc{$ N}
		       try C={DB getClient(id:N.id client:$)} in
			  {C updateUser(id:I.id firstname:I.firstname lastname:I.lastname name:I.name
					organization:I.organization email:I.email online:I.online)
			   "Could not send update info about "#U#" to "#N.id}
		       catch networkFailure(...) then skip end
		    end}
	 
	 %% Save the database
	 {WriteLog "*** Saving database (update of user information for "#U#")"}
	 {DB saveAll(dir:self.dbdir)}
      end
      
      meth !S_messageAck(id:Id mid:Mid) Sender C in
	 {WriteLog "Message ("#Mid#") has been read by "#Id}
	 thread M=Mid in
	    try 
	       Sender={DB getSender( mid: M sender:$)}
	       C={DB getClient(id: Sender client:$)}
	       {C messageAck(id:Id mid:M) "Couldn't send messageAck("#M#") to sender"}
	    catch _ then {WriteLog "Couldn't send messageAck("#M#") from "#Id#" to sender"} end
	 end
	 thread M=Mid in
	    if {DB messageAck(id:Id mid:M read:$)} then
	       {self S_removeMessage(mid:M)}
	    end
	 end
      end

      meth !S_removeMessage(mid:Mid)
	 try
	    {DB removeMessage(mid:Mid)}
	    {WriteLog "Message ("#Mid#") has been removed from server"}
	 catch _ then skip end
      end

      %getUserName
      meth !S_getUserName(id:ID name:?N)
	 Es={DB items($)}
	 fun{Find Es1}
	    case Es1 of A|Bs then
	       if A.id==ID then A
	       else {Find Bs} end
	    else raise noSuchEntry(ID) end
	    end
	 end
      in
	 try
	    N={Find Es}.name
	 catch _ then {WriteLog "Could not return username for "#ID} end
      end
      
      meth !S_removeApplication(aid:Aid id:Id)
	 try
	    {DB removeApplication( id: Aid author: Id)}
	    {WriteLog "["#Id#"] removes application ("#Aid#")"}
	    {self RemoveAppFromClients( aid: Aid )}
	 catch _ then
	    {WriteLog "["#Id#"] failed to remove application ("#Aid#")"} end
      end

      %% Unprotected methods (will change when methods.oz is recomiled)
      meth !S_dumpDB(uid:UID Ans) A Rec={DB toRecord(record:$)} in
	 {WriteLog UID#" requests database-dump"}
	 A=db(members:{Map {Record.toList Rec.membersDB} fun{$ X}
							    user(id:X.id email:X.email firstname:X.firstname lastname:X.lastname
								 organization:X.organization userlevel:X.userlevel extra:X.extra)
							 end}
	      online:{Map {Record.toList Rec.onlineDB} fun{$ X} X.id#X.online end}
	     )
	 try
	    Ans=A
	 catch _ then
	    {WriteLog "Can't return database dump to "#UID}
	 end
      end
      meth !S_getFAQ($ host:H<=unit)
	 if H\=unit then
	    {WriteLog "Http FAQ request from "#H}
	 end
	 {Map {DB entriesX($)} fun{$ X} X.2 end}
      end
      meth !S_updateFAQ(id:ID data:Data)
	 if Data\=unit then
	    {DB updateX(id:ID data:Data)}
	 else
	    {DB removeX(id:ID)}
	 end
      end
      
      %% Local methods (used ONLY by server)
      meth !HaltServer(1:Msg<=nil)
	 {Gate close}
	 {WriteLog "*** System is halting..."}
	 {WriteLog "*** Saving database"}

	 try {DB saveAll(dir:self.dbdir)}
	 catch X then {System.show 'haltServer0'#X} end
	 
	 thread
	    %% If the server failed to shutdown nice within 2 minutes then we kill it anyway!
	    {Delay 2*60000}
	    {WriteLog "*** Timeout"}
	    WaitQuit=unit
	 end
	 
	 {ForAll {DB items($)} proc{$ X}
				  try C={DB getClient(id:X.id client:$)} in
				     {WriteLog "*** Send 'logout' to: "#X.id}
				     {C serverLogout(Msg) "Failed to logout "#X.id}
				  catch networkFailure(...) then skip end
			       end}
	 unit=WaitQuit
      end

      meth RemoveAppFromClients(aid:Id)
	 thread
	    {ForAll {DB getAllOnline( entries: $)}
	     proc{$ X}
		try
		   {X.client appRemoved(aid:Id) "Failed to remove application ("#Id#") from client ("#X.id#")"}
		catch _ then skip end
	     end}
	 end
      end
      
      meth Notify(id:ID online:O)=M ON in
	 {DB getNotifyStatus(id:ID online:ON offline:_)}
	 {ForAll ON proc{$ N}
		       thread
			  try C={DB getClient(id:N.id client:$)} in
			     {C notify(id:ID online:O) "Could not notify ("#O#") "#N.id#" from "#ID}
			  catch networkFailure(...) then skip end
		       end
		    end}
      end

      meth otherwise(X)
	 {WriteLog "* Illeagal access: "#{Value.toVirtualString X 30 30}}
	 {Browse illegalAccesByClient(X)}
	 raise noSuchMethodInServer(X) end
      end
   end
   
   proc{StartServer Args}
      EnterTicket
      S={NewStationary ServerClass InitServer(dbdir:Args.dbdir)}
   in
      Logger = {New Log.log init(file:Args.dbdir#"server.log")}
      {WriteLog "*** Server is started ***"}
      {System.showInfo "Logfile is "#Args.dbdir#"server.log"}
      {DB loadAll(dir:Args.dbdir)}
      {WriteLog "Database is loaded"}
      
      Gate={New Connection.gate init(S EnterTicket)}
      {Pickle.save EnterTicket Args.ticketSave}
      {WriteLog "Ticket is saved to "#Args.ticketSave}

      thread
	 D={GetDate}
	 T1={New Tk.toplevel tkInit(title:"Server Interface (started: "#D.year#"-"#D.date#":"#D.time#")"
				    delete:proc{$}
					      {T tkClose}
					      WaitQuit=unit
					   end)}
	 T={New Tk.frame tkInit(parent:T1)}
	 proc{Separator Name F}
	    L fun{NS} {New Tk.frame tkInit(parent:F bd:1 relief:sunken height:2 width:10)} end
	 in 
	    F={New Tk.frame tkInit(parent:T)}
	    L={New Tk.label tkInit(parent:F text:Name)}
	    {Tk.batch [grid({NS} row:0 column:0 sticky:we)
		       grid(L    row:0 column:1 padx:4)
		       grid({NS} row:0 column:2 sticky:we)
		       grid(columnconfigure F 0 weight:1)
		       grid(columnconfigure F 2 weight:1)]}
	 end

	 %% Debug frame
	 DER=0
	 B0={New Tk.button tkInit(parent:T text:"Distribution Panel" relief:groove
				  action:proc{$}
					    {WriteLog "Start Distribution Panel on Server"}
					    {DP.open}
					 end)}
	 B1={New Tk.button tkInit(parent:T text:"Oz Panel" relief:groove
				  action:proc{$}
					    {WriteLog "Start Panel on Server"}
					    {Panel.object open}
					 end)}
	 {Tk.batch [grid({Separator "Debug"} row:DER column:0 columnspan:2 sticky:we pady:3)
		    grid(B0  row:DER+1 column:0 sticky:we)
		    grid(B1 row:DER+1 column:1 sticky:we)]}

	 
	 %% DB frame
	 DBR=4
	 B2={New Tk.button tkInit(parent:T text:"Browse Database" relief:groove
				  action:proc{$} Rec = {DB toRecord(record:$)} in
					    {WriteLog "Browse database"}
					    {ForAll [onlineDB membersDB id messageDB friendsDB notifyDB appid
						     applicationDB]	  
					     proc{$ F}{Browse Rec.F} end}
					 end)}
	 B2a={New Tk.button tkInit(parent:T text:"Who is Online?" relief:groove
				   action:proc{$} Rec = {DB toRecord(record:$)} in
					     {WriteLog "Browse online"}
					     {Browse online#{Map {Record.toList Rec.onlineDB} fun{$ X} X.id end}}
					  end)}
	 B2b={New Tk.button tkInit(parent:T text:"Browse Accounts" relief:groove
				   action:proc{$} Rec = {DB toRecord(record:$)} in
					     {WriteLog "Browse accounts"}
					     {Browse accounts#{Map {Record.toList Rec.membersDB} fun{$ X} L=X.id in L(X.firstname X.lastname) end}}
					  end)}
	 B2c={New Tk.button tkInit(parent:T text:"Browse Applications" relief:groove
				   action:proc{$} Rec = {DB toRecord(record:$)} in
					     {WriteLog "Browse applications"}
					     {Browse applications#{Map {Record.toList Rec.applicationDB} fun{$ X} L=X.author in L(X.id X.name) end}}
					  end)}
	 
	 {Tk.batch [grid({Separator "Database"} row:DBR column:0 columnspan:2 sticky:we pady:3)
		    grid(B2  row:DBR+1 column:0 sticky:we)
		    grid(B2a row:DBR+1 column:1 sticky:we)
		    grid(B2b row:DBR+2 column:0 sticky:we)
		    grid(B2c row:DBR+2 column:1 sticky:we)]}

	 %% Snapshot frame
	 SNR=7
	 B3={New Tk.button tkInit(parent:T text:"Make Snapshot" relief:groove
				  action:proc{$}
					    {WriteLog "Snapshot of Database"}
					    {DB saveAll(dir:Args.dbdir)}
					 end)}
	 B4={New Tk.button tkInit(parent:T text:"Browse Snapshot" relief:groove
				  action:proc{$}
					    {WriteLog "Browse Snapshot"}
					    {ForAll [members id message friends appid application]
					     proc{$ F}
						{Browse {Pickle.load Args.dbdir#F#'.icq'}}
					     end}
					 end)}
	 {Tk.batch [grid({Separator "Snapshot"} row:SNR column:0 columnspan:2 sticky:we pady:3)
		    grid(B3 row:SNR+1 column:0 sticky:we)
		    grid(B4 row:SNR+1 column:1 sticky:we)]}


	 %% Administrator frame
	 ADR=9
	 OF1={New Tk.frame tkInit(parent:T)}
	 COV={New Tk.variable tkInit(0)}
	 COL={New Tk.label tkInit(parent:OF1 text:"Clients Online:")}
	 COE={New Tk.entry tkInit(parent:OF1 textvariable:COV state:disabled justify:right)}
	 AF1={New Tk.frame tkInit(parent:T)}
	 CAL={New Tk.label tkInit(parent:AF1 text:"Number of Accounts:")}
	 CAV={New Tk.variable tkInit(0)}
	 CAE={New Tk.entry tkInit(parent:AF1 textvariable:CAV state:disabled justify:right)}
	 {Tk.batch [grid(COL row:0 column:0 sticky:e)
		    grid(COE row:0 column:1 sticky:we padx:2)
		    grid(CAL row:0 column:0 sticky:e)
		    grid(CAE row:0 column:1 sticky:we padx:2)
		    grid(columnconfigure OF1 1 weight:1)
		    grid(columnconfigure AF1 1 weight:1)]}

	 HSV={New Tk.variable tkInit('')}
	 HSE={New Tk.entry tkInit(parent:T textvariable:HSV bg:white fg:red)}
	 B5={New Tk.button tkInit(parent:T relief:groove
				  text: "Administrate"
				  action: proc {$}
					     {WriteLog "Start Administrator"}
					     {Sysadm.start user(id:server name:"Server") EnterTicket}
					  end)}
	 B6={New Tk.button tkInit(parent:T relief:groove
				  fg:red activeforeground:red
				  text:"Halt Server" action:proc{$}
							       {WriteLog "Halt Server"}
							       {S HaltServer({HSV tkReturnString($)})}
							    end)}
	 {Tk.batch [grid({Separator "Administration"} row:ADR column:0 columnspan:2 sticky:we pady:3)
		    grid(OF1 row:ADR+1 column:0 sticky:we padx:7)
		    grid(AF1 row:ADR+1 column:1 sticky:we padx:7)
		    grid(B5 row:ADR+2 column:0 sticky:we)
		    grid(B6 row:ADR+2 column:1 sticky:we)
		    grid(HSE row:ADR+3 column:0 sticky:we columnspan:2 pady:1)]}
      in
	 {Tk.batch [grid(T row:0 column:0 sticky:news padx:4 pady:1)
		    grid(columnconfigure T1 0 weight:1)
		    grid(columnconfigure T 0 weight:1)
		    grid(columnconfigure T 1 weight:1)
		    wm(resizable T1 1 0)]}
	 thread
	    proc{Loop} O T in
	       {DB getNoOfUsers(online:O total:T)}
	       {COE tk(config state:normal)}
	       {COV tkSet(O)}
	       {COE tk(config state:disabled)}
	       {CAE tk(config state:normal)}
	       {CAV tkSet(T)}
	       {CAE tk(config state:disabled)}
	       {Delay 5000}
	       {Loop}
	    end
	 in
	    {Delay 6000}
	    {Loop}
	 end
	 
	 thread
	    {Thread.setThisPriority high}
	    proc{SaveLoop}
	       {Delay 20*60000}
	       {DB saveAll(dir: Args.dbdir)}
	       {WriteLog "Autosave"}
	       {SaveLoop}
	    end
	 in
	    {SaveLoop}
	 end
      end
      
      {Wait WaitQuit}
      {WriteLog "Server is going down immediately...\n-----------------------------------------------------------\n"}
      {Logger close}
      {Delay 2500}
   end
   OSinfo = {OS.uName}
in
   {Property.put 'errors.toplevel' proc {$} {WriteLog "* Unhandled exception"} skip end}
   {System.showInfo  "Server is running on "#OSinfo.nodename#" ("#OSinfo.sysname#" "# 
    OSinfo.release#" "# OSinfo.machine#")"}
end


