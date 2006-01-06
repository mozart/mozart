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

   %% valid annotations
   Protocol = protocol(stationary: 1     % PN_SIMPLE_CHANNEL
		       migratory:  2     % PN_MIGRATORY_STATE
		       replicated: 5     % PN_EAGER_INVALID
		       variable:   3     % PN_TRANSIENT
		       replyvar:   4     % PN_TRANSIENT_REMOTE
		       ondemand:   11    % PN_IMMUTABLE_LAZY
		      )
   RefCount = refcount(persistent: 1     % RC_ALG_PERSIST
		       refcount:   2     % RC_ALG_WRC
		       lease:      4     % RC_ALG_TL
		      )

   %% annotate an entity
   proc {Annotate X An}
      Apn Arc
      PN AA RC
   in
      try
	 {List.partition An fun {$ A} {HasFeature Protocol A} end Apn Arc}
	 PN = case Apn
	      of nil then 0     % PN_NO_PROTOCOL
	      [] [P] then Protocol.P
	      end
	 AA = 0     % AA_NO_ARCHITECTURE
	 RC = for R in Arc  sum:Sum do {Sum RefCount.R} end
      catch _ then
	 raise dp('annotation syntax error' An) end
      end
      try
	 {Glue.setAnnotation X PN AA RC}
      catch E then
	 raise {Tuple.append E dp(An)} end
      end
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
