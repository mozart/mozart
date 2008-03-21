functor
import
   System
   Glue(setSiteState
	setConnection
	acceptConnection
	getThisSite
	initDP
	setRPC
	setAnnotation
	kill
	break
	getFaultStream) at 'x-oz://boot/Glue'
   Property
   DPDefaults(listen:DefaultListenFunc init)
   Error
   DPErrorFormatters
   DPService
export
   Init
   Prepare
   Initialized
   service:DPService
   Kill
   Break
   GetFaultStream
   Annotate
define
   {Error.registerFormatter dp DPErrorFormatters.dp}
   {DPDefaults.init}
   ListenFuncC={NewCell DefaultListenFunc}
   InitializedC={NewCell false}
   fun{Initialized}
      @InitializedC
   end
   proc{Prepare ListenFunc}
      ListenFuncC:=ListenFunc
   end
   proc{Init}
      N in
      if {Not InitializedC:=N} then
	 InStream
	 DistributedURIs={@ListenFuncC {NewPort InStream}}
	 Info={ExtractDistributedInfo DistributedURIs}
      in
	 {Glue.setRPC proc {$ P Args Ret}
			 try
			    {Procedure.apply P Args}
			    Ret=unit
			 catch E then
			    Ret={Value.failed E}
			 end
		      end}
	 {Glue.initDP {NewPort thread {ForAll $ ProcessDSS} end}}
	 {Glue.getThisSite}.info := Info
	 thread
	    {ForAll InStream DoAccept}
	 end
      end
      N=true
   end
   fun{ExtractDistributedInfo DistributedURIs}
      case DistributedURIs
      of [VH] andthen H={VirtualString.toString VH} in {List.isPrefix "oz-site://s(" H} then
	 H
      [] URIs then
	 {Exception.raiseError dp(dssLimit distributedURI URIs)}
	 unit
      end
   end
   proc{ProcessDSS M}
      case M
      of connect(ToSite) then
	 {System.show connect}
	 thread
	    URI={VirtualString.toString ToSite.info}
	    ConnectMeths={{Property.get 'dp.resolver'}.'oz-site' URI}.connect
	 in
	    if {Not {DoConnect ToSite ConnectMeths}} then
	       {Exception.raiseError dp(connection noLuck ToSite [URI])}
	    end
	 end
      [] connection_received(_/*ToSite*/ _/*FD*/) then
	 skip
      [] new_site(S) then
	 {DPService.incoming S service(to:'oz:newSite' msg:hello(S))}
      [] deliver(src:S msg:M)then
	 try
	    case M
	    of service(...) then
	       {DPService.incoming S M}
	    else
	       {DPService.incoming S service(to:'oz:siteMessage'  msg:M)}
	    end
	 catch E then
	    {PrintError E}
	 end
      [] M then
	 {PrintError {Exception.error dp(dss unknownNotification M)}}
      end
   end
   NextConnect={NewName}
   fun{DoConnect ToSite ConnS}
      case ConnS
      of nil then
	 false
      [] Conn|ConnT then
	 try
	    case Conn
	    of fd(FD) then
	       {Glue.setConnection ToSite fd(FD FD)}
	       true
	    [] fd(FD0 FD1) then
	       {Glue.setConnection ToSite Conn}
	       true
	    [] sock(Sock) then FD0 FD1 in
	       {Sock getDesc(FD0 FD1)}
	       {Glue.setConnection ToSite fd(FD0 FD1)}
	       true
	    [] none then
	       true
	    [] ignore then
	       raise NextConnect end
	    [] permFail then
	       {Glue.setSiteState ToSite permFail}
	       true
	    [] P andthen {IsProcedure P} then
	       {DoConnect ToSite {P}}orelse raise NextConnect end
	    end
	 catch !NextConnect then
	    {DoConnect ToSite ConnT}
	 [] E then
	    {PrintError E}
	    {DoConnect ToSite ConnT}
	 end
      end
   end
   proc{DoAccept X}
      try
	 case X
	 of fd(FD) then
	    {Glue.acceptConnection fd(FD FD)}
	 [] fd(FD0 FD1) then
	    {Glue.acceptConnection X}
	 [] sock(Sock) then FD0 FD1 in
	    {Sock getDesc(FD0 FD1)}
	    {Glue.acceptConnection fd(FD0 FD1)}
	 end
      catch E then
	 {PrintError E}
      end
   end
   PrintError=Error.printException
   
   local
      %% each annotation parameter is a record, possibly with no field
      fun {Access access(S)}
	 case S
	 of stationary then _#1#_#_#_     % AA_STATIONARY_MANAGER
	 [] migratory  then _#2#_#_#_     % AA_MIGRATORY_MANAGER
	 end
      end
      fun {Stationary stationary}  1#_#_#_#_ end     % PN_SIMPLE_CHANNEL
      fun {Migratory  migratory}   2#_#_#_#_ end     % PN_MIGRATORY_STATE
      fun {Pilgrim    pilgrim}     9#_#_#_#_ end     % PN_PILGRIM_STATE
      fun {Replicated replicated}  5#_#_#_#_ end     % PN_EAGER_INVALID
      fun {Variable   variable}    3#_#_#_#_ end     % PN_TRANSIENT
      fun {Reply      reply}       4#_#_#_#_ end     % PN_TRANSIENT_REMOTE
      fun {Immediate  immediate}  13#_#_#_#_ end     % PN_IMMEDIATE
      fun {Eager      eager}      12#_#_#_#_ end     % PN_IMMUTABLE_EAGER
      fun {Lazy       lazy}       11#_#_#_#_ end     % PN_IMMUTABLE_LAZY
      fun {Persistent persistent}  _#_#1#0#0 end     % RC_ALG_PERSIST
      fun {Refcount   refcount}    _#_#0#2#_ end     % RC_ALG_WRC
      fun {Lease      lease}       _#_#0#_#4 end     % RC_ALG_TL
      %% map each annotation label to a parameter constraint
      Constrain =
      constr(access:     Access
	     stationary: Stationary
	     migratory:  Migratory
	     pilgrim:    Pilgrim
	     replicated: Replicated
	     variable:   Variable
	     reply:      Reply
	     immediate:  Immediate
	     eager:      Eager
	     lazy:       Lazy
	     persistent: Persistent
	     refcount:   Refcount
	     lease:      Lease
	    )
   in
      %% annotate an entity
      proc {Annotate E Annot}
	 As = if {List.is Annot} then Annot else [Annot] end
	 PN AA RC0 RC1 RC2
	 Param = PN#AA#RC0#RC1#RC2
      in
	 try
	    %% constrain Param with the elements of As, and complete with
	    %% zeroes (= PN_NO_PROTOCOL, AA_NO_ARCHITECTURE, RC_ALG_NONE)
	    for A in As do {Constrain.{Label A} A Param} end
	    {Record.forAll Param proc {$ X} if {IsFree X} then X=0 end end}
	 catch _ then
	    raise dp('annotation format error' Annot) end
	 end
	 %% now set annotation with the parameters
	 {Init}
	 {Glue.setAnnotation E PN AA RC0+RC1+RC2}
      end
   end
   proc{Kill X}
      {Init}
      {Glue.kill X}
   end
   proc{Break X}
      {Init}
      {Glue.break X}
   end
   fun{GetFaultStream X}
      {Init}
      {Glue.getFaultStream X}
   end
end