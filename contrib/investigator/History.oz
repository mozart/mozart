functor

import

   Config(propColour:             PropColour
	  failedPropColour:       FailedPropColour
	  failedEdgeColour:       FailedEdgeColour
	  edgeColour:             EdgeColour
	  markedPropNodeAttr:     MarkedPropNodeAttr
	  markedParamNodeAttr:    MarkedParamNodeAttr
	  unMarkedPropNodeAttr:   UnMarkedPropNodeAttr
	  unMarkedParamNodeAttr:  UnMarkedParamNodeAttr
	 )

   Misc(propIsFailed: PropIsFailed)
   FS
   
export

   HistoryClass

   
define

   class HistoryClass

      attr
	 last: ~1
	 curr: ~1
	 hist: {Dictionary.new}

	 markedProp: unit
	 markedParam: unit
	 failedProp: unit

	 updateMarkedProp:  false
	 updateMarkedParam: false

	 sol_vars: {FS.value.make nil}
	 
      meth init
	 skip
      end

      meth set_sol_vars(SolVars)
	 sol_vars <- SolVars
      end

      meth get_sol_var(Id VS)
\ifdef IGNORE_REFERENCE
	 VS = ''
\else
	 VS = if {FS.isIn Id @sol_vars} then '* ' else '' end
\endif
      end

      meth get_sol_var_set(S)
	 S = @sol_vars
      end

%%%
%%% Markup mechanism
%%%

      meth reset_mark
	 updateMarkedProp  <- false
	 updateMarkedParam <- false
      end
      
      meth markup_prop(DaVin I)
	 markedProp <- I
	 updateMarkedProp <- true
	 {DaVin sendVS("graph(change_attr([node(\"cn<"#I#">\",[a(\"BORDER\",\""#MarkedPropNodeAttr#"\")])]))")}
      end

      meth get_mark_prop(I)
	 I = if @updateMarkedProp then @markedProp else unit end
      end
      
      meth unmark_prop
	 markedProp <- unit
      end

      meth get_prop_node_attr(I C)
	 C = "a(\"BORDER\",\""#if @markedProp == I
			       then updateMarkedProp <-  true
				  MarkedPropNodeAttr
			       else UnMarkedPropNodeAttr end#"\"),"
      end

      meth insert_menu_mark_prop(Id Str M)
	 M = "menu_entry(\"markprop<"#Id#">\",\"Mark "#Str#"\"),"
	 #"menu_entry(\"unmarkprop<"#Id#">\",\"Unmark\"),"
	 #"blank,"
      end

      meth mark_failed_prop(I)
	 failedProp <- I
      end
      
      meth unmark_failed_prop
	 failedProp <- unit
      end
	 
      meth get_prop_node_failed(PReference FailSet Id C)
\ifdef IGNORE_REFERENCE
	 C = PropColour
\else
	 C = if {PropIsFailed PReference} orelse {FS.isIn Id FailSet}
	     then FailedPropColour
	     else PropColour end
\endif
      end

      meth get_prop_edge_failed(FailSet SharedPropsList C)
\ifdef IGNORE_REFERENCE
	 C = PropColour
\else
	 C = if {FS.intersect {FS.value.make SharedPropsList} FailSet}
		== FS.value.empty
	     then EdgeColour
	     else FailedEdgeColour#"\"),a(\"EDGEPATTERN\",\"thick"
	     end
\endif
      end

      meth markup_param(DaVin I)
	 markedParam <- I
	 updateMarkedParam <- true
	 {DaVin sendVS("graph(change_attr([node(\"vn<"#I
		       #">\",[a(\"BORDER\",\""#MarkedParamNodeAttr#"\")])]))")}
      end

      meth get_mark_param(I)
	 I = if @updateMarkedParam then @markedParam else unit end
      end

      meth unmark_param
	 markedParam <- unit
      end
      
      meth get_param_node_attr(I C)
	 C = "a(\"BORDER\",\""#if @markedParam == I
			       then updateMarkedParam <-  true
				  MarkedParamNodeAttr
			       else UnMarkedParamNodeAttr end#"\"),"
      end

      
      meth insert_menu_mark_param(Id Str M)
	 M = "menu_entry(\"markparam<"#Id#">\",\"Mark "#Str#"\"),"
	 #"menu_entry(\"unmarkparam<"#Id#">\",\"Unmark\"),"
	 #"blank,"
      end

%%%
%%% History mechnism
%%%
      
      meth is_prev(B)
	 B = (@curr > 0)
      end

      meth is_next(B)
	 B = (@curr < @last)
      end

      meth get_prev_action(Action Args)
	 curr <- @curr - 1
	 Action#Args = {Dictionary.get @hist @curr}	 
      end

      meth get_curr_action(Action Args)
	 Action#Args = {Dictionary.get @hist @curr}
      end
      
      meth get_next_action(Action Args)
	 curr <- @curr + 1
	 Action#Args = {Dictionary.get @hist @curr}
      end

      meth add_action(Action Args)
	 OldLast = @last
      in
	 curr <- @curr + 1
	 
	 last <- @curr
	 {Dictionary.put @hist @curr Action#Args}

	 if OldLast > @last then
	    {For @last+1 OldLast 1 proc {$ I} {Dictionary.remove @hist I} end}
	 else skip end
      end

      meth insert_menu(M)
	 M =
	 if HistoryClass,is_prev($) 
	 then "menu_entry(\"prev\",\"Previous\"),"
	 else "" end
	 #if HistoryClass,is_next($) 
	 then "menu_entry(\"next\",\"Next\"),"
	 else "" end
	 #if HistoryClass,is_prev($) orelse HistoryClass,is_next($)
	  then "blank," else "" end
      end
   end

end
