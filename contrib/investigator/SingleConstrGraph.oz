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

   System

define

   IdCounter = {New Aux.counterClass init}

   fun {MakeParameterEdge Hist VarTable Event VarId EventPs AllParams}
      VarStr = VarTable.VarId.nameconstraint
\ifdef SHOW_ID
      #" ["#VarId#"]"
\endif
   in
      "l(\"e<"#{IdCounter next($)}#">\",e(\"\", ["
      #"a(\"_DIR\",\"none\"),"
      #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\")],l(\"vn<"
      #VarId#">\",n(\"\",["
      #"a(\"OBJECT\",\""#VarStr#"\"),"
      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
      #{Hist get_param_node_attr(VarId $)}
      #"a(\"_GO\",\""#Config.paramNodeShape#"\"),"
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_param(VarId VarStr $)}
      #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
      #",menu_entry(\"vg<"
      #AllParams.1
      #{FoldL AllParams.2
        fun {$ L R} if R == nil then L
                    else L#'|'#R
                    end
        end ""}
      #">\",\"Variable graph of all variables reachable by that constraint\")"

      #",menu_entry(\"vg<"

      #EventPs.1
      #{FoldL EventPs.2 fun {$ L R} if R == nil then L
                                    else L#'|'#R
                                    end
                        end ""}

      #">\",\"Variable graph of all variables reachable by that constraint at event ["#Event#"]\")"

      #",menu_entry(\"svg<"#VarId#">\",\"Single variable graph of "
      #VarStr#"\")])"

      #"],[]))))"
   end

   fun {MakeParameterEdges Hist VarTable Event Ps AllPs AllParams}
      case Ps
      of A|B|T then
         {MakeParameterEdge Hist VarTable Event A AllPs AllParams}#","
         #{MakeParameterEdges Hist VarTable Event B|T AllPs AllParams}
      [] A|nil then {MakeParameterEdge Hist VarTable Event A AllPs AllParams}
      else "" end
   end

   fun {MakeEventEdge Hist VarTable Event AllParams}
      "l(\"e<"#{IdCounter next($)}
      #">\",e(\"\", [a(\"_DIR\",\"none\"),a(\"EDGECOLOR\",\""#Config.edgeColour
      #"\")],l(\""#Event.1
      #"\",n(\"\",["
      #"a(\"OBJECT\",\""#Event.1#"\"),"
      #"a(\"COLOR\",\""#Config.eventColour#"\"),"
      #"a(\"_GO\",\""#Config.eventNodeShape#"\"),"
      #"m(["
      #{Hist insert_menu($)}
      #"menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
      #",menu_entry(\"vg<"
      #AllParams.1
      #{FoldL AllParams.2
        fun {$ L R} if R == nil then L
                    else L#'|'#R
                    end
        end ""}
      #">\",\"Variable graph of all variables reachable by that constraint\")"
      #",menu_entry(\"vg<"
      #Event.2.1
      #{FoldL Event.2.2 fun {$ L R} if R == nil then L
                                    else L#'|'#R
                                    end
                        end ""}

      #">\",\"Variable graph of all variables reachable by that constraint at event ["#Event.1#"]\")])"

      #"],["
      #{MakeParameterEdges Hist VarTable Event.1 Event.2 Event.2 AllParams}
      #"]))))"
   end

   fun {MakeEventEdges Hist VarTable Events AllParams}
      case Events
      of A|B|T then
         {MakeEventEdge Hist VarTable A AllParams}#","
         #{MakeEventEdges Hist VarTable B|T AllParams}
      [] A|nil then {MakeEventEdge Hist VarTable A AllParams}
      else "" end
   end

   fun {ConstraintEvents VarTable ReflC ReflParams}
      Params = {FoldR {FS.reflect.lowerBoundList ReflParams}
                fun {$ L R} (VarTable.L)|R end nil}

      Dict = {NewDictionary}

      proc {ConstraintEvents1 Params}
         case Params
         of H|T then Events = {Arity H.susplists} in
            {ForAll Events
             proc {$ Event}
                if {FS.isIn ReflC.id H.susplists.Event}
                then
                   Entry = {Dictionary.condGet Dict Event nil}
                in
                   {Dictionary.put Dict Event H.id|Entry}
                else skip end
             end}

            {ConstraintEvents1 T}
         else skip end
      end
   in
      {ConstraintEvents1 Params}
      {Dictionary.entries Dict}
   end

   fun {Make VarTable PropTable Hist C}
      [PropId] = {FS.reflect.lowerBoundList C}
      ReflC = PropTable.PropId
      LocC  = ReflC.location

      Location = if LocC == unit then "" else LocC.file#":"#LocC.line end

      ParamEvents = {ConstraintEvents VarTable ReflC ReflC.parameters}
      ParamsList = {FS.reflect.lowerBoundList ReflC.parameters}
      Name = ReflC.name
\ifdef SHOW_ID
      #" ["#PropId#"]"
\endif
   in
      {System.show g}
      {Hist reset_mark}

      scg(graph:
             ("[l(\"cn<"#PropId
              #">\",n(\"\",["
              #"a(\"OBJECT\",\""#Name#"\\n"#Location#"\"),"
              #"a(\"COLOR\",\""#{Hist get_prop_node_failed(ReflC.reference $)}
              #"\"),"
              #{Hist get_prop_node_attr(PropId $)}
              #"a(\"_GO\",\""#Config.propNodeShape#"\"),"

              #"m(["
              #{Hist insert_menu($)}
              #{Hist insert_menu_mark_prop(PropId
                                           ReflC.name#" ("#Location#")" $)}
              #"menu_entry(\"cg<all>\",\"Constraint graph of all constraints\")"
              #",menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
              #",menu_entry(\"vg<solvar>\",\"Variable graph of solution variables\")"
              #",menu_entry(\"vg<"
              #ParamsList.1
              #{FoldL ParamsList.2
                fun {$ L R} if R == nil then L
                            else L#'|'#R
                            end
                end ""}
              #">\",\"Variable graph of all variables reachable by that constraint\")])"

              #"],["
              #{MakeEventEdges Hist VarTable ParamEvents ParamsList}
              #"]))]")
         )
   end
end
