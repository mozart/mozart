functor

export
   Make

import

   Aux(counterClass)
   Config(paramColour
          edgeColour
          eventColour
          paramNodeShape
          propNodeShape
          eventNodeShape)
   FS

define

   IdCounter = {New Aux.counterClass init}

   fun {MakePropagatorEdge
        Hist PropTable Event PropId AllPs MenuAllReachablePs}
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
      #"a(\"_GO\",\""#Config.propNodeShape#"\"),"
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(PropId P.name#" ("#Location#")" $)}
      #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
      #MenuAllReachablePs

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

   fun {MakePropagatorEdges Hist PropTable Event Ps AllPs MenuAllReachablePs}
      case Ps
      of A|B|T then
         {MakePropagatorEdge
          Hist PropTable Event A AllPs MenuAllReachablePs}#","
         #{MakePropagatorEdges
           Hist PropTable Event B|T AllPs MenuAllReachablePs}
      [] A|nil then
         {MakePropagatorEdge Hist PropTable Event A AllPs MenuAllReachablePs}
      else "" end
   end

   fun {MakeEventEdge Hist PropTable V Event MenuAllReachablePs}
      Filtered = {FS.reflect.lowerBoundList V.susplists.Event}
      PropsOnEvent = {MakePropagatorEdges Hist PropTable Event Filtered Filtered MenuAllReachablePs}
   in
      "l(\"e<"#{IdCounter next($)}#">\",e(\"\",[a(\"_DIR\",\"none\"),"
      #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\")],l(\""#Event#"\","
      #"n(\"\",[a(\"OBJECT\",\""#Event#"\"),"
      #"a(\"COLOR\",\""#Config.eventColour#"\"),"
      #"a(\"_GO\",\""#Config.eventNodeShape#"\"),"

      #"m(["
      #{Hist insert_menu($)}
      #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
      #MenuAllReachablePs

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

   fun {MakeEventEdges Hist PropTable V Events MenuAllReachablePs}
      case Events
      of A|B|T then
         {MakeEventEdge Hist PropTable V A MenuAllReachablePs}#","
         #{MakeEventEdges Hist PropTable V B|T MenuAllReachablePs}
      [] A|nil then
         {MakeEventEdge Hist PropTable V A MenuAllReachablePs} else ""
      end
   end

   fun {Make VarTable PropTable Hist V}
      [VarId] = {FS.reflect.lowerBoundList V}
      ReflV = VarTable.VarId
      VarStr = ReflV.nameconstraint
\ifdef SHOW_ID
      #" ["#VarId#"]"
\endif

      ReachablePs = {FS.reflect.lowerBoundList ReflV.propagators}
      MenuAllReachablePs =
      if ReachablePs == nil then ""
      else
         ",menu_entry(\"cg<"#ReachablePs.1
         #{FoldL ReachablePs.2
           fun {$ L R} if R == nil then L
                       else L#'|'#R
                       end
           end ""}
         #">\",\"Constraint graph of constraints reachable by that variable\")"
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
              #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
              #",menu_entry(\"vg<solvar>\",\"Variable graph of solution variables\")"
              #",menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
              #MenuAllReachablePs
              #"])"
              #"],["
              #{MakeEventEdges
                Hist PropTable ReflV {Arity ReflV.susplists} MenuAllReachablePs}
              #"]))]")
          )
   end
end
