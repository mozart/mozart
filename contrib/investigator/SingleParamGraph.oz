functor

export
   Make

import

   Tables(getVarId getPropId)
   Aux(variableToVirtualString varReflect counterClass propLocation)
   Config(paramColour edgeColour eventColour)

define

   IdCounter = {New Aux.counterClass init}

   fun {MakePropagatorEdge Hist PropTable Event P AllPs}
      LocP = {Aux.propLocation P.reference}
      PropId = {Tables.getPropId PropTable P.reference}
      Location = if LocP == unit then ""
                 else LocP.file#":"#LocP.line
                 end
   in
      "l(\"e<"#{IdCounter next($)}
      #">\",e(\"\", [a(\"_DIR\",\"none\"), a(\"EDGECOLOR\",\""
      #Config.edgeColour
      #"\")],l(\"cn<"#PropId
      #">\",n(\"\",["
      #"a(\"OBJECT\",\""#P.name#"\\n"#Location#"\"),"
      #"a(\"COLOR\",\""#{Hist get_prop_node_failed(P.reference $)}#"\"),"
      #{Hist get_prop_node_attr(PropId $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(PropId P.name#" ("#Location#")" $)}
      #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
      #",menu_entry(\"cg<sub>\",\"Constraint graph of constraints reachable by that variable\")"

      #if AllPs == nil then ""
       else
          ",menu_entry(\"cg<"#{Tables.getPropId PropTable AllPs.1.reference}
          #{FoldL AllPs.2
            fun {$ L R} if R == nil then L
                        else L#'|'#{Tables.getPropId PropTable R.reference}
                        end
            end ""}
          #">\",\"Constraint graph of constraints reachable by that variable at event ["#Event#"]\")"

       end

      #",menu_entry(\"scg<"#{Tables.getPropId PropTable P.reference}
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
      Filtered = {Filter V.susplists.Event
                       fun {$ S} S.type == propagator end}
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
          ",menu_entry(\"cg<"#{Tables.getPropId PropTable Filtered.1.reference}
          #{FoldL Filtered.2
            fun {$ L R} if R == nil then L
                        else L#'|'#{Tables.getPropId PropTable R.reference}
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

   fun {Make VarTable PropTable Hist [V]}
      ReflV = {Aux.varReflect V}
      VarId = {Tables.getVarId VarTable V}
      VarStr = {Aux.variableToVirtualString ReflV.var}
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
              #",menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
              #",menu_entry(\"cg<sub>\",\"Constraint graph of constraints reachable by that variable\")])"
              #"],["
              #{MakeEventEdges Hist PropTable ReflV {Arity ReflV.susplists}}#"]))]")
          )
   end
end
