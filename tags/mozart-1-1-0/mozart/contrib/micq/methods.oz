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

local
   S_init={NewName}
   S_getApplicationInfo={NewName}
   S_getapplication={NewName}
   S_message={NewName}
   S_logout={NewName}
   S_addFriends={NewName}
   S_removeFriend={NewName}
   S_getFriends={NewName}
   S_setStatus={NewName}
   S_searchFriends={NewName}
   S_login={NewName}
   S_updateSettings={NewName}
   S_addApplication={NewName}
   S_editApplication={NewName}
   S_inviteUser={NewName}
   S_addUser={NewName}
   S_removeUser={NewName}
   S_getInfo={NewName}
   S_updateUser={NewName}
   S_messageAck={NewName}
   S_removeMessage={NewName}
   S_getUserName={NewName}
   S_removeApplication={NewName}
   S_getHistory={NewName}
   S_clearHistory={NewName}
   S_getUserInfo={NewName}
   S_getFAQ={NewName}
   S_dumpDB={NewName}
   S_updateFAQ={NewName}
in
   functor 
   export
      init:S_init
      getApplicationInfo:S_getApplicationInfo
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
      getHistory:S_getHistory
      clearHistory:S_clearHistory
      getUserInfo:S_getUserInfo
      getFAQ:S_getFAQ
      dumpDB:S_dumpDB
      updateFAQ:S_updateFAQ
   define
      skip
   end
end


