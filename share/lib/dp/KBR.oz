functor
import
   Glue at 'x-oz://boot/Glue'
   DPInit
export
   KbrInitializeNet
   KbrConnectNet
%   KbrGetStream
   KbrRoute
   KbrNetDesc
   KbrTransferResp
%   LookupDHTitem
%   RegisterKBRInterest

define
   {Wait Glue}
   {DPInit.init connection_settings _}
   

   local 
      LastCell = {NewCell _}
      NotInitialized = {NewCell true}
      proc{AssignLast S}
	 {Assign LastCell S}
	 {AssignLast S.2}
      end
      fun{Init} O N in
	 {Exchange NotInitialized O N}
	 if O then P in
	    thread {AssignLast {NewPort $ P}} end
	    N = not_started(P)
	    N
	 else
	    N =O
	    started
	 end
      end
   in
      proc{KbrInitializeNet}
	 case {Init} of not_started(P) then 
	    {Glue.createDHT P}
	 else
	    skip
	 end
      end
      
      proc{KbrConnectNet Str}
	 case {Init} of not_started(P) then 
	    {Glue.connectDHT P Str}
	 else
	    skip
	 end
      end
   end
   
   fun{KbrNetDesc}
      {Glue.createSiteRef}
   end
   
   proc{KbrRoute Key Value}
      {Glue.insertDHTitem Key Value}
   end
   
   proc{KbrTransferResp Val}
      {Glue.transferRespKBR Val}
   end
end
