functor
import
   Regex at 'x-oz://contrib/regex'
   URL(make resolve toAtom toBase)
export
   NormalizeID
define
\ifdef HYPHEN_HACK
   local
      fun {Loop S}
	 case S of nil then nil
	 [] H|T then
	    if H==&- then &/ else H end|{Loop T}
	 end
      end
   in
      fun {Dehyphenize S}
	 {Loop {VirtualString.toString S}}
      end
   end
   fun {HasHyphen ID}
      {Member &- {VirtualString.toString ID}}
   end
\endif
   %%
   ID_BAD_CHAR = {Regex.make '[^[:alnum:]:/_.-]'}
   %%
   %% {NormalizeID ID Base}
   %%	ID is interpreted as a URI relative to Base which must
   %% be the mogul URI of its section.  It is resolved and
   %% an atom is returned.
   %%
   fun {NormalizeID ID Base}
      if {Regex.search ID_BAD_CHAR ID}\=false then
	 {Raise mogul(id_syntax(ID))}
      end
      BaseU = {URL.toBase {URL.make if Base==unit then 'mogul:/' else Base end}}
\ifdef HYPHEN_HACK
      IdU = if {HasHyphen ID} then
	       {URL.resolve 'mogul:/' {Dehyphenize ID}}
	    elsecase BaseU.path of "mogul"|_ then
	       {URL.resolve 'mogul:/' ID}
	    else
	       {URL.resolve BaseU ID}
	    end
\else
      IdU = case BaseU.path of "mogul"|_ then
	       {URL.resolve 'mogul:/' ID}
	    else
	       {URL.resolve BaseU ID}
	    end
\endif
   in
      %% check the id is valid
      if IdU.scheme=="mogul" andthen
	 IdU.authority==unit andthen
	 BaseU.path\=IdU.path andthen
	 %% only mogul:/mogul/... sections are allowed
	 %% to point sideways.  All others must point
	 %% downwards.
	 ({List.isPrefix
	   {NoEmptyEnd BaseU.path}
	   {NoEmptyEnd IdU.path}}
	  orelse
	  case BaseU.path
	  of nil then
	     case IdU.path
	     of "mogul"|_ then true
	     else false end
	  [] [nil] then
	     case IdU.path
	     of "mogul"|_ then true
	     else false end
	  [] "mogul"|_ then true
	  else false end)
      then
	 case {Reverse {CondSelect IdU path nil}}
	 of nil|((_|_)=L) then {URL.toAtom {AdjoinAt IdU path {Reverse L}}}
	 else {URL.toAtom IdU} end
      else
	 raise mogul(id_prefix(want:{URL.toAtom BaseU}
			       got :{URL.toAtom IdU}))
	 end
      end
   end
   %%
   fun {NoEmptyEnd L}
      case {Reverse L} of nil|L then {Reverse L} else L end
   end
end
