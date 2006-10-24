functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   Make

import

   FS
   Config(paramColour paramNodeShape edgeColour)

\ifdef DEBUG
   System
\endif
   
define
   
   fun {ShareProps PropTable S1 S2}
\ifdef DEBUG
      {System.show shareProps}
\endif
      {FS.reflect.lowerBoundList {FS.intersect S1 S2}}
   end
   
   fun {MakeEdges FailSet Hist PropTable H T}
\ifdef DEBUG
      {System.show makeEdges}
\endif
      if T == nil then ""
      else
	 SharedProps = {ShareProps PropTable H.propagators T.1.propagators}
      in
	 if SharedProps == nil then ""
	 else
	    "l(\"e<"#H.id#"|"#T.1.id#">\",e(\"\", [a(\"_DIR\",\"none\"),"
%	    #"a(\"EDGEPATTERN\",\"thick\"),"
	    #"a(\"EDGECOLOR\",\""#{Hist get_prop_edge_failed(FailSet SharedProps $)}#"\"),"
	    
	    #"m(["
	    #{Hist insert_menu($)}
	    #{FoldL SharedProps
	      fun {$ L R}
		 Location = (PropTable.R).location
	      in
		 L#"menu_entry(\"scg<"#R#">\",\"Propagator graph of "
		 #(PropTable.R).name
		 #if Location == unit then ""
		  else " ("#Location.file#":"#Location.line#")"
		  end 
		 #"\"),"
	      end ""}

	    #"menu_entry(\"cg<"

	    #SharedProps.1
	    #{FoldL SharedProps.2 fun {$ L R} if R == nil then L
					      else L#'|'#R
					      end
				  end ""}
	    
	    #">\",\"Propagator graph of propagators imposed onto these two variables\")"
	    #",menu_entry(\"cg<all>\",\"Propagator graph of all propagators\")"
	    
	    #"])], r(\"vn<"#T.1.id#">\")))"
	    #if T.2 == nil then "" else ","end
	 end
	 #{MakeEdges FailSet Hist PropTable H T.2}
      end
   end
   
   fun {MakeNode FailSet Ignore Hist VarTable PropTable H}
\ifdef DEBUG
      {System.show makeNode}
\endif
      VarStr = {Hist get_sol_var(H.id $)}
      #H.nameconstraint
\ifdef SHOW_ID
      #" ["#H.id#"]"
\endif
   in
      "l(\"vn<"#H.id#">\",n(\"\",["
      #"a(\"OBJECT\",\""#VarStr#"\"),"
      #"a(\"COLOR\",\""#Config.paramColour#"\"),"
      #"a(\"_GO\",\""#Config.paramNodeShape#"\"),"
      #{Hist get_param_node_attr(H.id $)}
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_param(H.id VarStr $)}

      #"menu_entry(\"addconvg<"#H.id#">\",\"Add #"
      #{FS.card H.connected_vars}#" variable nodes connected to "
      #VarStr#"\")"

      #",blank"      
      #",menu_entry(\"svg<"#H.id#">\",\"Single variable graph of "#VarStr#"\")"
      #",menu_entry(\"vg<solvar>\",\"Variable graph only of root variables\")"
      #",menu_entry(\"vg<all>\",\"Variable graph of all variables\")"
      #",blank"
      #",menu_entry(\"corrcg\",\"Corresponding propagator graph\")"
      #"])],["
      #{MakeEdges FailSet Hist PropTable H
	{FoldR {FS.reflect.lowerBoundList {FS.diff H.connected_vars Ignore}}
	 fun {$ L R} (VarTable.L)|R end nil}}#"]))"
   end
   
   fun {MakeNodes FailSet IgnoreIn Hist VarTable PropTable L}
\ifdef DEBUG
      {System.show makeNodes}
\endif
      if L == nil then ""
      else
	 Ignore = {FS.union {FS.value.make L.1.id} IgnoreIn}
      in
	 {MakeNode FailSet Ignore Hist VarTable PropTable L.1}#","
	 #{MakeNodes FailSet Ignore Hist VarTable PropTable L.2} 
      end
   end
   
   fun {Make FailSet VarTable PropTable Hist Vs}
      {Hist reset_mark}

      vg(graph:
	    case {FoldR {FS.reflect.lowerBoundList Vs}
		  fun {$ L R} (VarTable.L)|R end nil}

	    of nil then ""
	    [] L   then
	       "["#{MakeNodes FailSet
		    {FS.diff {FS.value.make 1#{Width VarTable}} Vs}
		    Hist VarTable PropTable L}#"]"
	    end
	)
   end
end
