functor
export
   'class' : Externalizable
define
   class Externalizable
      meth externalize($)
	 Label = {self extern_label($)}
	 Slots = {self extern_slots($)}
	 D = {NewDictionary}
      in
	 for S in Slots do
	    D.S := {self externalize_slot(S @S $)}
	 end
	 {Dictionary.toRecord Label D}
      end
      meth internalize(R)
	 {Record.forAllInd R
	  proc {$ K V}
	     {self internalize_slot(K V)}
	  end}
      end
      meth externalize_slot(S V $) V end
      meth internalize_slot(S V) S<-V end
   end
end
