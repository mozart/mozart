% this code uses still the quadratic version

functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   Make

import

   FS(value intersect reflect)
   Tables(getVarId getPropId getProp)
   Aux(propName mergeSuspLists1 variableToVirtualString varReflect
       propLocation)
   Config(paramColour edgeColour)
   
define
   
   fun {ShareProps PropTable Ps1 Ps2}
      S1 = {FS.value.make
	    {Map Ps1 fun {$ P} {Tables.getPropId PropTable P} end}}
      S2 = {FS.value.make
	    {Map Ps2 fun {$ P} {Tables.getPropId PropTable P} end}}
   in
      {FS.reflect.lowerBoundList {FS.intersect S1 S2}}
   end
   
   fun {MakeEdges Hist PropTable H T}
      if T == nil then ""
      else
	 SharedProps = {ShareProps PropTable H.p T.1.p}
      in
	 if SharedProps == nil then ""
	 else
	    "l(\"e<"#H.id#"|"#T.1.id#">\",e(\"\", [a(\"_DIR\",\"none\"),"
	    #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\"),"

	    #"m(["
	    #{Hist insert_menu($)}
	    #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
	    #",menu_entry(\"cg<"

	    #SharedProps.1
	    #{FoldL SharedProps.2 fun {$ L R} if R == nil then L
					  else L#'|'#R
					  end
			      end ""}
	    
	    #">\",\"Constraint graph of constraints imposed onto these two variables\")"
	    #{FoldL SharedProps
	      fun {$ L R}
		 Prop = {Tables.getProp PropTable R}
		 LocationProp  = {Aux.propLocation Prop}
		 Loc = if LocationProp == unit then noLoc
		      else loc(file:   LocationProp.file
			       path:   LocationProp.path
			       line:   LocationProp.line
			       column: LocationProp.column)
		      end
	      in
		 
		 L#",menu_entry(\"scg<"#R#">\",\"Constraint graph of "
		 #{Aux.propName Prop}
		 #if Loc == noLoc then ""
		  else " ("#Loc.file#":"#Loc.line#")"
		  end 
		 #"\")"
	      end ""}
	    
	    #"])], r(\"vn<"#T.1.id#">\")))"
	    #if T.2 == nil then "" else ","end
	 end
	 #{MakeEdges Hist PropTable H T.2}
      end
   end
   
   fun {MakeNode Hist PropTable H T}
      VarStr = {Aux.variableToVirtualString H.r}
   in
      "l(\"vn<"#H.id#">\",n(\"\",["
      #"a(\"OBJECT\",\""#VarStr#"\"),"
      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
      #{Hist get_param_node_attr(H.id $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_param(H.id VarStr $)}
      #"menu_entry(\"vg<all>\",\"Variabale graph of all variables\")"
      #",menu_entry(\"svg<"#H.id#">\",\"Single variable graph of "
      #VarStr#"\")"
      #"])],["
      #{MakeEdges Hist PropTable H T}#"]))"#if T == nil then "" else ","end
   end
   
   fun {MakeNodes Hist PropTable H T}
      {MakeNode Hist PropTable H T}
      #if T == nil then "" else {MakeNodes Hist PropTable T.1 T.2} end
   end

   fun {Make VarTable PropTable Hist Vs}
      {Hist reset_mark}

      vg(graph:
	    case {List.map Vs
		  fun {$ V}
		     ReflectedVar = {Aux.varReflect V}
		  in
		     v(id: {Tables.getVarId VarTable V}
		       r:  V
		       p:  {Aux.mergeSuspLists1 PropTable ReflectedVar}
		      )
		  end}
	    of H|T then "["#{MakeNodes Hist PropTable H T}#"]"
	    else "" end
	)
   end
end
