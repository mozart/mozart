functor

export
   Make

import

   Misc(counterClass)
   Config(paramColour
	  edgeColour
	  eventColour
	  paramNodeShape
	  propNodeShape
	  eventNodeShape)
   FS

\ifdef DEBUG
   System
\endif
   
define   

   IdCounter = {New Misc.counterClass init}
   
   fun {MakePropagatorEdge FailSet
	Hist PropTable Event PropId AllPs MenuAllReachablePs} 
\ifdef DEBUG
      {System.show makePropagatorEdge}
\endif
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
      #"\")],l(\"cn<"#PropId#'|'#{IdCounter next($)} %PropId
      #">\",n(\"\",["
      #"a(\"OBJECT\",\""#Name#"\\n"#Location#"\"),"
      #"a(\"COLOR\",\""#{Hist get_prop_node_failed(P.reference FailSet PropId $)}#"\"),"
      #{Hist get_prop_node_attr(PropId $)}
      #"a(\"_GO\",\""#Config.propNodeShape#"\"),"
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(PropId P.name#" ("#Location#")" $)}

      #"menu_entry(\"scg<"#P.id
      #">\",\"Single propagator graph of "#P.name
      #if LocP == unit then ""
       else " ("#LocP.file#":"#LocP.line#")"
       end 
      #"\")"

      #if MenuAllReachablePs \= "" then ","#MenuAllReachablePs else "" end
      
      #if AllPs == nil then ""
       else
	  ",menu_entry(\"cg<"#AllPs.1
	  #{FoldL AllPs.2
	    fun {$ L R} if R == nil then L
			else L#'|'#R
			end
	    end ""}
	  #">\",\"Propagator graph of propagators reachable by that variable at event ["#Event#"]\")"
	  
       end

      #",menu_entry(\"cg<all>\",\"Propagator graph of all propagators\")"

      #"])],[]))))"
   end
   
   fun {MakePropagatorEdges FailSet Hist PropTable Event Ps AllPs MenuAllReachablePs}
\ifdef DEBUG
      {System.show makePropagatorEdges}
\endif
      case Ps
      of A|B|T then
	 {MakePropagatorEdge FailSet
	  Hist PropTable Event A AllPs MenuAllReachablePs}#","
	 #{MakePropagatorEdges FailSet
	   Hist PropTable Event B|T AllPs MenuAllReachablePs}
      [] A|nil then
	 {MakePropagatorEdge FailSet Hist PropTable Event A AllPs MenuAllReachablePs}
      else "" end
   end
   
   fun {MakeEventEdge FailSet Hist PropTable V Event MenuAllReachablePs}
\ifdef DEBUG
      {System.show makeEventEdge}
\endif
      Filtered = {FS.reflect.lowerBoundList V.susplists.Event}
      PropsOnEvent = {MakePropagatorEdges FailSet Hist PropTable Event Filtered Filtered MenuAllReachablePs}
   in
      "l(\"e<"#{IdCounter next($)}#">\",e(\"\",[a(\"_DIR\",\"none\"),"
      #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\")],l(\""#Event#"\","
      #"n(\"\",[a(\"OBJECT\",\""#Event#"\"),"
      #"a(\"COLOR\",\""#Config.eventColour#"\"),"
      #"a(\"_GO\",\""#Config.eventNodeShape#"\"),"

      #"m(["
      #{Hist insert_menu($)}
      #if Filtered == nil then ""
       else
	  "menu_entry(\"cg<"#Filtered.1
	  #{FoldL Filtered.2
	    fun {$ L R} if R == nil then L
			else L#'|'#R
			end
	    end ""}
	  #">\",\"Propagator graph of propagators reachable by that variable at event ["#Event#"]\"),"
       end

      #if MenuAllReachablePs \= "" then MenuAllReachablePs#"," else "" end
      #"menu_entry(\"cg<all>\",\"Propagator graph of all propagators\")"


      #"])],["#PropsOnEvent#"]))))"
   end
   
   fun {MakeEventEdges FailSet Hist PropTable V Events MenuAllReachablePs}
\ifdef DEBUG
      {System.show makeEventEdges}
\endif
      case Events
      of A|B|T then
	 {MakeEventEdge FailSet Hist PropTable V A MenuAllReachablePs}#","
	 #{MakeEventEdges FailSet Hist PropTable V B|T MenuAllReachablePs}
      [] A|nil then
	 {MakeEventEdge FailSet Hist PropTable V A MenuAllReachablePs} else ""
      end
   end

   fun {Make FailSet VarTable PropTable Hist V}
      [VarId] = {FS.reflect.lowerBoundList V}
      ReflV = VarTable.VarId
      VarStr = {Hist get_sol_var(VarId $)}
      #ReflV.nameconstraint
\ifdef SHOW_ID
      #" ["#VarId#"]"
\endif            

      ReachablePs = {FS.reflect.lowerBoundList ReflV.propagators}
      MenuAllReachablePs =
      if ReachablePs == nil then ""
      else
	 "menu_entry(\"cg<"#ReachablePs.1
	 #{FoldL ReachablePs.2
	   fun {$ L R} if R == nil then L
		       else L#'|'#R
		       end
	   end ""}
	 #">\",\"Propagator graph of propagators reachable by that variable\")"
      end
   in
      {Hist reset_mark}

      svg(graph:
	     ("[l(\"vn<"#VarId#">\",n(\"\",[a(\"OBJECT\",\""#VarStr#"\"),"
	      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
	      #{Hist get_param_node_attr(VarId $)}
	      #"a(\"_GO\",\""#Config.paramNodeShape#"\"),"
	      #"m(["
	      #{Hist insert_menu($)}
	      #{Hist insert_menu_mark_param(VarId VarStr $)}
	      #"menu_entry(\"vg<solvar>\",\"Variable graph of only root variables\")"
	      #",menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
	      #",blank"
	      #if MenuAllReachablePs \= "" then ","#MenuAllReachablePs else "" end 
	      #",menu_entry(\"cg<all>\",\"Propagator graph of all propagators\")"
	      #"])"
	      #"],["
	      #{MakeEventEdges FailSet
		Hist PropTable ReflV {Arity ReflV.susplists} MenuAllReachablePs}
	      #"]))]")
	  )
   end
end
