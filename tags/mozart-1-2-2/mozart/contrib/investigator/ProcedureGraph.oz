functor

   % this code is hacked. id >= 1000 are reserved for procedure. all
   % beneath are propagators.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   Make

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   FS
   System
   Config(procNodeShape propNodeShape procNodeColour edgeColour) 

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

	    #{FoldL SharedVars
	      fun {$ L R}
		 L#"menu_entry(\"svg<"#R#">\",\"Single variable graph of "
		 #VarTable.R.name#"\"),"
	      end ""}

	    #"menu_entry(\"vg<"
	    #SharedVars.1
	    #{FoldL SharedVars.2 fun {$ L R} if R == nil then L
					  else L#'|'#R
					  end
				 end ""}
	    #">\",\"Variables graph of all variables shared by these two propagators\")"
	    #",menu_entry(\"vg<all>\",\"Variables graph of all variables\")"
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
      #"a(\"OBJECT\",\""#if H.id < 1000 then Name#"\\n"#Location elseif Name == 'Toplevel abstraction' then 'proc {Hamilton}\\nhamil.oz:2' else "proc {"#Name#"}\\n"#Location end#"\"),"
      #"a(\"COLOR\",\""#Config.procNodeColour#"\"),"
      #{Hist get_prop_node_attr(H.id $)}
      #"a(\"_GO\",\""#if H.id < 1000 then Config.propNodeShape else Config.procNodeShape end#"\"),"
      #"m(["
      #{Hist insert_menu($)}
      #{Hist insert_menu_mark_prop(H.id H.name#" ("#Location#")" $)}
      
      #"menu_entry(\"addconcg<"#H.id#">\",\"Add #"
      #{FS.card H.if H.id >= 1000 then connected_procs else connected_props end}#" propagator nodes connected to "
      #H.name#" ("#Location#")\")"
      #",blank"
      
      #",menu_entry(\"scg<"#H.id#">\",\"Single propagator graph of "
      #H.name
      #if H.location == unit then ""
       else " ("#H.location.file#":"#H.location.line#")"
       end 
      #"\")"

      #",menu_entry(\"cg<all>\",\"Propagator graph all propagators\")"
      
      #",blank"
      #",menu_entry(\"corrvg\",\"Corresponding variable graph\")"
      #"])"
      #"],["
      #{MakeEdges Hist VarTable H 
	{FoldR
	 {FS.reflect.lowerBoundList
	  {FS.diff
	   if H.id >= 1000 then {FS.union H.connected_procs H.connected_props}
	   else {FS.union H.connected_props
		 {FS.unionN
		  {Map {FS.reflect.lowerBoundList H.connected_props}
		   fun {$ PropId}
		      {FS.value.make
		       {Map
			{FS.reflect.lowerBoundList PropTable.PropId.connected_props}
			fun {$ Prop}
			   PropTable.Prop.location.propInvoc.invoc
			end}
		      }
		   end
		  }
		 }
		}
	   end
	   Ignore}}
	 fun {$ L R} (PropTable.L)|R end nil}}
      #"]))"
   end
   
   fun {MakeNodes IgnoreIn Hist VarTable PropTable L}
      {System.printInfo '.'}
      if L == nil then ""
      else
	 Ignore = {FS.union {FS.value.make L.1.id} IgnoreIn}
      in
	 {MakeNode Ignore Hist VarTable PropTable L.1}#if L.2 == nil then "" else "," end
	 #{MakeNodes Ignore Hist VarTable PropTable L.2}
      end
   end

   fun {Make VarTable PropTable ProcTable Hist Ps}
      DirtyProcTable = {Adjoin PropTable ProcTable}
   in
      {Hist reset_mark} % tmueller ?
      
      cg(graph:
	    case {FoldR {FS.reflect.lowerBoundList Ps}
		  fun {$ L R} (DirtyProcTable.L)|R end nil}
	    of nil then ""
	    [] L then "["#{MakeNodes
			   {FS.diff {FS.value.make {Arity DirtyProcTable}}
			    Ps}
			   Hist VarTable DirtyProcTable L}#"]"

	    end
	)
   end
   
end
