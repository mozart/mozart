functor

export
   Make

import

   Tables(getVarId getPropId)
   Aux(propReflect variableToVirtualString propLocation
       varReflect counterClass vectorToList memberEqProp)
   Config(paramColour edgeColour eventColour)

define

   IdCounter = {New Aux.counterClass init}

   fun {MakeParameterEdge Hist VarTable Event P EventPs}
      VarStr = {Aux.variableToVirtualString P}
      VarId = {Tables.getVarId VarTable P}
   in
      "l(\"e<"#{IdCounter next($)}#">\",e(\"\", ["
      #"a(\"_DIR\",\"none\"),"
      #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\")],l(\"vn<"
      #VarId#">\",n(\"\",["
      #"a(\"OBJECT\",\""#VarStr#"\"),"
      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
      #{Hist get_param_node_attr(VarId $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_param(VarId VarStr $)}
      #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
      #",menu_entry(\"vg<sub>\",\"Variable graph of all variables reachable by that constraint\")"

      #",menu_entry(\"vg<"

      #{Tables.getVarId VarTable EventPs.1}
      #{FoldL EventPs.2 fun {$ L R} if R == nil then L
                                    else L#'|'#{Tables.getVarId VarTable R}
                                    end
                        end ""}

      #">\",\"Variable graph of all variables reachable by that constraint at event ["#Event#"]\")"

      #",menu_entry(\"svg<"#{Tables.getVarId VarTable P}#">\",\"Single variable graph of "#VarStr#"\")])"

      #"],[]))))"
   end

   fun {MakeParameterEdges Hist VarTable Event Ps AllPs}
      case Ps
      of A|B|T then
         {MakeParameterEdge Hist VarTable Event A AllPs}#","
         #{MakeParameterEdges Hist VarTable Event B|T AllPs}
      [] A|nil then {MakeParameterEdge Hist VarTable Event A AllPs}
      else "" end
   end

   fun {MakeEventEdge Hist VarTable C Event}
      "l(\"e<"#{IdCounter next($)}
      #">\",e(\"\", [a(\"_DIR\",\"none\"),a(\"EDGECOLOR\",\""#Config.edgeColour
      #"\")],l(\""#Event.1
      #"\",n(\"\",[a(\"OBJECT\",\""
      #Event.1#"\"),a(\"COLOR\",\""#Config.eventColour#"\"),"

      #"m(["
      #{Hist insert_menu($)}
      #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
      #",menu_entry(\"vg<sub>\",\"Variable graph of all variables reachable by that constraint\")"
      #",menu_entry(\"vg<"
      #{Tables.getVarId VarTable Event.2.1}
      #{FoldL Event.2.2 fun {$ L R} if R == nil then L
                                    else L#'|'#{Tables.getVarId VarTable R}
                                    end
                        end ""}

      #">\",\"Variable graph of all variables reachable by that constraint at event ["#Event.1#"]\")])"

      #"],["
      #{MakeParameterEdges Hist VarTable Event.1 Event.2 Event.2}#"]))))"
   end

   fun {MakeEventEdges Hist VarTable C Events}
      case Events
      of A|B|T then
         {MakeEventEdge Hist VarTable C A}#","
         #{MakeEventEdges Hist VarTable C B|T}
      [] A|nil then {MakeEventEdge Hist VarTable C A}
      else "" end
   end

   proc {ConstraintEvents Dict ReflC Params}
      case Params
      of H|T then Events = {Arity H.susplists} in
         {ForAll Events
          proc {$ E}
             if  {Aux.memberEqProp ReflC.reference
                   {Map H.susplists.E
                    fun {$ C}
                       if C.type == propagator
                       then C.reference else unit end
                    end}}
             then
                Entry = {Dictionary.condGet Dict E nil}
             in
                {Dictionary.put Dict E H.var|Entry}
             else skip end
          end}
         {ConstraintEvents Dict ReflC T}
      else skip end
   end

   fun {Make VarTable PropTable Hist [C]}
      ReflC = {Aux.propReflect C}
      LocC  = {Aux.propLocation C}
      ReflParams = {Map {Filter {Aux.vectorToList ReflC.params}
                         fun {$ V} {Not {IsDet V}} end} Aux.varReflect}
      Dict = {NewDictionary}
      ParamEvents

      PropId = {Tables.getPropId PropTable C}
      Location = if LocC == unit then "" else LocC.file#":"#LocC.line end

   in
      {ConstraintEvents Dict ReflC ReflParams}
      ParamEvents = {Dictionary.entries Dict}
      {Hist reset_mark}

      scg(graph:
             ("[l(\"cn<"#PropId
              #">\",n(\"\",["
              #"a(\"OBJECT\",\""#ReflC.name#"\\n"#Location#"\"),"
              #"a(\"COLOR\",\""#{Hist get_prop_node_failed(PropId $)}#"\"),"
              #{Hist get_prop_node_attr(PropId $)}

              #"m(["
              #{Hist insert_menu($)}
              #{Hist insert_menu_mark_prop(PropId
                                           ReflC.name#" ("#Location#")" $)}
              #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
              #",menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
              #",menu_entry(\"vg<sub>\",\"Variable graph of all variables reachable by that constraint\")])"

              #"],["
              #{MakeEventEdges Hist VarTable ReflC ParamEvents}#"]))]")
         )
   end
end
