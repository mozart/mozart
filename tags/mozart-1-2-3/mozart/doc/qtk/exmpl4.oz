%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Parallel document retrieval 
%% using HttpClient.urlGET class
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
functor
import 
   System
   Application
   HttpClient(urlGET:UrlGET) at 'HttpClient.ozf' 
define
   Url1 = "Address of the document located in USA"
   Url2 = "Address of the document located in Australia"
   ApplicEnd
   proc {UseIt}
      HttpObj1
      HttpObj2
      Out1 Http1
      Out2 Http2
      ServiceEnd
   in      
      HttpObj1={New UrlGET init(inPrms(file:"temp1"))}
      HttpObj2={New UrlGET init(inPrms(file:"temp2"))}
      thread
	 try
	    {HttpObj1 getService(Url1 Out1 Http1)}
	 catch E then
	    {System.show E}
	 finally
	    ServiceEnd=unit
	 end
      end
      thread
	 try
	    {HttpObj2 getService(Url2 Out2 Http2)}
	 catch E then
	    {System.show E}
	 finally
	    ServiceEnd=unit
	 end
      end
      {Wait ServiceEnd}
   end
in
   thread {UseIt} ApplicEnd=unit end
   {Wait ApplicEnd}
   {Application.exit 0}
end
