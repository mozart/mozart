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
   
import
   Pickle(save load)
   System
   OS(stat)
export
   db: InvokeDB
   getID:GetNewID
define
   Counter={NewCell 0}
   AppCounter={NewCell 0}
   InstCounter={NewCell 0}
   DB_extension=".icq"
   DB InvokeDB
   
   fun{GetNewID} O N in
      {Exchange Counter O N}
      N=O+1
   end

   fun{FileTimeStamp File} Time1 Time2 in
      try
	 Time1 = {OS.stat File#"1"#DB_extension}.mtime
      catch _ then Time1 = 0 end
      try
	 Time2 = {OS.stat File#"2"#DB_extension}.mtime
      catch _ then Time2 = 0 end
      file(1:Time1 2:Time2)
   end

   fun{OldestFile File}
      Time={FileTimeStamp File}
   in
      if Time.2 > Time.1 then
	 File#"1"#DB_extension
      else
	 File#"2"#DB_extension
      end
   end
   
   fun{NewestFiles File}
      Time={FileTimeStamp File}
   in
      if Time.1 == 0 then
	 if Time.2 == 0 then nil
	 else [File#"2"#DB_extension] end
      elseif Time.2 == 0 then
	 [File#"1"#DB_extension]
      elseif Time.2 > Time.1 then
	 [ File#"2"#DB_extension 
	   File#"1"#DB_extension] 
      else
	 [File#"1"#DB_extension  File#"2"#DB_extension] 
      end
   end
	 
   fun{GetAppID} O N in
      {Exchange AppCounter O N}
      N=O+1
   end

   fun{GetInstance} O N in
      {Exchange InstCounter O N}
      N=O+1
   end

   local
      fun {NoDoublesR Lr Ack}
	 if Lr==nil then Ack
	 else L|Ls = Lr in
	    if {List.subtract Ls L} == Ls then
	       {NoDoublesR Ls {Append Ack [L]}}
	    else
	       {NoDoublesR {Append [L] {List.subtract Ls L}} Ack}
	    end
	 end
      end
   in
      fun{NoDoubles List}
	 {NoDoublesR List nil}
      end
   end
   
   MsgLock = {NewLock}
   
   proc { SaveMessID File }
      {Pickle.save messageID(id: {Access Counter}) {OldestFile File}}
   end

   fun{LoadCounter Files}
      fun{LoadValue F} 
	 {Pickle.load F}.id
      end
      fun{Load Files A}
	 if Files == nil then A
	 else
	    V F|Fs = Files
	 in
	    try 
	       V={LoadValue F}
	    catch _ then V={Load Fs 0} end
	    V
	 end
      end
   in
      {Load Files 0}
   end
   
   proc { LoadMessID File }
      {Assign Counter {LoadCounter {NewestFiles File}}}
   end
   
   proc { SaveAppID File }
      {Pickle.save appID(id: {Access AppCounter}) {OldestFile File}}
   end

   proc { LoadAppID File }
      {Assign AppCounter {LoadCounter {NewestFiles File}}}
   end
    
   fun { ToLowerString Value }
      {List.map {VirtualString.toString Value#""} fun {$ C}
						     {Char.toLower C}
						  end}
   end

   class StorageClass
      prop
	 locking
      feat
	 DB Online type
	 
      meth init
	 self.DB={Dictionary.new}
      end

      meth toRecord( record: $ )
	 {Dictionary.toRecord self.type self.DB $}
      end

      meth store(id:ID ...)=M
	 lock
	    if {Dictionary.member self.DB ID} then
	       raise idAllreadyInUse(M) end
	    else
	       {Dictionary.put self.DB ID M}
	    end
	 end
      end

      meth get(id:ID entry:$)
	 case {Dictionary.condGet self.DB ID unit} of unit then
	    raise noSuchEntry(ID) end
	 elseof X then X end
      end
      
      meth remove( id:ID )
	 lock
	    case {Dictionary.condGet self.DB ID unit} of unit then
	       raise noSuchEntry(ID) end
	    else {Dictionary.remove self.DB ID} end
	 end
      end
      
      meth update(id:ID ...)=M
	 lock
	    {Dictionary.put self.DB ID {Record.adjoin
					{Record.adjoin
					 {Dictionary.get self.DB ID} M}
					store()}}
	 end
      end

      meth entries($)
	 {Dictionary.entries self.DB} 
      end

      meth items($)
	 {Dictionary.items self.DB}
      end

      meth isMember( id: ID status: S )
	 {Dictionary.member self.DB ID S}
      end

      meth saveToDisk( file: F )
	 lock
	    {Pickle.save {self toRecord(record: $)} {OldestFile F}}
	 end
      end

      meth loadFromDisk( file:File )
	 fun{LoadPickleS Files}
	    fun{LoadValue F}
	       try
		  {Pickle.load {VirtualString.toString F}}
	       catch _ then ~1 end
	    end
	    
	    fun{Load Files A}
	       if Files == nil then A
	       else
		  V F|Fs = Files
	       in
		  V={LoadValue F}
		  if V==~1 then  {Load Fs A}
		  else V end
	       end
	    end
	 in
	    {Load Files nil}
	 end
      in
	 lock
	    R = {LoadPickleS {NewestFiles File}}
	 in
	    if R\=nil then
	       {Record.forAll R proc {$ O}
				   {self O}
				end}
	    else
	       {System.showInfo
		"Warning: Couldn't find database: "#self.type}
	    end
	 end
      end
   end

   class Members from StorageClass
      feat
	 type: members
      meth store(id:ID firstname:FName lastname:LName organization:Org email:Email passwd:Pass userlevel: UL
		 settings: S<=nil extra:Xtra<=nil)
	 StorageClass, store(id:ID firstname: FName lastname:LName organization:Org
			     email:Email passwd:Pass userlevel:UL settings:S extra:Xtra)
      end
   end
   
   class Online from StorageClass
      feat
	 type: onlineStatus
      meth store(id:ID online: S client: C history: H<=nil)
	 StorageClass, store(id:ID online: S client: C  history:H )
      end	      
   end

   class XDBclass from StorageClass
      feat
	 type: xdb
      meth store(id:ID data: D)
	 StorageClass, store(id:ID data:D )
      end	      
   end

   class Friends from StorageClass
      feat
	 type: friends
      meth store(id:ID friends: F)
	 StorageClass, store(id:ID friends: F)
      end	      		    
   end

   class MailBox from StorageClass
      feat
	 type: messages
      meth store(id:MID receiver: R sender: S message:M date:D reply_to: Re)
	 StorageClass, store(id: MID receiver: R sender: S message: M date: D reply_to: Re)
      end
   end
   
   class Notify from StorageClass
      feat
	 type: notify
      meth store(id: ID notify: N)
	 StorageClass, store(id:ID notify: N )
      end
   end
   
   class Applications from StorageClass
      feat
	 type: application
      meth store(id: ID clienturl: C serverurl: S author: A name: N
		 description: D<=nil)
	 StorageClass, store(id:ID clienturl: C serverurl: S author: A
			     name: N description: D)
      end
   end

   class History from StorageClass
      feat
	 type: history
      meth store( friend: F mid: Mid message: M  type: T reply_to: R date:D id: _<=nil)
	 StorageClass, store(id: {VirtualString.toAtom F#Mid} friend:F mid: Mid
			     message: M  type: T reply_to: R date: D)
      end
   end

   MembersDB={New Members init()}
   OnlineDB={New Online init()}
   FriendsDB={New Friends init()}
   NotifyDB={New Notify init()}
   MessageDB={New MailBox init()}
   ApplicationDB={New Applications init()}
   XDB={New XDBclass init()}
   
   class DBase prop locking
      feat
	 membersDB 
	 onlineDB
	 friendsDB
	 notifyDB
	 messageDB
	 applicationDB
	 xDB
      attr
	 path

      meth init()
	 self.membersDB = proc{$ X} {MembersDB X} end
	 self.onlineDB = proc{$ X} {OnlineDB X} end 
	 self.friendsDB = proc{$ X} {FriendsDB X} end
	 self.notifyDB = proc{$ X} {NotifyDB X} end
	 self.messageDB = proc{$ X} {MessageDB X} end
	 self.applicationDB= proc{$ X} {ApplicationDB X} end
	 self.xDB= proc{$ X} {XDB X} end
      end

      meth setDBdir( dir: D )
	 path <- D
      end
     
      meth storeX(id:ID data:D) {self.xDB store(id:ID data:D)} end
      meth updateX(id:ID data:D) {self.xDB update(id:ID data:D)} end
      meth removeX(id:ID) {self.xDB remove(id:ID)} end
      meth entriesX($) {self.xDB entries($)} end
      
      meth toRecord( record: $ )
	 db( membersDB: {self.membersDB toRecord( record:$ )}
	     onlineDB: {self.onlineDB toRecord( record:$)}
	     friendsDB: {self.friendsDB toRecord( record:$)}
	     notifyDB: {self.notifyDB toRecord( record:$)}
	     messageDB: {self.messageDB toRecord( record:$)}
	     id: messageID(id: {Access Counter})
	     appid: applicationID(id: {Access AppCounter})
	     applicationDB: {self.applicationDB toRecord( record:$)})
      end
      
      meth search( id: ID<=nil firstname: FName<=nil lastname: LName<=nil organization: Org<=nil
		   email: Email<=nil hits: $ )
	 {Filter {self.membersDB items($)} fun {$ E}
					      (ID == nil orelse
					       {List.sub {ToLowerString ID}
					       {ToLowerString E.id}})
					      andthen
					      (FName == nil orelse
					       {List.sub {ToLowerString FName}
						{ToLowerString E.firstname}})
					      andthen
					      (LName == nil orelse
					       {List.sub {ToLowerString LName}
						{ToLowerString E.lastname}})
					      andthen
					      (Org == nil orelse
					       {List.sub {ToLowerString Org}
						{ToLowerString E.organization}})
					      andthen
					      (Email == nil orelse
					       {List.sub {ToLowerString Email}
						{ToLowerString E.email}})
					   end $}
      end

      meth updateUser( id: ID firstname: FName lastname: LName organization: Org
		       email: Email passwd: PassWd userlevel: UL extra:Xtra<=nil)
	 {self.membersDB update( id: ID firstname: {List.take FName 25}
				 lastname: {List.take LName 25} organization: Org
				 email: Email passwd: PassWd userlevel:UL
				 extra:Xtra)}
      end

      meth updateSettings( id: ID settings: S )
	 {self.membersDB update( id: ID settings:S)}
      end
      
      meth addUser( id: ID firstname: FName lastname: LName
		    organization: Org friends: Friends extra:Xtra<=nil
		    email: Email passwd: PassWd userlevel: UL)
	 lock
	    if {self.membersDB isMember(id: ID status:$)} then raise idAllreadyInUse(ID) end
	    else
	       {self.membersDB store( id: ID firstname: {List.take FName 25} lastname: {List.take LName 25}
				      organization: Org email: Email
				      passwd: PassWd userlevel:UL
				      extra:Xtra)}
	       {self addFriends( id: ID friends: Friends) }
	    end
	 end
      end

      meth removeUser( id: Id )
	 try
	    {self.friendsDB remove(id: Id)}
	 catch _ then skip end
	 try
	    {self.notifyDB remove(id: Id)}
	 catch _ then skip end
	 try
	    {self.membersDB remove(id: Id)}
	    catch _ then skip end
      end
	 
      
      meth addApplication( id: ID name: N clienturl: C serverurl: S
			   description: D
			   author: A)
	 ID = {GetAppID}
	 {self.applicationDB store( id: ID name: N clienturl: C serverurl: S
				    author: A description: D)}
      end

      meth updateApplication( id: ID name: N clienturl: C serverurl: S
			      description: D author: A)
	 {self.applicationDB update( id: ID name: N clienturl: C
				     serverurl: S author: A description: D)}
      end

      meth getApplicationInfo( id: ID info: $ )
	 if {self.applicationDB isMember( id: ID status:$ )}==false then
	    nil
	 else 
	    {self.applicationDB get( id: ID entry: $ )}
 	 end
      end

      meth getApplication( id: ID  application: $) I in
	 {self getApplicationInfo( id: ID info: I )}
	 if I==nil then raise noSuchApp(ID) end 
	 else url( id: ID name: I.name serverurl: I.serverurl
		   clienturl: I.clienturl author: I.author
		   instance: {GetInstance} description: I.description)
	 end
      end

      meth isSysadm( id: Id sysadm: $ )
	 try
	    if {self get(id:Id entry: $)}.userlevel==sysadm then true
	    else false end
	 catch _ then false end
      end
      
      meth removeApplication( id: ID author: _) % No authority check!
	 if {self.applicationDB isMember( id: ID status:$ )}
	 then {self.applicationDB remove( id: ID )}
	 else raise removeDenied( ID ) end end
      end

      meth getApps(entries:E) 
	 E={Map {self.applicationDB items($)} fun {$ X} app(id:X.id name:X.name author: X.author) end}
      end

      meth getUserInfo( id: ID entry: E ) Entry En in
	 try
	    {self.membersDB get( id: ID entry: En ) }
	    Entry = {Record.adjoin En store( firstname:{List.take {VirtualString.toString En.firstname#""} 25}
					     lastname:{List.take {VirtualString.toString En.lastname#""} 25})}
	    E = {Record.adjoin store( name: Entry.firstname#" "#Entry.lastname
				      friends: Friends online: {self isOnline( id:ID online: $)}) Entry }
	 catch _ then
	    raise noSuchEntry( ID ) end
	 end
      end
       
      meth get( id: ID entry: E ) Entry Friends En in
	 try
	    {self.membersDB get( id: ID entry: En ) }
	    Entry = {Record.adjoin En store( firstname:{List.take {VirtualString.toString En.firstname#""} 25}
					     lastname:{List.take {VirtualString.toString En.lastname#""} 25})}
	    {self getFriends(id: ID friends: Friends ) }
	    E = {Record.adjoin store( name: Entry.firstname#" "#Entry.lastname
				      friends: Friends online: {self isOnline( id:ID online: $)}) Entry }
	 catch _ then
	    raise noSuchEntry( ID ) end
	 end
      end

      meth entries($) 
	 {Map {self.membersDB entries($)} fun {$ X} Friends K#V=X in
					     {self getFriends(id: V.id friends: Friends ) }
					     K#{Record.adjoin store( name: V.firstname#" "#V.lastname
								     friends: Friends
								     online: {self isOnline( id:V.id online: $)})
						V $} 
					  end $ }
      end

      meth items($)
	 {Map {self.membersDB items($)} fun {$ X} Friends in
					   {self getFriends(id: X.id friends: Friends ) }
					   {Record.adjoin store( name: X.firstname#" "#X.lastname
								 friends: Friends
								 online: {self isOnline( id:X.id online: $)})
					    X $} 
					end $ }
      end
      
      meth isOnline( id: ID online: $)
	 if {self.onlineDB isMember( id:ID status: $)} then
	    {self.onlineDB get(id:ID entry: $)}.online
	 else false end 
      end

      meth getAllOnline( entries:$ )
	 {self.onlineDB items($)}
      end
      
      meth getClient( id: ID client: $ )
	 if {self isOnline(id:ID online:$)} \= false then
	    {self.onlineDB get(id:ID entry:$)}.client
	 else
	    raise networkFailure(notOnline ID) end
	 end
      end
	    
      meth setOnline( id:ID online: O client: _<=nil)
	 if {self  isOnline( id: ID online: $ )} == false then
	    raise notLoggedIn( ID ) end 
	 else
	    {self.onlineDB update(id: ID online: O )}
	 end
      end
      
      meth online( id:ID client: C )
	 {self setOnline(id: ID online: online  )}
      end

      meth offline( id: ID client: C)
	 {self setOnline( id:ID online: offline)}
      end

      meth login( id: ID client:C )
	 if {self  isOnline( id: ID online: $ )} \= false then
	    raise allreadyLoggedIn( ID ) end 
	 else H in
	    H = {New History init}
	    try 
	       {H loadFromDisk( file: @path#'mess_'#ID)}
	    catch X then {System.printError "Couldn't load history for "#ID#" reason: "#X} end
	    {self.onlineDB store(id: ID online: online client: C history:H )}
	 end
      end
      
      meth logout( id: ID ) H in
	 H = {self.onlineDB get(id: ID entry: $)}.history 
	 % Spara history databasen här!
	 thread
	    {H saveToDisk( file: @path#'mess_'#ID )}
	 end
	 {self.onlineDB remove( id:ID )}
      end

      meth addFriends( id:ID friends: Friends )
	 if {self.friendsDB isMember( id:ID status: $)} then
	    lock
	       {self.friendsDB update( id:ID
				       friends: {NoDoubles {List.append {self getFriends(id:ID friends: $)}
						 Friends}})}
	    end
	 else
	    {self.friendsDB store( id:ID friends: Friends)}
	 end
	 {ForAll Friends proc {$ X} {self setNotify( id:X notify: [ID])} end}
      end

      meth removeFriend( id: ID friend: F )
	 lock
	    if {self.friendsDB isMember( id:ID status: $)} then
	       {self.friendsDB update( id:ID
				       friends: {List.subtract {self getFriends(id:ID friends: $)} F})}
	    else
	       {self.friendsDB store( id:ID friends: nil)}
	    end
	    {self removeNotify( id:F notify: ID)}
	 end
      end
            
      meth getFriends( id: ID friends: $) 
	 {self.friendsDB get( id:ID entry: $)}.friends	
      end

      meth getFriendsStatus( id: ID online: On offline: Off ) F in
	 {List.map {self getFriends( id:ID friends: $ )} fun {$ I} E in
							    {self get(id: I entry: E)}
							    friends( id: I name: E.name
								     online: {self isOnline(id:I
											    online:$)})
							 end F}
	 On= {Filter F fun{$ X} T=X.online in
			   T == online orelse T == away
			end}
	 Off= {Filter F fun{$ X} T=X.online in
			  T == offline orelse T == false
		       end}
      end

      meth getHistory( id: ID history:History )
	 fun{MakeHistoryList SortedList}
	    fun{MakeRecordR Ls Acc}
	       if Ls == nil then Acc
	       else
		  L|Lx = Ls Old Sent F=L.friend
		  R = {CondSelect Acc L.friend m(old: nil sent: nil)}
	       in

		  if L.type == sent then
		     Old = R.old
		     Sent = {List.append [message(mid: L.mid date: L.date
						  message: L.message
						  reply_to: L.reply_to)] R.sent }
		  else
		     Sent = R.sent
		     Old = {List.append [receiveMessage(mid: L.mid date: L.date
							      message: L.message reply_to: L.reply_to
							sender: L.friend)] R.old }
		  end

		  {MakeRecordR Lx {Record.adjoinAt Acc F
				   m(old: Old sent: Sent)}}
	       end
	    end
	    Tmp = {NewCell nil}
	 in
	    
	    {Record.forAllInd {MakeRecordR SortedList msg()}
	     proc{$ I X}
		{Assign Tmp {Append 
			     [messages(id: I
				       old: X.old
				       sent: X.sent)] {Access Tmp}}}
	     end}
	    {Access Tmp}
	 end
	 
	 H = {self.onlineDB get(id: ID entry: $)}.history
	 E = {H items($)} 
      in
	 History = {MakeHistoryList E}
      end

      meth clearHistory( id: ID friend: F )
	 H = {self.onlineDB get(id: ID entry: $)}.history
	 E = {H items($)}
      in
	 {ForAll E proc{$ X}
		      if X.friend == F then
			 {H remove(id: X.id )}
		      end
		   end}
      end
      
      meth setNotify( id:ID notify: Notify )
	 if {self.notifyDB isMember( id:ID status: $)} then
	    lock
	       {self.notifyDB update( id:ID notify: {List.append {self getNotify(id: ID notify: $)}
						     Notify})}
	    end
	 else
	    {self.notifyDB store( id:ID notify: Notify)}
	 end
      end

      meth removeNotify( id: ID notify: N )
	 if {self.notifyDB isMember( id:ID status: $)} then
	    lock
	       {self.notifyDB update( id:ID
				      notify: {List.subtract {self getNotify(id:ID notify: $)} N})}
	    end
	 else
	    {self.notifyDB store( id:ID notify: nil)}
	 end
      end
	    
      meth getNotify( id: ID notify: $)
	 if {self.notifyDB isMember(id:ID status:$)} then	
	    {self.notifyDB get( id:ID entry: $)}.notify	
	 else
	    nil
	 end
      end

      meth getNotifyStatus( id: ID online: On offline: Off ) F in
	 lock
	    {List.map {self getNotify( id:ID notify: $ )} fun {$ I} E in
							     {self get(id: I entry: E)}
							     notify( id: I name: E.name)
							  end F}
	    Off= {Filter F fun{$ X} {self isOnline(id:X.id online:$)}==false end}
	    On= {Filter F fun{$ X} {self isOnline(id:X.id online:$)}\=false end}
	 end
      end

      meth storeMessage(id: MID receiver: R sender: S message: M date: D reply_to: Re)
	 {self.messageDB store( id: MID receiver: R sender: S message: M date:D reply_to: Re)}
	 thread H = {self.onlineDB get(id: S entry: $)}.history in
	    {ForAll R proc{$ O}
			 {H store( friend: O message:M mid: MID date: D reply_to: Re type: sent)}
		      end}
	 end
      end

      meth messageAck( id: Id mid: Mid read: $)
	 lock MsgLock then R in
	    try M = {self.messageDB get(id: Mid entry:$)}
	        H = {self.onlineDB get(id: Id entry: $)}.history 
	    in
	       R = {List.subtract M.receiver Id}
	       {self.messageDB update( id: Mid receiver: R )}
	       {H store( friend: M.sender type: received date: M.date reply_to: M.reply_to
			 mid: Mid message: M.message)}
	       if R==nil then true else false end
	    catch _ then false end
	 end
      end
      
      meth removeMessage(mid: MID )
	 {self.messageDB remove( id: MID )}
      end

      meth removeMessagesFrom(id: Id )
	 {ForAll {Filter {self.messageDB items($)} fun{$ X}
						      X.sender == Id
						   end} proc {$ Y}
							   {self removeMessage( mid: Y.id)}
							end}
      end


      meth getSender( mid: Id sender: S) Msg in
	 try
	    {self.messageDB get(id:Id entry: Msg)}
	 catch _ then raise noMessage( Id ) end end
	 S = Msg.sender
      end
 
      
      meth getMessages( id: ID messages: M )
	 M = {Sort {Filter {self.messageDB items($)} fun{$ X}
							{List.member ID X.receiver}
						     end $} fun{$ X Y}
							       X.id < Y.id
							    end}
      end

      meth saveAll( dir: _) % The directory is set earlier, just here for backward comp.
	 lock
	    {self makeBackup}
	    {self.membersDB saveToDisk( file: @path#members) }
	    {self.friendsDB saveToDisk( file: @path#friends) }
	    {self.messageDB saveToDisk( file: @path#message) }
	    {self.applicationDB saveToDisk( file: @path#application)}
	    {self.xDB saveToDisk( file: @path#xdb)}
	    {SaveMessID @path#id }
	    {SaveAppID @path#appid }
	    
	    {ForAll {self.onlineDB items($)} proc{$ X} H = X.history in
						{H saveToDisk( file: @path#"mess_"#X.id )}
					     end}
	 end
      end

      meth makeBackup()
	 skip
      end
      
      meth buildNotifyTable() F = {self.friendsDB entries($)} in
	 {ForAll F proc{$ X} K#V=X in
		      {self addFriends( id: K friends: nil)}
		      {ForAll V.friends proc {$ I}
					   {self setNotify( id: I notify: [V.id] )}
					end} end}
	 
      end

      meth getNoOfUsers( online: O total: T )
	 O = {List.length {self getAllOnline( entries: $)}}
	 T = {List.length {self.membersDB entries($)}}
      end
      
      meth loadAll( dir: _) % The directory is set earlier, just here for backward comp.
	 lock
	    {self.membersDB loadFromDisk( file: @path#members ) } 
	    {self.friendsDB loadFromDisk( file: @path#friends ) }
	    {self.messageDB loadFromDisk( file: @path#message ) }
	    {self.applicationDB loadFromDisk( file: @path#application)}
	    {self.xDB loadFromDisk( file: @path#xdb)}
	    {LoadMessID @path#id}
	    {LoadAppID @path#appid}
	    {self buildNotifyTable}
	 end
      end
   end
in
   DB = {New DBase init}
   InvokeDB = proc{$ X} {DB X} end
end


