functor
   
import
   QUI
   QHTMLDevel(defineWidget:DefineWidget
	      baseClass:BaseClass
	      configure:Configure
	      set:Set
	      get:Get
	      join:Join
	      quoteDouble:QuoteDouble
	      undefined:Undefined
	      checkType:CheckType
	      qInit:QInit
	      buildInnerHTML:BuildInnerHTML)
   QHTMLType(translate2HTML:Translate2HTML)

define
   
   {DefineWidget
    r(desc:img
      tag:img
      params:[alt#r(type:html
		    init:outer(true)
		    set:outer(true)
		    get:internal(true))
	      border#r(type:int
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      dynsrc#r(type:html
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      map#r(type:no
		    init:custom(true)
		    set:custom(true)
		    get:internal(true))
	      usemap#r(type:vs
		       init:outer(false)
		       set:outer(false)
		       get:internal(false))
	      longdesc#r(type:html
			 init:outer(true)
			 set:outer(true)
			 get:internal(true))
	      loop#r(type:int
		     init:outer(true)
		     set:outer(true)
		     get:internal(true))
	      lowsrc#r(type:html
		       init:outer(true)
		       set:outer(true)
		       get:internal(true))
	      src#r(type:html
		    init:outer(true)
		    set:outer(true)
		    get:internal(true))
	      start#r(type:[fileopen mouseover]
		      unmarshall:String.toAtom
		      init:outer(true)
		      set:outer(true)
		      get:internal(true))]
      events:nil
     )
    class $ from BaseClass
       meth !QInit(...)=M
	  {self {Record.adjoin M set}}
       end
       meth set(...)=M
	  {Record.forAllInd M
	   proc{$ I V}
	      case I of map then
		 if V==nil then
		    {self Set(usemap Undefined)}
		 else
		    {self Set(usemap "#m"#{self Get(id $)})}
		 end
		 {CheckType V list M}
		 {ForAll V
		  proc{$ R}
		     proc{Test S C H T}
			proc{Check I}
			   {CheckType C lint M}
			   if (I==inf andthen {Length C}>2)
			      orelse (I=={Length C}) then
			      skip
			   else
			      {QUI.raiseError V "Invalid number of coords for shape : "#S M}
			   end
			end
		     in
			case S
			of circle then {Check 3}
			[] polygon then {Check inf}
			[] rectangle then {Check 4}
			else
			   {QUI.raiseError V "Invalid shape type, expecting circle,polygon or rectangle : "#S M}
			end
			{CheckType H vs M}
			{CheckType T ['_parent' '_self' '_top' '_blank'] M}
		     end
		  in
		     case R
		     of area(shape:S
			     coords:C
			     href:H) then
			{Test S C H '_blank'}
		     [] area(shape:S
			     coords:C
			     href:H
			     target:T) then
			{Test S C H T}
		     else
			{QUI.raiseError V "Invalid type, expecting a list of records area(shape:Atom coords:ListOfIntegers href:VS)" M}
		     end
		  end}
		 {self Configure(innerHTML
				 {QuoteDouble '"'#{self BuildInnerHTML($)}#'"'}
				)}
	      end
	   end}
       end
       meth !BuildInnerHTML($)
	  if {self Get(map $)}\=Undefined then
	     "<MAP name='m"#{self Get(id $)}#"'>"#
	     {Join
	      {List.map
	       {self Get(map $)}
	       fun{$ R}
		  "<area shape='"#R.shape#"' coords='"#{Join R.coords ","}#"' href="#{Translate2HTML R.href html}#" target="#{CondSelect R target '_blank'}#">"
	       end} ""}
	  else "" end
       end
    end}

end
   
      

   
