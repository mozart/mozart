%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Simple use of HttpClient.cgiGET class
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
functor
import 
   System
   Application
   HttpClient(cgiGET:CgiGET) at 'HttpClient.ozf'   
define
   Url = "search.yahoo.com/bin/search"
   CgiParams = ["p=travel"]
   OutPrms
   HttpPrms
   HttpObj
   ServiceEnd
in   
   HttpObj={New CgiGET init(inPrms(file:"temp"))}
   thread
      try
	 {HttpObj getService(Url CgiParams OutPrms HttpPrms)}
      catch E then
	 {System.show E}
      finally 
	 ServiceEnd=unit
      end
   end
   {Wait ServiceEnd}
   {System.show sizeread#OutPrms.sizeRead}
   {System.show http_server#{VirtualString.toAtom HttpPrms.server}}
   {Application.exit 0}
end
