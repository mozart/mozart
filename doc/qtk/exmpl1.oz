%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Simple use of HttpClient.urlGET class
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
functor
import 
   System
   Application
   HttpClient(urlGET:UrlGET) at 'HttpClient.ozf' 
define
   Url = "http://www.info.ucl.ac.be/index.html"
   HttpObj
   ServiceEnd
in   
   HttpObj={New UrlGET init(_)}
   thread
      try
	 {HttpObj getService(Url _ _)}
      catch E then
	 {System.show E}
      finally 
	 ServiceEnd=unit
      end
   end
   {Wait ServiceEnd}
   {Application.exit 0}
end
