%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Simple use of the fetchRate, bytesRead, stopTemp
%% and startTemp methods of HttpClient.urlGET class
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
functor
import 
   System
   Application
   HttpClient(urlGET:UrlGET) at 'HttpClient.ozf'  
define
   Url = "Address of the document"
   OutPrms
   ApplicEnd
   proc {UseIt}
      HttpObj
      ServiceEnd
      proc {GetRate B}
	 if {IsDet B}==false then
	    {Delay 300}
	    {System.show {HttpObj fetchRate($)}#{HttpObj bytesRead($)}}
	    {GetRate B}
	 end
      end
      proc {ProcessStrm Stream} 
	 %% Processing the Stream
	 ...
      end
   in      
      HttpObj={New UrlGET init(inPrms(file:"temp" toStrm:true))}
      thread
	 try
	    {HttpObj getService(Url OutPrms _)}
	 catch E then
	    {System.show E}
	 finally 
	    ServiceEnd=unit
	 end
      end
      thread {GetRate ServiceEnd} end
      thread {ProcessStrm OutPrms.sOut} end
      {Delay 850}
      %% temporarely stops the transmission
      {HttpObj stopTemp}
      {Delay 3000}
      %% continues the transmission
      {HttpObj startTemp}
      {Wait ServiceEnd}
   end
in
   thread {UseIt} ApplicEnd=unit end
   {Wait ApplicEnd}
   {Application.exit 0}
end
