functor

export
   CollectConstraintsClass
   
import
   Misc(memberEqVar memberEqProp varReflect vectorToList)
   
define
\ifdef DEBUG
   class VarsBucketImpl
\else
   class VarsBucket
      attr vs: nil
\endif
	 
      meth init skip end
      
      meth is_in(V B) B = {Misc.memberEqVar V @vs} end
      
      meth put(V) vs <- V | @vs end
      
      meth not_empty(B) B = @vs \= nil end
      
      meth get(Vs) Vs = @vs end
   end
   
\ifdef DEBUG
   class VarsBucket
      attr vs: nil
	 
      meth otherwise(M)
	 {Show 'VarsBucket'#M}
	 VarsBucketImpl,M
      end
   end
\endif
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
   
\ifdef DEBUG
   class PropsBucketImpl
\else
    class PropsBucket
       attr ps: nil
\endif
	  
       meth init skip end
       
       meth is_in(P B) B = {Misc.memberEqProp P @ps} end
       
       meth not_empty(B) B = @ps \= nil end
       
       meth get(P) P = @ps end
       
       meth put(V VarBucket)
	  SuspLists = {Misc.varReflect V}.susplists
	  SuspListNames = {Arity SuspLists}
       in
	  {ForAll SuspListNames
	   proc {$ Name}
	      {ForAll SuspLists.Name
	       proc {$ Susp}
		  if Susp.type == propagator then
		     if PropsBucket,is_in(Susp.reference $) 
			  then skip
		     else
			ps <- Susp.reference | @ps
			{ForAll {Misc.vectorToList Susp.params}
			 proc {$ V} {VarBucket put(V)} end}
		     end
		  else skip end
	       end}
	   end}
       end
    end
    
\ifdef DEBUG
    class PropsBucket
       attr ps:nil
	  
       meth otherwise(M)
	  {Show 'PropsBucket'#M}
	  PropsBucketImpl,M
       end
    end
\endif
    
    
%%-----------------------------------------------------------------------------
    
    class CollectConstraintsClass
       attr
	  vars props
	  
       meth init
	  vars  <- {New VarsBucket  init}
	  props <- {New PropsBucket init}      
       end
       
       meth collect(Vs)
	  {ForAll {Misc.vectorToList Vs}
	   proc {$ V}
	      if {@vars is_in(V $)} then skip
	      else
		 NewParameters = {New VarsBucket init}
	      in
		 {@vars put(V)}
		 {@props put(V NewParameters)}
		 
		 CollectConstraintsClass,collect({NewParameters get($)}) 
	      end
	   end}
       end
       
       meth get_vars(Vs) Vs = {@vars get($)} end
       
       meth get_props(Ps) Ps = {@props get($)} end
    end
end
