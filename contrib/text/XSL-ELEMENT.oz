functor
import
   Search(
      conj	: AND
      always	: ALWAYS
      ) at 'XSL-SEARCH.ozf'
   Pos  at 'XSL-POSITION.ozf'
export
   Element WithQualification
define
   fun {Element Type Qual}
      {AND [{WithType Type} {WithQualification Qual}]}
   end
   %%
   fun {WithType Type}
      case Type of any then ALWAYS
      elseof type(ID) then
	 fun {$ Node Stack Info Yes No}
	    if {CondSelect Node type unit}==ID then
	       {Yes Node Stack Info No}
	    else {No} end
	 end
      end
   end
   %%
   fun {WithQualification Qual}
      L = {Map Qual WithQualifier}
   in {AND L} end
   %%
   fun {WithQualifier Q}
      case Q
      of child(TYPE) then {WithQualChild TYPE}
      elseof position(POS) then {WithQualPosition POS}
      elseof attribute(ATTR SUCH) then {WithQualAttribute ATTR SUCH}
      end
   end
   %%
   fun {WithQualChild TYPE}
      fun {OK X} {CondSelect X type unit}==TYPE end
   in
      fun {$ Node Stack Info Yes No}
	 if {Some Node.content OK} then
	    {Yes Node Stack Info No}
	 else {No} end
      end
   end
   %%
   fun {WithQualPosition POS}
      P = Pos.POS
   in
      fun {$ Node Stack Info Yes No}
	 case Stack of Parent|_ then
	    if {P Node Parent.content} then
	       {Yes Node Stack Info No}
	    else {No} end
	 else {No} end
      end
   end
   %%
   fun {WithQualAttribute ATTR SUCH}
      case SUCH
      of exists then
	 fun {$ Node Stack Info Yes No}
	    if {HasFeature {CondSelect Node attribute unit} ATTR}
	    then {Yes Node Stack Info No} else {No} end
	 end
      elseof equal(V) then
	 fun {$ Node Stack Info Yes No}
	    if {CondSelect {CondSelect Node attribute unit} ATTR unit}
	       ==V
	    then {Yes Node Stack Info No} else {No} end
	 end
      elseof contains(V) then
	 raise notImplemented(attribute(ATTR SUCH)) end
      end
   end
end
