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
         removeApplication:S_removeApplication) at 'methods.ozf'

prepare
   SMeth=meths(getApplicationInfo:S_getApplicationInfo
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
               removeApplication:S_removeApplication)

import
   Browser(browse:Browse)
   System
   OS(uName)
   Connection(take)
   Pickle(save load)
   Module
   DisplayMess at 'messagedisplay.ozf'
   MManager(newSecureManager:NewSecureManager) at 'manager.ozf'
   Mobility(newStationary:NewStationary stationaryClass:StationaryClass) at 'mobility.ozf'
   CGUI(start:StartGUI
        shutdown:StopGUI
        addapp:AddApp
        startapp:AddAppGUI
        invite:InviteClient
        friends:Friends
        removeFriend:RemoveFriend
        updateUser:UpdateUser
        setInfo:SetInfo
        messageAck:MessageAck
        receiveMessage:ReceiveMessage
        getAllMessages:GetAllMessages
        changeStatus:ChangeStatus
        insertMess:InsertMessFromFile
        errorBox:ErrorBox
        removeApp:RemoveApp
        updateApp:UpdateApp) at 'clientgui.ozf'

export
   save: Save
   start: StartClient

define

   WaitQuit

   proc{Save File} {Pickle.save {GetAllMessages} File} end

   proc{Load File $} try {Pickle.load File} catch _ then [messages(id:nil old:nil sent:nil)] end end

   class ClientClass from StationaryClass
      feat
         args  ticketDB
      attr
         id  server  args  name  ServerRef

      meth init(server:S args:A)
         proc{MyServer M}
            A={Label M}
            Secure_A M1
            ID=@id
         in
            if {IsName A} then Secure_A=A else Secure_A=SMeth.A {System.printInfo "* "} end
            M1={Adjoin M Secure_A()}
            {System.print invokeServer(ID A)}
            try {@ServerRef M1} %{System.printInfo ", "} {System.show ok(A)}
            catch X then
               %{System.printInfo ", "} {System.show failed(A)}
               case X of networkFailure(...) then
                  {Browse networkFailure(serverGone X.1)}
                  {ErrorBox "The server has crashed..."}
                  {StopGUI}
                  {Delay 1}
                  unit=WaitQuit
               else
                  {Browse client#X}
                  raise X end
               end
            end
         end
      in
         ServerRef<-S
         server<-MyServer
         args<-A
         self.ticketDB={Dictionary.new}
      end

      meth registerclient(id:L passwd:PW)
         id<-L
         try
            {@server S_login(id:L passwd:PW client:self.this host:{OS.uName}.nodename)}
         catch X then {System.printError X} end
      end

      meth startgui(settings:S<=nil)
         try
            {StartGUI self.this @server @id {Load @args.file} S}
         catch X then {Browse exception(startgui X)} end
      end

      meth haltApplication( application: Instance )
         if {Dictionary.member self.ticketDB Instance} then
            thread
               try
                  {{Dictionary.get self.ticketDB Instance}.module.stop}
               catch _ then skip end
            end
         end
      end

      meth messageAck(id:_ mid:_)=M
         {MessageAck M}
      end

      meth startapplication(id:I) A T F R in
         {@server S_getapplication(id:I application:A)}
         case {Label A} of url then
            thread
               try
                  {self remoteLink(A.serverurl
                                   execute:proc{$ M} {M.start user(id:@id name:@name) T} end
                                   module:F
                                   remote:R)}
               catch X then {Browse application(name:A.name exception:X)} end
            end
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
                  case {Label U} of url then
                     {self remoteLink(U.1 execute:proc{$ M} {M.start user(name:@name id:@id) T} end)}
                  end
               catch X then {Browse exception(startAppClientFailed X)} end
            end
         end
      end

      %% Start a client to a server we are running
      meth startClient(application:AID)
         try A={Dictionary.get self.ticketDB AID} in
            {self remoteLink(A.clienturl execute:proc{$ M} {M.start user(name:@name id:@id) A.ticket} end)}
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
         {StopGUI}
         {System.printError "*** Close client\n"}
         {Delay 1}
         unit=WaitQuit
      end

      meth serverLogout(Msg<=nil)
         if Msg\=nil then
            {ErrorBox "Logout message from server: "#Msg}
         else
            {System.printError "*** Server close-down\n"}
         end
         {StopGUI}
         {Delay 1}
         unit=WaitQuit
      end

      meth apply(F execute:E<=proc{$ _} skip end)
         {System.printError "*** Applying a functor\n"}
         thread
            M1 M={New Module.manager init}
         in
            {M enter(name:'DisplayMess' DisplayMess)}
            M1={M apply(url:'' F $)}
            {E M1}
         end
      end

      meth remoteLink(U execute:E<=proc{$ _} skip end module:F<=_ remote: R<=_)
        % Starter in
        %  Starter = functor $
%                  import
%                     Module
%                  export
%                     stop:Stop
%                  define
%                     M={New Module.manager init}
%                     Stop
%                  in
%                     proc {Stop}
%                        {F.stop}
%                        {Delay 3000}
%                     end
%                     F={M link(url:U $)}
%                     {E F}
%                  end

%        {System.printError "*** Linking a remote functor\n"}

%        R = {New Remote.manager init(host: localhost
%                                       detach: false)}
%        {R apply(url:'' Starter)}
         {self link(U execute:E module:F remote:R)}
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
      try
         S={Connection.take {Pickle.load A.ticketURL}}
         C={NewStationary ClientClass init(server:S args:A)}
      in
         {C registerclient(id:A.login passwd:A.passwd)}
      catch X then
         case X of error(1:connection(ticketToDeadSite _) ...) then
            {ErrorBox "Server ("#A.ticketURL#") is down!"}
            unit=WaitQuit
         else
            {Browse X}
            {ErrorBox "Unknown exception: "#{Label X}}
            unit=WaitQuit
         end
      end
      {Wait WaitQuit}
   end
in
   skip
end
