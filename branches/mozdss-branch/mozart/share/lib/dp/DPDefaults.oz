%%%
%%% Authors:
%%%   Yves Jaradin (yves.jaradin@uclouvain.be)
%%%
%%% Contributors:
%%%   Raphael Collet (raphael.collet@uclouvain.be)
%%%
%%% Copyright:
%%%   Yves Jaradin, 2008
%%%
%%% Last change:
%%%   $Date: 2008-03-06 13:33:44 +0100 (Thu, 06 Mar 2008) $ by $Author: yjaradin $
%%%   $Revision: 16860 $
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
   Finalize
   Open
   OS
   Property
   Pickle
   Site

export
   Init
   Listen

define
   %% serialize/deserialize messages in Channel
   fun{Ser M Tail}
      case M
      of nil then &<|&>|Tail
      [] _|_ then &<|{FoldL M fun{$ Acc X}{Ser X $ Acc}end $ &>|Tail}
      [] &< then {Exception.raiseError dp(line badChar &<)} unit
      [] &> then {Exception.raiseError dp(line badChar &>)} unit
      else
	 M|Tail
      end
   end
   proc{Deser T S}
      case T
      of nil then
	 S.1=nil
	 S.2=nil
      [] &<|T then X Y in
	 S.1=X|Y
	 {Deser T X|Y|S.2}
      [] &>|T then
	 S.1=nil
	 {Deser T S.2}
      [] H|T then X in
	 S.1=H|X
	 {Deser T X|S.2}
      end
   end

   %% A Channel wraps a socket connection to communicate with a site
   class Channel from Open.socket
      attr
	 id
	 closeable: true     % set to false by unHook()

      meth init(U)
	 Ip#PortNum#Id={DecomposeURI U}
      in
	 Open.socket,init()
	 id:=Id
	 {self connect(host:Ip port:{String.toInt PortNum})}
	 {Finalize.register self proc {$ C} {C close} end}
      end
      meth dOpen(X Y)
	 Open.socket,dOpen(X Y)
	 {Finalize.register self proc {$ C} {C close} end}
      end
      meth close(...)=M
	 if @closeable then Open.socket,M end
      end
      meth unHook()
	 closeable:=false
      end

      meth getId($)
	 @id
      end

      meth sendMsg(M)
	 {self send(vs:{Ser M nil})}
      end
      meth receiveMsg($)
	 {self Receive("" $)}
      end
      meth Receive(I $)
	 L
	 SerF={Append I Open.socket,read(list:$ len:L)}
      in
	 try
	    {Deser SerF [$]|nil}
	 catch _ then
	    if L>0 then
	       {self Receive(SerF $)}
	    else
	       {Exception.raiseError dp(line dropped)} unit
	    end
	 end
      end
      meth ReceiveN(N $)
	 L T SerF=Open.socket,read(list:$ tail:T size:N len:L)
      in
	 if N<L then
	    T={self ReceiveN(N-L $)}
	 else
	    T=nil
	 end
	 SerF
      end
      meth receiveMsgForce(M $)
	 SerW={Ser M nil}
	 LL={Length SerW}
	 SerF={self ReceiveN(LL $)}
      in
	 SerF==SerW
      end
   end

   %% define default settings for DP module
   proc{Init}
      Resolvers={NewDictionary}
   in
      Resolvers.'oz-site':=GetConnectMeths
      {Property.put 'dp.firewalled' false}
      {Property.put 'dp.resolver' Resolvers}
      {Property.put 'dp.listenerParams'
       default(id: 'h'#(({OS.time} mod 257)*65536+{OS.getPID}) )}
   end

   %% connect to a site URI, and return the available connection
   %% methods to that site
   fun {GetConnectMeths Uri}
      Chan
   in
      try
	 Chan={New Channel init(Uri)}
	 {Chan sendMsg(["get" Uri])}     % send request: <get Uri>
	 case {Chan receiveMsg($)}
	 of ["ok" !Uri PSite Meths] then     % successful reply
	    s(site:{Pickle.unpack {Decode PSite}}
	      connect:{Map Meths
		       fun{$ M}
			  case M
			  of ["uri" Uri] then
			     {ConnectToUri Uri}
			  [] ["direct"] then
			     {ConnectDirect Chan}
			  [] ["reverse"] then
			     {ConnectReverse Chan}
			  [] M then
			     {Exception.raiseError dp(line unknownConnMeth M)}
			     ignore
			  end
		       end})
	 [] ["dead" !Uri PSite] then     % site is dead, cannot connect
	    s(site:{Pickle.unpack {Decode PSite}}
	      connect:[permFail])
	 else
	    s(site:{Value.failed siteNotReachable}
	      connect:nil)
	 end
      catch system(os(os "connect" ...) ...) andthen {IsCritical Uri} then
	 s(site:{Value.failed siteIsDead}
	   connect:[permFail])	       
      [] _ then
	 s(site:{Value.failed siteNotReachable}
	   connect:nil)
      end
   end
   fun {ConnectToUri Uri}
      fun {$}
	 Scheme={String.toAtom {String.token Uri &: $ _}}
      in
	 {{Property.get 'dp.resolver'}.Scheme Uri}.connect
      end
   end
   fun {ConnectDirect Chan}
      fun {$}
	 {Chan sendMsg(["connect"])}     % send request: <connect>
	 if {Chan receiveMsgForce("accept" $)} then
	    %% accepted: the channel can be used as a connection
	    {Chan unHook()}
	    [sock(Chan)]
	 else
	    [ignore]
	 end
      end
   end
   fun {ConnectReverse Chan}
      if {Property.get 'dp.firewalled'} then ignore else
	 fun {$}
	    {Chan sendMsg(["reverseConnect" {Site.allURIs {Site.this}}])}
	    [none]
	 end
      end
   end

   %% launch site connection server on the current site
   fun {Listen IncomingP}
      Server={New Open.socket init()}
      Params={Property.get 'dp.listenerParams'}
      IP={DoGetIp {Value.condSelect Params ip best}}
      PN={DoBind Server {Value.condSelect Params port 'from'(9000)}}
      ID={Value.condSelect Params id 0}
      Uri={ComposeURI IP PN ID}
      proc {Serve Chan}
	 thread
	    try
	       case {Chan receiveMsg($)}
	       of ["get" !Uri] then     % receive request <get Uri>
		  PSite={Encode {ByteString.toString {Pickle.pack {Site.this}}}}
		  Meths=[["direct"] ["reverse"]]
	       in
		  {Chan sendMsg(["ok" Uri PSite Meths])}     % reply with data
		  case {Chan receiveMsg($)}
		  of ["connect"] then
		     {Chan sendMsg("accept")}
		     {Chan unHook()}
		     {Send IncomingP sock(Chan)}
		  [] ["reverseConnect" Uris] then
		     for U in Uris   break:Break do
			try S={Site.resolve U} in
			   {Wait S}
			   {Site.allURIs S _}     % Force connect
			   {Break}
			catch _ then
			   skip          % try next Uri
			end
		     end
		  end
	       end
	    catch _ then
	       skip     % in case of an error, we simply drop the channel
	    end
	 end
      end
   in
      {Server listen()}
      thread
	 %% infinite loop; create a Channel for every incoming connection
	 for do {Serve {Server accept(accepted:$ acceptClass:Channel)}} end
      end
      [Uri]
   end

   %% bind Socket S to a port, following the specification D
   fun{DoBind S D}
      case D
      of 'from'(X) then
	 try
	    {S bind(takePort:X)} X
	 catch _ then
	    {DoBind S 'from'(X+1)}
	 end
      [] free then
	 {S bind(port:$)}
      [] exact(X) then
	 {S bind(takePort:X)} X
      end
   end

   %% extract an IP address, following specification D
   fun{DoGetIp D}
      case D
      of exact(Ip) then
	 Ip
      [] dns(N) then
	 {OS.getHostByName N}.addrList.1
      [] best then
	 {BestIp {OS.getHostByName {OS.uName}.nodename}.addrList}
      end
   end

   %% return the best IP address to connect in the list
   fun {BestIp IPs}
      X|T={Map IPs CategorizeIP}
   in
      {FoldL T Best2 X}.2
   end
   fun {Best2 (C1#_)=CIP1 (C2#_)=CIP2}
      if (C2=='global' andthen C1\='global') orelse
	 (C2=='private' andthen C1\='global' andthen C1\='private') orelse
	 (C2=='local' andthen C1\='global' andthen
	  C1\='private' andthen C1\='local') orelse
	 (C2=='loopback' andthen C1=='reserved')
      then CIP2 else CIP1 end
   end

   %% adjoin a category to the given IP address, following RFC3330
   fun {CategorizeIP IP}
      (case IP
       of &1|&0|&.|_ then 'private'
       [] &1|&2|&7|&.|_ then 'loopback'
       [] &1|&6|&9|&.|&2|&5|&4|_ then 'local'
       [] &1|&7|&2|&.|T then
	  case T
	  of &1|&6|&.|_ then 'private'
	  [] &1|&7|&.|_ then 'private'
	  [] &1|&8|&.|_ then 'private'
	  [] &1|&9|&.|_ then 'private'
	  [] &2|_|&.|_ then 'private'
	  [] &3|&0|&.|_ then 'private'
	  [] &3|&1|&.|_ then 'private'
	  end
       [] &1|&9|&2|&.|&1|&6|&8|&.|_ then 'private'
       [] &2|&2|&4|&.|_ then 'multicast'
       [] &2|&2|&5|&.|_ then 'multicast'
       [] &2|&2|&6|&.|_ then 'multicast'
       [] &2|&2|&7|&.|_ then 'multicast'
       [] &2|&2|&8|&.|_ then 'multicast'
       [] &2|&2|&9|&.|_ then 'multicast'
       [] &2|&3|_|&.|_ then 'multicast'
       [] &2|&4|_|&.|_ then 'reserved'
       [] &2|&5|_|&.|_ then 'reserved'
       else
	  'global'
       end)#IP
   end

   %% compose/decompose a site URI
   fun {ComposeURI IP PN ID}
      {VirtualString.toString 'oz-site://'#IP#':'#PN#'/'#ID}
   end
   fun {DecomposeURI URI}
      IP PN ID in
      {String.token {String.token {List.drop URI PrefixLen} &: IP} &/ PN ID}
      IP#PN#ID
   end
   PrefixLen={Length "oz-site://"}

   %% tell whether the Uri is critical to connect to the site
   fun {IsCritical URI}
      %% it is the case when the site id starts with letter 'h'
      case {DecomposeURI URI}.3 of &h|_ then true else false end
   end

   %% encode/decode a string of bytes
   fun{Encode Xs}
      case Xs
      of nil then nil
      [] H|T then &A+(H div 16)|&a+(H mod 16)|{Encode T}
      end
   end
   fun{Decode Xs}
      case Xs
      of nil then nil
      [] A|B|T then (A-&A)*16+(B-&a)|{Decode T}
      end
   end
end
