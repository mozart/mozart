functor
import
   Regex at 'x-oz://contrib/regex'
   Text(
      makeBS      : MakeBS
      strip       : Strip
      toLower     : ToLower
      emptyToUnit : EmptyToUnit
      capwords    : Capwords
      )
export
   'class' : ContactName
define
   RE1 = {Regex.compile '^(van|von|de|di)([[:space:]].*),(.*)$'           [extended icase]}
   RE2 = {Regex.compile '^(.*),(.*)$'                                     [extended icase]}
   RE3 = {Regex.compile '^(.*[[:space:]])(van|von|de|di)([[:space:]].*)$' [extended icase]}
   RE4 = {Regex.compile '(.*[[:space:]])([^[:space:]]+)[[:space:]]*$'     [extended icase]}
   %%
   class ContactName
      feat von first last
      meth init(S)
	 D = {Strip {MakeBS S}}
      in
	 try
	    case {Regex.search RE1 D} of false then skip
	    [] M then raise found(
			       von  : {Regex.group 1 M D}
			       last : {Regex.group 2 M D}
			       first: {Regex.group 3 M D})
		      end
	    end
	    case {Regex.search RE2 D} of false then skip
	    [] M then raise found(
			       von  : unit
			       last : {Regex.group 1 M D}
			       first: {Regex.group 2 M D})
		      end
	    end
	    case {Regex.search RE3 D} of false then skip
	    [] M then raise found(
			       von  : {Regex.group 2 M D}
			       last : {Regex.group 3 M D}
			       first: {Regex.group 1 M D})
		      end
	    end
	    case {Regex.search RE4 D} of false then skip
	    [] M then raise found(
			       von  : unit
			       last : {Regex.group 2 M D}
			       first: {Regex.group 1 M D})
		      end
	    end
	    raise found(
		     von  : unit
		     last : S
		     first: unit)
	    end
	 catch found(von:V first:F last:L) then
	    Von   = if V==unit then unit else {EmptyToUnit {ToLower  {Strip V}}} end
	    First = if F==unit then unit else {EmptyToUnit {Capwords {Strip F}}} end
	    Last  = if L==unit then unit else {EmptyToUnit {Capwords {Strip L}}} end
	 in
	    self.von   = Von
	    self.first = First
	    self.last  = Last
	 end
      end
      meth toVSIndex($)
	 V1 = if self.first==unit then '' else ', '#self.first      end
	 V2 = if self.von  ==unit then V1 else ' ('#self.von#')'#V1 end
	 V3 = if self.last ==unit then V2 else self.last#' '#V2     end
      in
	 V3
      end
      meth toVS($)
	 V1 = if self.last ==unit then '' else self.last         end
	 V2 = if self.von  ==unit then V1 else self.von#' '#V1   end
	 V3 = if self.first==unit then V2 else self.first#' '#V2 end
      in
	 V3
      end
   end
end
