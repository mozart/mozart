%\define DEBUG

functor

import
   Open
   OS
   Property
   QUI
   QHTMLDevel(checkType:CheckType
	      defineWidget:DefineWidget
	      undefined:Undefined
	      onConnect:OnConnect
	      onDisconnect:OnDisconnect
	      baseClass:BaseClass
	      set:Set
	      get:Get
	      quoteDouble:QuoteDouble
	      quote:SQuote
	      buildHTML:BuildHTML
	      getId:GetId
	      buildInnerHTML:BuildInnerHTML
	      processEvent:ProcessEvent
	      execEvent:ExecEvent
	      dataVal:DataVal
%	      quote:Quote
	      reSetCustom:ReSetCustom
	      dataInfo:DataInfo
	      join:Join
	      qInit:QInit)
   QHTMLType(translate2HTML:Translate2HTML
	     record2str:Record2str
	     translate2OZ:Translate2OZ)
   QHTMLWebServer
   ConnectionServer
   
define

   GetBufferSize=100
%   proc{Print M} try {Show {VirtualString.toAtom M}} catch _ then skip end end

   %% This part of the code that opens a web browser is taken
   %% from Nils Franzen.

   NoArgs={NewName}
   IsWin32=({Property.get 'platform.os'}==win32)

   proc{DisplayUrl URL}
      try
	 if IsWin32 then
	    %% rundll32 url.dll,FileProtocolHandler http://www.mozart-oz.org
%	    _={OS.exec "rundll32" ["url.dll,FileProtocolHandler" URL] true}
	    _={OS.exec "explorer" [URL] true}
	 else
	    %% netscape -remote openURL(http://www.mozart-oz.org)
	    ExitCode={OS.system "netscape -raise -remote 'openURL("#URL#")'"}
	 in
	    if ExitCode\=0 then
	       %% Command failed, start up the browser with: netscape http://www.mozart-oz.org
	       _={OS.exec "netscape" [URL] true}
	    end
	 end
      catch EE then
	 {OS.wait _ _}
	 raise browserError(URL EE) end
      end
   end

   %% end of Nils part... thanks..

   fun{Record2HTML R}
      {Label R}#"("#
      {Join {List.map {Record.toList R}
	     fun{$ E}
		'"'#{QuoteDouble {VirtualString.toString E}}#'"'
	     end}
       ", "}#
      ")"
   end

   fun{Quote R1}
      R={VirtualString.toString R1}
   in
      case R of &"|_ then
	 Str={List.drop {List.take R {Length R}-1} 1}
      in
	 '"'#{QuoteDouble Str}#'"'
      else
	 R
      end
   end
   
   fun{SplitV Str R}
      A B
   in
      {List.takeDropWhile Str
       fun{$ C} C\=&, end A B}
      R=if B==nil then nil else {List.drop B 1} end
      {String.toAtom A}
   end

   fun{GetRandomStr}
      Min Max
      {OS.randLimits Min Max}
      {OS.srand 0}
   in
      {List.map
       {List.make 32}
       fun{$ C}
	  C=(({OS.rand}-Min)*26 div Max)+&A
       end}
   end
   
   {DefineWidget
    r(desc:toplevel(toplevel
		    container)
      tag:body
      params:[leftMargin#r(type:int
			   default:0
			   init:outer(false)
			   set:outer(false)
			   get:outer(false))
	      rightMargin#r(type:int
			    default:0
			    init:outer(false)
			    set:outer(false)
			    get:outer(false))
	      bottomMargin#r(type:int
			     default:0
			     init:outer(false)
			     set:outer(false)
			     get:outer(false))
	      topMargin#r(type:int
			  default:0
			  init:outer(false)
			  set:outer(false)
			  get:outer(false))
	      scriptId#r(type:vs
			 default:""
			 init:custom(false)
			 set:custom(false)
			 get:internal(true))
	      isconnected#r(type:bool
			    init:custom(false)
			    set:custom(false)
			    get:custom(true))
	      html#r(type:vs
		     init:custom(false)
		     set:custom(false)
		     get:custom(true))
	      activeElement#r(type:no
			      init:custom(false)
			      set:custom(false)
			      get:custom(true))
	      alinkColor#r(type:color
			   init:custom(true)
			   set:custom(true)
			   get:custom(true))
	      bgColor#r(type:color
			init:custom(true)
			set:custom(true)
			get:custom(true))
	      charset#r(type:vs
			init:custom(true)
			set:custom(true)
			get:custom(true))
