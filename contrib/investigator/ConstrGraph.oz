functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   Make

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   FS(value intersect reflect include var)
   System
   Tables(getVar getVarId makeVarTable getPropId makePropTable)
   Aux(propReflect varReflect vectorToList propLocation mergeSuspLists)
   Config(propColour edgeColour)

define

   fun {ShareVars VarTable Vs1 Vs2}
      {System.show shareVars}

      S1 = {FS.value.make
            {Map Vs1 fun {$ V} {Tables.getVarId VarTable V} end}}
      S2 = {FS.value.make
            {Map Vs2 fun {$ V} {Tables.getVarId VarTable V} end}}
   in
      {FS.reflect.lowerBoundList {FS.intersect S1 S2}}
   end

   fun {MakeEdges Hist VarTable H T}
      {System.show makeEdges}

      if T == nil then ""
      else
         SharedVars = {ShareVars VarTable H.p T.1.p}
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
                 #{System.printName {Tables.getVar VarTable R}}#"\")"
              end ""}
            #"])], r(\"cn<"
            #T.1.id#">\")))"
            #if T.2 == nil then "" else ","end
         end
         #{MakeEdges Hist VarTable H T.2}
      end
   end

   fun {MakeNode Hist VarTable H}
      {System.show makeNode}

      "l(\"cn<"#H.id#">\",n(\"\",[a(\"OBJECT\",\""#H.n
      #if H.loc == noLoc then ""
       else "\\n"#H.loc.file#":"#H.loc.line
       end
      #"\"),a(\"COLOR\",\""#Config.propColour#"\"),"

      #"m(["
      #{Hist insert_menu($)}
      #"menu_entry(\"cg<all>\",\"Constraint graph all constraints\")"
      #",menu_entry(\"scg<"#H.id#">\",\"Single constraint graph of "
      #H.n#"\")])"
      #"],["
      #{MakeEdges Hist VarTable H H.ps}#"]))"
   end

   fun {MakeNodes Hist VarTable L}
      {System.show makeNodes}

      if L == nil then ""
      else
         {MakeNode Hist VarTable L.1}
         #","
         #{MakeNodes Hist VarTable L.2}
      end
   end

   fun {Make Hist Ps}
      {System.show make}

      VarTable  = {Tables.makeVarTable}
      PropTable = {Tables.makePropTable}
      Ignore = {FS.var.decl}
   in
      cg(graph:
            case {Map Ps
                  fun {$ P}
                     ReflectedProp = {Aux.propReflect P}
                     LocationProp  = {Aux.propLocation P}
                     AllParamsOfP  = {Aux.vectorToList ReflectedProp.params}
                     Id = {Tables.getPropId PropTable P}
                     {FS.include Id Ignore}
                  in
                     p(id: Id
                       p:  AllParamsOfP
                       ps: {Map {Aux.mergeSuspLists PropTable
                                 {FS.reflect.lowerBound Ignore}
                                 {Map AllParamsOfP Aux.varReflect}}
                            fun {$ P}
                               ReflectedProp = {Aux.propReflect P}
                            in
                               p(id: {Tables.getPropId PropTable P}
                                 p:  {Aux.vectorToList ReflectedProp.params}
                                 n:  ReflectedProp.name)
                            end}
                       n:   ReflectedProp.name
                       loc: if LocationProp == unit then noLoc
                            else loc(file:   LocationProp.file
                                     path:   LocationProp.path
                                     line:   LocationProp.line
                                     column: LocationProp.column)
                            end
                      )
                  end}
            of nil then ""
            [] L then "["#{MakeNodes Hist VarTable L}#"]"
            end
         varTable:
            VarTable
         propTable:
            PropTable
        )
   end

end
