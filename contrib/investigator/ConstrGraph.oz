functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   Make

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   FS
   System
   Config(edgeColour)

define

   fun {ShareVars S1 S2}
\ifdef DEBUG
      {System.show shareVars}
\endif
      {FS.reflect.lowerBoundList {FS.intersect S1 S2}}
   end

   fun {MakeEdges Hist VarTable H T}
\ifdef DEBUG
      {System.show makeEdges}
\endif

      if T == nil then ""
      else
         SharedVars = {ShareVars H.parameters T.1.parameters}
      in
         if SharedVars == nil then ""
         else
            "l(\"e<"#H.id#"|"#T.1.id#">\",e(\"\", [a(\"_DIR\",\"none\"),"
            #"a(\"EDGECOLOR\",\""#Config.edgeColour#"\"),"
            #"m(["
            #{Hist insert_menu($)}
            #"menu_entry(\"vg<all>\",\"Variables graph of all variables\")"
            #",menu_entry(\"vg<"
            #SharedVars.1
            #{FoldL SharedVars.2 fun {$ L R} if R == nil then L
                                          else L#'|'#R
                                          end
                                 end ""}
            #">\",\"Variables graph of all variables shared by these two constraints\")"
            #{FoldL SharedVars
              fun {$ L R}
                 L#",menu_entry(\"svg<"#R#">\",\"Single variable graph of "
                 #VarTable.R.name#"\")"
              end ""}
            #"])], r(\"cn<"
            #T.1.id#">\")))"
            #if T.2 == nil then "" else ","end
         end
         #{MakeEdges Hist VarTable H T.2}
      end
   end

   fun {MakeNode Ignore Hist VarTable PropTable H}
\ifdef DEBUG
      {System.show makeNode}
\endif
      Location = if H.location == unit then ""
                 else H.location.file#":"#H.location.line
                 end

      Name = H.name
\ifdef SHOW_ID
      #" ["#H.id#"]"
\endif
   in
      "l(\"cn<"#H.id#">\",n(\"\",["
      #"a(\"OBJECT\",\""#Name#"\\n"#Location#"\"),"
      #"a(\"COLOR\",\""#{Hist get_prop_node_failed(H.reference $)}#"\"),"
      #{Hist get_prop_node_attr(H.id $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(H.id H.name#" ("#Location#")" $)}
      #"menu_entry(\"cg<all>\",\"Constraint graph all constraints\")"
      #",menu_entry(\"corrvg\",\"Corresponding variable graph\")"
      #",menu_entry(\"addconcg<"#H.id#">\",\"Add constraints #"
      #{FS.card H.connected_props}#" connected to "
      #H.name#" ("#Location#")\")"
      #",menu_entry(\"scg<"#H.id#">\",\"Single constraint graph of "
      #H.name
      #if H.location == unit then ""
       else " ("#H.location.file#":"#H.location.line#")"
       end
      #"\")])"
      #"],["
      #{MakeEdges Hist VarTable H
        {FoldR {FS.reflect.lowerBoundList {FS.diff H.connected_props Ignore}}
         fun {$ L R} (PropTable.L)|R end nil}}#"]))"
   end

   fun {MakeNodes IgnoreIn Hist VarTable PropTable L}
      {System.printInfo '.'}
      if L == nil then ""
      else
         Ignore = {FS.union {FS.value.make L.1.id} IgnoreIn}
      in
         {MakeNode Ignore Hist VarTable PropTable L.1}#","
         #{MakeNodes Ignore Hist VarTable PropTable L.2}
      end
   end

   fun {Make VarTable PropTable Hist Ps}
      {Hist reset_mark} % tmueller ?

      cg(graph:
            case {FoldR {FS.reflect.lowerBoundList Ps}
                  fun {$ L R} (PropTable.L)|R end nil}
            of nil then ""
            [] L then "["#{MakeNodes
                           {FS.diff {FS.value.make 1#{Width PropTable}} Ps}
                           Hist VarTable PropTable L}#"]"
            end
        )
   end

end
