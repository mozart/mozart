functor

export
   Make

import

   Aux(variableToVirtualString counterClass)
   Config(paramColour edgeColour eventColour)
   FS

define   

   IdCounter = {New Aux.counterClass init}
   
   fun {MakePropagatorEdge Hist PropTable Event PropId AllPs}
      P = PropTable.PropId
      LocP = P.location
      Location = if LocP == unit then ""
		 else LocP.file#":"#LocP.line
		 end
      Name = P.name
\ifdef SHOW_ID
      #" ["#PropId#"]"
\endif            
   in
      "l(\"e<"#{IdCounter next($)}
      #">\",e(\"\", [a(\"_DIR\",\"none\"), a(\"EDGECOLOR\",\""
      #Config.edgeColour
      #"\")],l(\"cn<"#PropId
      #">\",n(\"\",["
      #"a(\"OBJECT\",\""#Name#"\\n"#Location#"\"),"
      #"a(\"COLOR\",\""#{Hist get_prop_node_failed(P.reference $)}#"\"),"
      #{Hist get_prop_node_attr(PropId $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(PropId P.name#" ("#Location#")" $)}
      #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
      #",menu_entry(\"cg<sub>\",\"Constraint graph of constraints reachable by that variable\")"
      
      #if AllPs == nil then ""
       else
	  ",menu_entry(\"cg<"#AllPs.1
	  #{FoldL AllPs.2
	    fun {$ L R} if R == nil then L
			else L#'|'#R
			end
	    end ""}
	  #">\",\"Constraint graph of constraints reachable by that variable at event ["#Event#"]\")"
	  
       end

      #",menu_entry(\"scg<"#P.id
      #">\",\"Single Constraint graph of "#P.name
      #if LocP == unit then ""
       else " ("#LocP.file#":"#LocP.line#")"
       end 
      #"\")"


      #"])],[]))))"
   end
   
   fun {MakePropagatorEdges Hist PropTable Event Ps AllPs}
      case Ps
      of A|B|T then
	 {MakePropagatorEdge Hist PropTable Event A AllPs}#","
	 #{MakePropagatorEdges Hist PropTable Event B|T AllPs}
      [] A|nil then {MakePropagatorEdge Hist PropTable Event A AllPs}
      else "" end
   end
   
   fun {MakeEventEdge Hist PropTable V Event}
      Filtered = {FS.reflect.lowerBoundList V.susplists.Event}
      PropsOnEvent = {MakePropagatorEdges Hist PropTable Event Filtered Filtered}
   in
      "l(\"e<"#{IdCounter next($)}#">\",e(\"\",[a(\"_DIR\",\"none\"),"
      #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\")],l(\""#Event#"\","
      #"n(\"\",[a(\"OBJECT\",\""#Event#"\"),"
      #"a(\"COLOR\",\""#Config.eventColour#"\"),"

      #"m(["
      #{Hist insert_menu($)}
      #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
      #",menu_entry(\"cg<sub>\",\"Constraint graph of constraints reachable by that variable\")"

      #if Filtered == nil then ""
       else
	  ",menu_entry(\"cg<"#Filtered.1
	  #{FoldL Filtered.2
	    fun {$ L R} if R == nil then L
			else L#'|'#R
			end
	    end ""}
	  #">\",\"Constraint graph of constraints reachable by that variable at event ["#Event#"]\")"
	  
       end

      #"])],["#PropsOnEvent#"]))))"
   end
   
   fun {MakeEventEdges Hist PropTable V Events}
      case Events
      of A|B|T then
	 {MakeEventEdge Hist PropTable V A}#","
	 #{MakeEventEdges Hist PropTable V B|T}
      [] A|nil then
	 {MakeEventEdge Hist PropTable V A} else ""
      end
   end

   fun {Make VarTable PropTable Hist V}
      [VarId] = {FS.reflect.lowerBoundList V}
      ReflV = VarTable.VarId
      VarStr = {Aux.variableToVirtualString ReflV.reference}
\ifdef SHOW_ID
      #" ["#VarId#"]"
\endif            
   in
      {Hist reset_mark}

      svg(graph:
	     ("[l(\"vn<"#VarId#">\",n(\"\",[a(\"OBJECT\",\""#VarStr#"\"),"
	      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
	      #{Hist get_param_node_attr(VarId $)}
	      #"m(["
	      #{Hist insert_menu($)}
	      #{Hist insert_menu_mark_param(VarId VarStr $)}
	      #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
	      #",menu_entry(\"vg<solvar>\",\"Variable graph of solution variables\")"
	      #",menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
	      #",menu_entry(\"cg<sub>\",\"Constraint graph of constraints reachable by that variable\")])"
	      #"],["
	      #{MakeEventEdges Hist PropTable ReflV {Arity ReflV.susplists}}#"]))]")
	  )
   end
end
