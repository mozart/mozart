%%%
%%% Authors:
%%%   Yves Jaradin (yves.jaradin@uclouvain.be)
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
   class OzSimpleProto
      from Open.socket
      attr
	 id
	 closeLock
      meth init(U)
	 [Ip PortS Id]={String.tokens
			{List.takeWhile
			 {List.drop U {Length "oz-site://"}}
			 fun{$C}C\=&/ end}
			&:}in
	 Open.socket,init()
	 id:=Id
	 {self connect(host:Ip port:{String.toInt PortS})}
	 closeLock:={NewLock}
	 local
	    FD
	    L=@closeLock
	 in
	    {self getDesc(FD FD)}
	    {Finalize.register self proc{$ _}
				       {Wait 3000} %Should be configurable
				       lock L then
					  {OS.close FD}
				       end
				    end}
	 end
      end
      meth dOpen(X Y)
	 Open.socket,dOpen(X Y)
	 closeLock:={NewLock}
	 local
	    FD
	    L=@closeLock
	 in
	    {self getDesc(FD FD)}
	    {Finalize.register self proc{$ _}
				       {Wait 3000} %Should be configurable
				       lock L then
					  {OS.close FD}
				       end
				    end}
	 end
      end
      meth unHook()
	 R in
	 thread lock @closeLock then R=unit {Wait _} end end
	 {Wait R}
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
   proc{Init}
      Resolvers={NewDictionary}
   in
      Resolvers.'oz-site':=
      fun{$ Uri}
	 OzS
      in
	 try
	    Answer
	 in
	    OzS={New OzSimpleProto init(Uri)}
	    {OzS sendMsg(["get" Uri])}
	    Answer={OzS receiveMsg($)}
	    case Answer
	    of ["ok" !Uri PSite Meths] then
	       s(site:{Pickle.unpack {Decode PSite}}
		 connect:
		    {Map Meths
		     fun{$ M}
			case M
			of ["uri" Uri] then
			   fun{$}
			      Scheme={String.toAtom {List.takeWhile Uri fun{$C}C\=&:end}}
			   in
			      {{Property.get 'dp.resolver'}.Scheme Uri}.connect
			   end
			[] ["direct"] then
			   fun{$}
			      {OzS sendMsg(["connect"])}
			      if {OzS receiveMsgForce("accept" $)} then
				 {OzS unHook()}
				 [sock(OzS)]
			      else
				 [ignore]
			      end
			   end
			[] ["reverse"] then
			   if{Not {Property.get 'dp.firewalled'}} then
			      fun{$}
				 {OzS sendMsg(["reverseConnect" {Site.allURIs {Site.this}}])}
				 [none]
			      end
			   else
			      ignore
			   end
			[] M then
			   {Exception.raiseError dp(line unknownConnMeth M)}
			   ignore
			end
		     end})
	    [] ["dead" !Uri PSite] then
	       s(site:{Pickle.unpack {Decode PSite}}
		 connect:[permFail])
	    else
	       s(site:{Value.failed siteNotReachable}
		 connect:nil)
	    end
	 catch E=system(os(os "connect" ...) ...) then
	    if {OzS getId($)}==0 then
	       s(site:{Value.failed siteIsDead}
		 connect:[permFail])	       
	    else
	       raise E end
	    end
	 end
      end
      {Property.put 'dp.firewalled' false}
      {Property.put 'dp.resolver' Resolvers}
      {Property.put 'dp.listenerParams'
       default(id: ({OS.time} mod 257)*65536+{OS.getPID} )}
   end
   fun{Listen IncomingP}
      Ip Port Id Uri Serv Params
   in
      Params={Property.get 'dp.listenerParams'}
      Serv={New Open.socket init()}
      Ip={DoGetIp {Value.condSelect Params ip best}}
      Port={DoBind Serv {Value.condSelect Params port 'from'(9000)}}
      Id={Value.condSelect Params id 0}
      Uri={VirtualString.toString 'oz-site://'#Ip#':'#Port#':'#Id}
      {Serv listen()}
      thread
	 for OzS from fun{$}{Serv accept(accepted:$ acceptClass:OzSimpleProto)}end do
	    thread
	       case {OzS receiveMsg($)}
	       of ["get" !Uri] then
		  Meths=[["direct"] ["reverse"]] in
		  {OzS sendMsg(["ok" Uri {Encode{ByteString.toString {Pickle.pack {Site.this}}}} Meths])}
		  case
		     try{OzS receiveMsg($)}
		     catch error(dp(line dropped)...) then dropped
		     end
		  of ["connect"] then
		     {OzS sendMsg("accept")}
		     {OzS unHook()}
		     {Send IncomingP sock(OzS)}
		  [] ["reverseConnect" Uris] then
		     for U in Uris continue:C break:B do
			try
			   S={Site.resolve U}in
			   {Wait S}
			   {Site.allURIs S _}%Force connect
			catch _ then
			   {C}
			end
			{B}
		     end
		  [] dropped then
		     skip
		  end
	       end
	    end
	 end
      end
      [Uri]
   end
   fun{DoBind S D}
      case D
      of 'from'(X) then
	 try
	    {S bind(takePort:X)}
	    X
	 catch _ then
	    {DoBind S 'from'(X+1)}
	 end
      [] free then
	 {S bind(port:$)}
      [] exact(X) then
	 {S bind(takePort:X)}
	 X
      end
   end
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
   fun{BestIp IPs}
      {Sort IPs fun{$ X Y}
		   EX={EvalIp X}
		   EY={EvalIp Y}
		in
		   (EX=='global' andthen EY\='global')
		   orelse
		   (EX=='private' andthen EY\='global' andthen EY\='private')
		   orelse
		   (EX=='local' andthen EY\='global' andthen EY\='private' andthen EY\='local')
		   orelse
		   (EX=='loopback' andthen EY=='reserved')
		end}.1
   end
   fun{EvalIp IP} %RFC3330
      case IP
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
      end
   end
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