%	      cookie#r(type:vs
%		       init:custom(true)
%		       set:custom(true)
%		       get:custom(true))
	      defaultCharset#r(type:vs
			       init:custom(true)
			       set:custom(true)
			       get:custom(true))
	      domain#r(type:vs
		       init:custom(true)
		       set:custom(true)
		       get:custom(true))
	      fgColor#r(type:color
			init:custom(true)
			set:custom(true)
			get:custom(true))
	      linkColor#r(type:color
			  init:custom(true)
			  set:custom(true)
			  get:custom(true))
	      readyState#r(type:atom
			   init:custom(false)
			   set:custom(false)
			   get:custom(true))
	      referrer#r(type:vs
			 init:custom(false)
			 set:custom(false)
			 get:custom(true))
	      vlinkColor#r(type:color
			   init:custom(true)
			   set:custom(true)
			   get:custom(true))
	      title#r(type:vs
%		      default:"QHTML application"
		      init:custom(true)
		      set:custom(true)
		      get:internal(true))
	      clientInformation#r(type:no
				  init:custom(false)
				  set:custom(false)
				  get:custom(true))
	      defaultStatus#r(type:vs
			      init:custom(true)
			      set:custom(true)
			      get:internal(true))
	      innerHeight#r(type:int
			    init:custom(true)
			    set:custom(true)
			    get:custom(true))
	      innerWidth#r(type:int
			   init:custom(true)
			   set:custom(true)
			   get:custom(true))
	      location#r(type:vs
			 init:custom(false)
			 set:custom(false)
			 get:custom(true))
	      outerHeight#r(type:int
			    init:custom(true)
			    set:custom(true)
			    get:custom(true))
	      outerWidth#r(type:int
			   init:custom(true)
			   set:custom(true)
			   get:custom(true))
	      pageXOffset#r(type:int
			    init:custom(true)
			    set:custom(true)
			    get:custom(true))
	      pageYOffset#r(type:int
			    init:custom(true)
			    set:custom(true)
			    get:custom(true))
	      status#r(type:vs
		       init:custom(true)
		       set:custom(true)
		       get:internal(true))
	      screen#r(type:no
		       init:custom(false)
		       set:custom(false)
		       get:custom(true))
	      menuButtonBottomColor#r(type:color
				      default:rgb(80 80 80)
				      init:custom(true)
				      set:custom(false)
				      get:internal(true))
	      menuButtonRightColor#r(type:color
				     default:rgb(80 80 80)
				     init:custom(true)
				     set:custom(false)
				     get:internal(true))
	      menuButtonTopColor#r(type:color
				   default:rgb(207 207 207)
				   init:custom(true)
				   set:custom(false)
				   get:internal(true))
	      menuButtonLeftColor#r(type:color
				    default:rgb(207 207 207)
				    init:custom(true)
				    set:custom(false)
				    get:internal(true))
	      menuBgColor#r(type:color
			    default:rgb(224 224 224)
			    init:custom(true)
			    set:custom(false)
			    get:internal(true))
	      menuFgColor#r(type:color
			    default:black
			    init:custom(true)
			    set:custom(false)
			    get:internal(true))
	     ] 
      events:[onconnect#r(bind:custom(true))
	      ondisconnect#r(bind:custom(true))])
    class $ from BaseClass
       feat
	  ObjDict
	  GetParamsDict
	  Child
       attr
	  objIdNu
	  getParamsNu
	  page
       meth !QInit(...)=M
	  self.ObjDict={NewDictionary}
	  objIdNu<-0
	  self.GetParamsDict={NewDictionary}
	  getParamsNu<-0
	  {self Set(childPrefixId "main")}
	  {self Set(refId ""#{self get(id:$)})}
	  self.Child={QUI.build M.1 self}
	  local
	     Result={Record.filterInd M
		     fun{$ I V}
			if I==1 then false
			elseif I==2 then
			   {QUI.raiseError self.widgetId "Toplevel widget can contain one and only one widget" M}
			   false
			elseif {List.member I [menuButtonBottomColor menuButtonRightColor menuButtonTopColor menuButtonLeftColor menuBgColor menuFgColor]} then
			   false
			else true
			end
		     end}
	  in
	     {self {Record.adjoin Result set}}
	  end
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      P={Translate2HTML V self.DataInfo.I.type}
	   in
	      if {List.member I [defaultStatus innerHeight innerWidth outerHeight outerWidth pageXOffset pageYOffset status]} then
		 {self send('window.'#I#'='#{Quote P})}
	      else
		 {self send('document.'#I#'='#{Quote P})}
	      end
	   end}
       end
       meth get(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I
	      of isconnected then V={IsDet @page}
	      [] html then V={VirtualString.toString
			      if {List.member self.Child.widgetId [frameset tdframe lrframe]} then
				 {self BuildInnerHTML($)}
			      else
				 {self BuildHTML($)}
			      end}
	      [] activeElement then
		 Ref={self return('ozreturn=document.activeElement.id' $)}
	      in
		 if Ref==Undefined then V=Ref else
		    V={Dictionary.condGet self.ObjDict {String.toAtom Ref} Undefined}
		 end
	      [] clientInformation then
		 V=fun{$ N}
		      C=[appCodeName#vs
			 appMinorVersion#vs
			 appName#vs
			 appVersion#vs
			 browserLanguage#vs
			 cookieEnabled#bool
			 cpuClass#vs
			 language#vs
			 onLine#bool
			 platform#vs
			 systemLanguage#vs
			 userAgent#vs
			 userLanguage#vs
			 javaEnabled#bool
			 taintEnabled#bool]
		      Type
		   in
		      if {List.some C fun{$ E#T} if E==N then Type=T true else false end end} then
			 {Translate2OZ {self return('ozreturn=window.clientInformation.'#N $)} Type}
		      else
			 {QUI.raiseError clientInformation "Invalid client information requested" N}
			 Undefined
		      end
		   end
	      [] screen then
		 V=fun{$ N}
		      C=[availHeight#int
			 availWidht#int
			 availLeft#int
			 availTop#int
			 bufferDepth#int
			 colorDepth#int
			 height#int
			 width#int
			 pixelDepth#int
			 updateInterval#int]
		      Type
		   in
		      if {List.some C fun{$ E#T} if E==N then Type=T true else false end end} then
			 {Translate2OZ {self return('ozreturn=window.screen.'#N $)} Type}
		      else
			 {QUI.raiseError clientInformation "Invalid screen information requested" N}
			 Undefined
		      end
		   end		 
	      else
		 Ref=if {List.member I [innerHeight innerWidth location outerHeight outerWidth pageXOffset pageYOffset]} then
			{self return('ozreturn=window.'#I $)}
		     else
			{self return('ozreturn=document.'#I $)}
		     end
	      in
		 V={Translate2OZ Ref self.DataInfo.I.type}
	      end
	   end}
       end
       meth elementFromPoint(X Y $)
	  Ref={self return('ozreturn=document.elementFromPoint('#X#','#Y#').id' $)}
       in
	  {Dictionary.condGet self.ObjDict
	   if Ref==Undefined then Ref else {VirtualString.toAtom Ref} end
	   Undefined}
       end
       meth !GetId(Obj I)
	  if Obj==self then I=root else
	     I={VirtualString.toAtom e#@objIdNu}
	     self.ObjDict.I:=Obj
	     objIdNu<-@objIdNu+1
	  end
       end
       meth !BuildInnerHTML(V)
	  {self.Child BuildHTML(V)}
       end
       meth connect(P)
	  In Out ThId
       in
	  {P getIOStreams(In Out)}
	  page<-r(obj:P out:Out thid:ThId)
	  {self send("setmenucolor("#{Join
				      {List.map [menuButtonBottomColor menuButtonRightColor menuButtonTopColor menuButtonLeftColor menuBgColor menuFgColor]
				       fun{$ C}
					  {Record2str {self Get(C $)}}
				       end}
				      ","}#")")}
	  {self send(putHTML({self get(html:$)}))}
	  thread
	     ThId={Thread.this}
	     {ForAll In
	      proc{$ O}
		 Remain
	      in
\ifdef DEBUG		 
		 {Print "Received :"#O}
\endif		 
		 case {SplitV O Remain}
		 of event then
		    Event
		    Obj={Dictionary.get self.ObjDict {SplitV Remain Event}}
		 in
		    {Obj ProcessEvent(Event)}
		 [] menu then
		    L R
		    {List.takeDropWhile Remain fun{$ C} C\=&m end L R}
		    Obj={Dictionary.get self.ObjDict {String.toAtom L}}
		    Nu={SplitV {List.drop R 1} _}
		 in
		    {Obj ProcessEvent({VirtualString.toString Nu#","})}
		 [] get then
		    lock
		       Val
		       Id={SplitV Remain Val}
		       X|Xs={Dictionary.get self.GetParamsDict Id}
		    in
		       X=case Val of &0|Rs then Rs else Undefined end
		       {Dictionary.put self.GetParamsDict Id Xs}
		    end
		 [] bye then {self disconnect}
		 else skip end % ignore garbage
	      end}
	  end
	  {self OnConnect}
       end
       meth disconnect
	  if {IsDet @page} then
	     ThId=@page.thid
	  in
	     page<-_
	     {self OnDisconnect}
	     {Thread.terminate ThId}
	  end
       end
       meth send(M)
	  Str
       in
	  if {IsDet @page} then
	     if {VirtualString.is M} then
		Str=M
	     else
		Str="top."#{Record2HTML M}
	     end
\ifdef DEBUG	     
	     {Print "Sending :"#Str}
\endif	     
	     {Send @page.out Str}
	  end
       end
       meth return(S M)
	  if {IsDet @page} then
	     lock
		Id={VirtualString.toAtom @getParamsNu}
		getParamsNu<-((@getParamsNu+1) mod GetBufferSize)
		%% find end of list
		fun{FindEnd L}
		   if {IsDet L} then
		      _|Xs=L in {FindEnd Xs}
		   else L end
		end
		if {Not {Dictionary.member self.GetParamsDict Id}} then {Dictionary.put self.GetParamsDict Id _} end
		X|_={FindEnd {Dictionary.get self.GetParamsDict Id}}
	     in
		X=M
		{self send({SQuote "var ozreturn; "#S#"; sendToOZ('get','"#Id#",'+enc(ozreturn));"})}
	     end
	  else
	     M=Undefined
	  end
       end
       meth configure(Obj Param Value)
	  {self send({Obj get(refId:$)}#"."#Param#'='#Value)}
       end
       meth cget(Obj Param Value)
	  {self return("ozreturn="#{Obj get(refId:$)}#"."#Param Value)}
       end
       meth saveHTML(FileName)
	  HTML={self get(html:$)}
	  F
       in
	  try
	     F={New Open.file init(name:FileName
				   flags:[write create truncate])}
	     {F write(vs:"<html>\n"#HTML#"\n</html>")}
	     {F close}
	  catch _ then raise error(unableToSaveFile) end end
       end
       meth !OnConnect
	  {self ReSetCustom}
	  {self.Child OnConnect}
	  {self ExecEvent(onConnect)}
       end
       meth !OnDisconnect
	  {ForAll {Dictionary.items self.GetParamsDict} % pending gets are set to undefined
	   proc{$ E}
	      proc{Loop R}
		 if {IsDet R} then
		    try R.1=Undefined catch _ then skip end
		    {Loop {List.drop R 1}}
		 end
	      end
	   in
	      {Loop E}
	   end}
	  {self.Child OnDisconnect}
	  {self ExecEvent(ondisconnect)}
       end
       meth openURL(url:URL
		    copyHistory:CopyHistory<=false
		    directories:Directories<=true
		    height:Height<=NoArgs
		    left:Left<=NoArgs
		    location:Location<=true
		    menuBar:MenuBar<=true
		    resizable:Resizable<=true
		    scrollbars:ScrollBars<=true
		    status:Status<=true
		    toolbar:Toolbar<=true
		    top:Top<=NoArgs
		    width:Width<=NoArgs)=M
	  if {IsDet @page} then
	     BL=[copyHistory#CopyHistory directories#Directories location#Location
		 menuBar#MenuBar resizable#Resizable scrollbars#ScrollBars status#Status
		 toolbar#Toolbar]
	     IL=[height#Height left#Left top#Top width#Width]
	     {ForAll BL
	      proc{$ _#V} {CheckType V bool M} end}
	     {ForAll IL
	      proc{$ _#V} if V\=NoArgs then {CheckType V int M} end end}
	     Params
	  in
	     Params={Join
		     {List.append
		      {List.map
		       {List.filter BL fun{$ _#I} I end}
		       fun{$ N#_} N end}
		      {List.map
		       {List.filter IL fun{$ _#I} I\=NoArgs end}
		       fun{$ N#V} N#"="#V end}
		     }
		     ","}
	     {self send({SQuote "open('"#URL#"','"#
%			 {Reverse {List.takeWhile {Reverse {VirtualString.toString URL}}
%				   fun{$ C} C\=&/ end}}#
			 "_blank"#
			 "','"#Params#"')"})}
	  end
       end
       meth openWindow(connect:O
		       ...)=M
	  if {IsDet @page} then
	     % start a new connection server
	     SPort={ConnectionServer.startServer
		    proc{$ C}
		       if {IsFree O} then O=C end
		    end}
	     WPort
	     WObj={New QHTMLWebServer.webServer init(port:WPort)}
	     Name={GetRandomStr}
	     TSt
	  in
	     {WObj defineIndex(SPort home:Name)}
	     {WObj start}
	     {self {Record.subtract
		    {Record.adjoin M openURL(url:"http://"#{QHTMLWebServer.getThisIP}#":"#WPort#"/"#Name)}
		    connect}}
	     thread
		{Delay 30000} % waits 30 seconds for the connection to be made
		TSt=unit
	     end
	     {WaitOr TSt O} % in any case, close the connection
	     thread
		{WObj stop}
	     end
	     thread
		{ConnectionServer.stopServer SPort}
	     end
	  end
       end
       meth show
	  if {IsFree @page} then
	     % start a new connection server
	     O
	     SPort={ConnectionServer.startServer
		    proc{$ C}
		       if {IsFree O} then O=C end
		    end}
	     WPort
	     WObj={New QHTMLWebServer.webServer init(port:WPort)}
	     Name={GetRandomStr}
	     TSt
	  in
	     {WObj defineIndex(SPort home:Name)}
	     {WObj start}
	     {DisplayUrl "http://127.0.0.1:"#WPort#"/"#Name}
	     thread
		{Delay 120000} % waits 120 seconds for the connection to be made (Netscape can be slow to load)
		TSt=unit
	     end
	     {WaitOr TSt O} % in any case, close the connection
	     thread
		{WObj stop}
	     end
	     thread
		{ConnectionServer.stopServer SPort}
	     end
	     if {IsDet O} then
		{self connect(O)}
		if {Not {Dictionary.member self.DataVal ondisconnect}} then
		   {self bind(event:ondisconnect action:proc{$} {self close} end)} % default action : close the window upon disconnection
		end
	     end
	  end
       end
       meth alert(Msg)=M
	  {CheckType Msg vs M}
	  {self send('window.alert("'#{QuoteDouble Msg}#'")')}
       end
       meth blur
	  {self send('window.blur()')}
       end
       meth close
	  {self.Child close}
	  {self send('document.write("Application closed."); window.close();')}
       end
       meth confirm(Msg $)=M
	  {CheckType Msg vs M}
	  T={self return('ozreturn=window.confirm("'#{QuoteDouble Msg}#'")' $)}
       in
	  if T==Undefined then Undefined elseif T=="true" then true else false end
       end
       meth focus
	  {self send('window.focus()')}
       end
       meth moveBy(X Y)=M
	  {CheckType X int M}
	  {CheckType Y int M}
       in
	  {self send('window.moveBy('#X#','#Y#');')}
       end
       meth moveTo(X Y)=M
	  {CheckType X int M}
	  {CheckType Y int M}
       in
	  {self send('window.moveTo('#X#','#Y#');')}
       end
       meth print
	  {self send('window.print()')}
       end
       meth prompt(Msg Default<="" $)=M
	  {CheckType Msg vs M}
	  {CheckType Default vs M}
	  T={self return('ozreturn=window.prompt("'#{SQuote Msg}#'","'#{SQuote Default}#'"); if (ozreturn==null) {ozreturn="false,0"} else {ozreturn="true,"+ozreturn};' $)}
       in
	  {Wait T}
	  if T==Undefined then Undefined else
	     R
	  in
	     if {SplitV T R}=='false' then false
	     else R end
	  end
       end
%       meth resizeBy(X Y)=M % not available with IE
%	  {CheckType X int M}
%	  {CheckType Y int M}
%       in
%	  {self send('window.resizeBy('#X#','#Y#');')}
%       end
       meth scroll(X Y)=M
	  {CheckType X int M}
	  {CheckType Y int M}
       in
	  {self send('window.scroll('#X#','#Y#');')}
       end
       meth scrollBy(X Y)=M
	  {CheckType X int M}
	  {CheckType Y int M}
       in
	  {self send('window.scrollBy('#X#','#Y#');')}
       end
       meth scrollTo(X Y)=M
	  {CheckType X int M}
	  {CheckType Y int M}
       in
	  {self send('window.scrollTo('#X#','#Y#');')}
       end
    end}

end
   
      

   
