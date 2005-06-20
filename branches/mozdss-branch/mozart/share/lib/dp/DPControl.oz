functor
import
   Glue at 'x-oz://boot/Glue'
   DPInit
   System
export
   SetDGC
   GetDGC
   GetMsgPriority
   SetMsgPriority
   Annotate
   PrintDSSTables
   Migrate
   SetDssLogLevel
define
   %%
   %% Force linking of base library
   %%
   {Wait Glue}
   {DPInit.init connection_settings _}
   
   fun{SetDGC E Algs}
      case {GetDGC E} of local_entity then
	 local_entity
      elseof persistent then
	 persistent == persistent
      elseof A then 
	 Algs2 = if Algs == persistent then nil else Algs end
      in
	 if {All Algs2 fun{$ M} {Member M A} end} then 
	    {ForAll
	     {Filter A fun{$ M} {Not {Member M Algs2}} end}
	     proc{$ Al}
		{Glue.setDGC  E Al _}
	     end}
	    true
	 else
	    false
	 end
      end
   end
   fun{GetDGC E}
      case {Glue.getDGC E} of
	 nil then persistent
      elseof local_entity then
	 local_entity
      elseof A then 
	 {Map A fun{$ V} V.1 end}
      end
   end

   fun{GetDGCAlgs}
      {Glue.getDGCAlgs}
   end

   fun{GetDGCAlgInfo Alg}      
      if {Value.condSelect {GetDGCAlgs} Alg 'not_found'} == not_found then
	 algorithm_not_found
      else
	 {Glue.getDGCAlgInfo Alg}
      end
   end

   fun{SetDGCAlg Alg Use}
      if {Value.condSelect {GetDGCAlgs} Alg 'not_found'} == not_found then
	 algorithm_not_found
      else
	 {Glue.setDGCAlg Alg Use}
	 done
      end
   end

   fun{SetDGCAlgProp Alg Prop Val}
      if {Not {Number.is Val}} then
	 illegal_property_value
      elseif {Value.condSelect {GetDGCAlgs} Alg 'not_found'} == not_found then
	 algorithm_not_found
      elseif {Value.condSelect {GetDGCAlgInfo Alg} Prop 'no_prop'} == no_prop then
	 property_not_found
      else
	 {Glue.setDGCAlgProp Alg Prop Val}
	 done
      end
   end

   fun{GetMsgPriority}
      {Glue.getMsgPriority}
   end

   proc{SetMsgPriority Msg Prio}
      {Glue.setMsgPriority Msg Prio}
   end

   proc{Annotate E An}
      Dec = r(synch_chanel: 1
	      migratory:    2
	      once_only:    3
	      once_only_remote: 4
	      invalidation_eager:  5
	      stationary:    1
	      pilgrim:       9
	      invalidation_lazy:  10
	      
	      persistent:   1 * 256
	      fracWRC:      2 * 256
	      timelease:    4 * 256
	      irc:          8 * 256
	      refList_1:    16 * 256
	      refList_2:    32 * 256

	      stationary_manager: 1 * 256 * 256
	      mobile_manager:     2 * 256 * 256
	     )
   
      IntVal = {FoldL An fun{$ Ind V} Ind + Dec.V end 0}
   in
      {Glue.setAnnotation E IntVal}
   end
   
   proc{PrintDSSTables}
      {Glue.printDPTables}
   end

   proc{Migrate E}
      {Glue.migrateManager E}
   end

   proc{SetDssLogLevel Level}
      I = case Level
	  of nothing then 0 
	  [] print   then 1
	  [] important then 2
	  [] behavior then 3
	  [] debug then 4
	  [] all then 5
	  end
   in
      {Glue.setDssLogLevel I}
   end

end
