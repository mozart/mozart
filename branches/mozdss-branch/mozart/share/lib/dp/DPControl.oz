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

   local
      %% annotations as constraints
      Constrain =
      constr(stationary: fun {$}  1#_#_#_#_ end     % PN_SIMPLE_CHANNEL
	     migratory:  fun {$}  2#_#_#_#_ end     % PN_MIGRATORY_STATE
	     pilgrim:    fun {$}  9#_#_#_#_ end     % PN_PILGRIM_STATE
	     replicated: fun {$}  5#_#_#_#_ end     % PN_EAGER_INVALID
	     variable:   fun {$}  3#_#_#_#_ end     % PN_TRANSIENT
	     replyvar:   fun {$}  4#_#_#_#_ end     % PN_TRANSIENT_REMOTE
	     ondemand:   fun {$} 11#_#_#_#_ end     % PN_IMMUTABLE_LAZY
	     persistent: fun {$}  _#_#1#0#0 end     % RC_ALG_PERSIST
	     refcount:   fun {$}  _#_#0#2#_ end     % RC_ALG_WRC
	     lease:      fun {$}  _#_#0#_#4 end     % RC_ALG_TL
	    )
   in
      %% annotate an entity
      proc {Annotate E As}
	 PN AA RC0 RC1 RC2
	 Annot = PN#AA#RC0#RC1#RC2
      in
	 try
	    %% constrain Annot with the elements of As, and complete with
	    %% zeroes (= PN_NO_PROTOCOL, AA_NO_ARCHITECTURE, RC_ALG_NONE)
	    for A in As do {Constrain.A Annot} end
	    {Record.forAll Annot proc {$ X} if {IsFree X} then X=0 end end}
	 catch _ then
	    raise dp('annotation format error' As) end
	 end
	 %% now set annotation with the parameters
	 {Glue.setAnnotation E PN AA RC0+RC1+RC2}
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
