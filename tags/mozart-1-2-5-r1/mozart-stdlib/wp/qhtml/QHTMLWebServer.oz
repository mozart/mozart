functor

import

   LiteWebServer
   SiteData(data:Data)
   Open
   Connection

export

   StartServer
   StartServerOnPort
   GetRedirector
   WriteRedirector
   StopServer
   GetThisIP
   WebServer

define

   DefaultL DefaultR

   ServerDict={NewDictionary}
   
   {ForAll Data
    proc{$ Ind#Data}
       if Ind=="default.htm" then
	  F={VirtualString.toString '<PARAM NAME="port" VALUE="'}
	  LF={Length F}
	  proc{Split Str Left Right}
	     fun{Loop L I}
		if {List.take L LF}==F then I+LF-1
		else
		   _|Cs=L
		in
		   {Loop Cs I+1}
		end
	     end
	  in
	     {List.takeDrop Str {Loop Str 1} Left Right}
	  end
	  B
       in
	  {Split Data DefaultL B}
	  DefaultR={List.dropWhile B fun{$ C} C\=&" end} % "
       end
    end}
   
   class WebServer from LiteWebServer.webServer
      meth init(...)=M
	 LiteWebServer.webServer,{Record.filterInd M fun{$ I _} I==port orelse I==takePort end}
	 {ForAll Data
	  proc{$ Ind#Data}
	     if Ind\="default.htm" then
		{self addEntry(Ind Data)}
	     end
	  end}
      end
      meth defineIndex(P home:Home<=unit)
	 {self addEntry(Home
			{VirtualString.toString DefaultL#P#DefaultR})}
      end
   end


   
   fun{StartServer Port}
      P
      S={New WebServer init(port:P)}
   in
      {Dictionary.put ServerDict P S}
      {S defineIndex(Port)}
      {S start}
      P
   end

   proc{StartServerOnPort Port PortNu}
      S={New WebServer init(takePort:PortNu)}
   in
      {Dictionary.put ServerDict PortNu S}
      {S defineIndex(Port)}
      {S start}
   end
   
   fun{GetThisIP}
      %% VERY UGLY, but the only solution I found so far :(
      Ticket={Connection.offer unit}
      {Connection.take Ticket _}
   in
      {List.takeWhile
       {List.drop {VirtualString.toString Ticket} 13}
       fun{$ C} C\=&: end}
   end
   
   fun{GetRedirector Port}
      URL="http://"#{GetThisIP}#":"#Port#"/"
   in
      '<html><head><meta http-equiv="pragma" content="no-cache"><meta http-equiv="expires" content="Tue, 20 Feb 2001 16:11:32 GMT"><meta http-equiv="Refresh" content="0; URL='#URL#'"><title>QHTML redirector page</title></head><body><table style="width:100%; height:100%"><tr><td align=middle><a href="'#URL#'">Click here if your webbrowser is not redirecting you automatically.</a></td></tr></table></body></html>'
   end
   
   proc{WriteRedirector Port FileName}
      HTML={GetRedirector Port}
      F
   in
      try
	 F={New Open.file init(name:FileName
			       flags:[write create truncate])}
	 {F write(vs:HTML)}
	 {F close}
      catch _ then raise error(unableToSaveFile) end end
   end

   proc{StopServer P}
      {{Dictionary.get ServerDict P} stop}
   end

end
