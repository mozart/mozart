functor

export
   
   BrowserPluginActivate
   ExplorerPluginActivate
   InvestigateConstraints
   InvestigateProcedureGraph
   DisplayConstraintGraph
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   FS
   ProcedureGraph
   ConstrGraph
   ParamGraph
   SingleConstrGraph
   SingleParamGraph
   DaVinci at 'x-oz://contrib/DaVinci'
   Reflect(spaceReflect: ReflectSpace) at 'x-oz://contrib/Reflect' 
   Explorer
   Browser
   Emacs   
   History
   Config(unMarkedPropNodeAttr:  UnMarkedPropNodeAttr
	  unMarkedParamNodeAttr: UnMarkedParamNodeAttr)

   System

   Misc(vectorToList: VectorToList varEq: VarEq)
   
   Error
   Property
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define
   fun {CorrespondingItems CurrItems CurrTable NewItems}
      {List.foldLTail {FS.reflect.lowerBoundList CurrItems}
       fun {$ L R}
	  case R
	  of A|B|T then
	     {FS.union L
	      {FoldL B|T
	       fun {$ L R}
		  {FS.union {FS.intersect
			     CurrTable.R.NewItems
			     CurrTable.A.NewItems}
		   L}
	       end
	       {FS.value.make nil}}}
	  else
	     L
	  end
       end {FS.value.make nil}}
   end
      
   proc {Loop FailSet VarTable PropTable Hist DaVin Stream Result}
      case Stream.1
      of quit then skip
      [] popup_selection_node(_ C) then
	 {MakeAction FailSet VarTable PropTable Hist DaVin Stream C Result}
      [] popup_selection_edge(_ C) then
	 {MakeAction FailSet VarTable PropTable Hist DaVin Stream C Result}
      [] node_selections_labels([cn(C)]) then
	 {MakeAction FailSet VarTable PropTable Hist DaVin Stream cn(C) Result}
      [] node_selections_labels([cn(C _)]) then
	 {MakeAction FailSet VarTable PropTable Hist DaVin Stream cn(C) Result}
      [] error(...) then
	 {Error.printException Stream.1}
	 {Loop FailSet VarTable PropTable Hist DaVin Stream.2 Result}
      else
	 {Loop FailSet VarTable PropTable Hist DaVin Stream.2 Result}
      end 
   end
   
   proc {MakeAction FailSet VarTable PropTable Hist DaVin Stream C Result}
      Skip
      NextResult
      NextArgs
      NextArgsType 
      NextAction =
      case {Label C}
      of     cg  then
\ifdef DEBUG
	 {System.show mainAction_cg}
\endif
	 NextArgsType = c Skip = no  ConstrGraph.make
      elseof scg then
\ifdef DEBUG
	 {System.show mainAction_scg}
\endif
	 NextArgsType = c Skip = no  SingleConstrGraph.make
      elseof vg  then
\ifdef DEBUG
	 {System.show mainAction_vg}
\endif
	 NextArgsType = v Skip = no  ParamGraph.make
      elseof svg then
\ifdef DEBUG
	 {System.show mainAction_svg}
\endif
	 NextArgsType = v Skip = no  SingleParamGraph.make
      elseof cn  then
\ifdef DEBUG
	 {System.show mainAction_cn}
\endif
	 LocationProp  = PropTable.(C.1).location
      in
	 if LocationProp \= unit then
	    {Emacs.condSend.interface
	     bar(file:   {VirtualString.toAtom LocationProp.path#"/"
			  #LocationProp.file}
		 line:   LocationProp.line
		 column: LocationProp.column
		 state:  runnable)}
	 else skip end
	 
	 NextArgsType = unit
	 Skip         = yes
	 unit
      elseof markparam then
\ifdef DEBUG
	 {System.show mainAction_markparam}
\endif
	 MarkParam = {Hist get_mark_param($)}
      in
	 NextArgsType = unit
	 Skip         = yes
	 if MarkParam == unit then skip else
	    {DaVin sendVS("graph(change_attr([node(\"vn<"
			  #MarkParam#">\",[a(\"BORDER\",\""
			  #UnMarkedParamNodeAttr#"\")])]))")}	    
	 end
	 {Hist markup_param(DaVin C.1)}
	 unit
      elseof unmarkparam then
\ifdef DEBUG
	 {System.show mainAction_unmarkparam}
\endif
	 MarkParam = {Hist get_mark_param($)}
      in
	 NextArgsType = unit
	 Skip         = yes
	 if MarkParam == unit then skip else
	    {DaVin sendVS("graph(change_attr([node(\"vn<"
			  #MarkParam#">\",[a(\"BORDER\",\""
			  #UnMarkedParamNodeAttr#"\")])]))")}	    
	 end
	 {Hist unmark_param}
	 unit
      elseof markprop then
\ifdef DEBUG
	 {System.show mainAction_markprop}
\endif
	 MarkProp = {Hist get_mark_prop($)}
      in
	 if MarkProp == unit then skip else
	    {DaVin sendVS("graph(change_attr([node(\"cn<"
			  #MarkProp#">\",[a(\"BORDER\",\""
			  #UnMarkedParamNodeAttr#"\")])]))")}	    
	 end
	 NextArgsType = unit
	 Skip         = yes
	 {Hist markup_prop(DaVin C.1)}
	 unit
      elseof unmarkprop then
\ifdef DEBUG
	 {System.show mainAction_unmarkprop}
\endif
	 MarkProp = {Hist get_mark_prop($)}
      in
	 NextArgsType = unit
	 Skip         = yes
	 if MarkProp == unit then skip else
	    {DaVin sendVS("graph(change_attr([node(\"cn<"
			  #MarkProp#">\",[a(\"BORDER\",\""
			  #UnMarkedPropNodeAttr#"\")])]))")}
	 end
	 {Hist unmark_prop}
	 unit
      elseof prev then
\ifdef DEBUG
	 {System.show mainAction_prev}
\endif
	 NextArgsType = unit
	 Skip         = display
	 {Hist get_prev_action($ NextArgs)}
      elseof next then
\ifdef DEBUG
	 {System.show mainAction_next}
\endif
	 NextArgsType = unit
	 Skip         = display
	 {Hist get_next_action($ NextArgs)}
      elseof corrcg then
\ifdef DEBUG
	 {System.show mainAction_corrcg}
\endif	 
	 CurrVars = {Hist get_curr_action(_ $)} % add assertion
	 NextAction = ConstrGraph.make
      in
	 Skip         = display
	 NextArgs     = {CorrespondingItems CurrVars VarTable propagators}

	 {Hist add_action(NextAction NextArgs)}
	 NextAction
      elseof corrvg then
\ifdef DEBUG
	 {System.show mainAction_corrvg}
\endif
	 CurrProps = {Hist get_curr_action(_ $)} % add assertion
	 NextAction = ParamGraph.make
      in
	 Skip         = display
	 NextArgs     = {CorrespondingItems CurrProps PropTable parameters}

	 {Hist add_action(NextAction NextArgs)}
	 NextAction
      elseof addconcg then
\ifdef DEBUG
	 {System.show mainAction_addconcg}
\endif
	 CurrProps    = {Hist get_curr_action(_ $)} % add assertion
	 NextAction   = ConstrGraph.make
	 ConnectProps = PropTable.(C.1).connected_props
      in
	 Skip         = display
	 NextArgs     = {FS.union ConnectProps CurrProps}
	 {Hist add_action(NextAction NextArgs)}
	 NextAction
      elseof addconvg then
\ifdef DEBUG
	 {System.show mainAction_addconvg}
\endif
	 CurrVars    = {Hist get_curr_action(_ $)} % add assertion
	 NextAction  = ParamGraph.make
	 ConnectVars = VarTable.(C.1).connected_vars
      in
	 Skip         = display
	 NextArgs     = {FS.union ConnectVars CurrVars}
	 {Hist add_action(NextAction NextArgs)}
	 NextAction
      else
	 {Exception.raiseError vc("Unexpected case" "Loop1")} error
      end
   in
      if Skip == no then
	 NextArgs =
	 case {Map {Arity C} fun {$ F} C.F end}
	 of [all] then
	    {FS.value.make 1#{Width if NextArgsType == v
				    then VarTable
				    else PropTable end}}
	 elseof [solvar] then NextArgsType = v {Hist get_sol_var_set($)}
	 elseof L then
	    {FS.value.make L}
	 end
	 {Hist add_action(NextAction NextArgs)}
	 NextResult = {NextAction FailSet VarTable PropTable Hist NextArgs}
	 {DaVin graph(NextResult.graph)}
      elseif Skip == display then
	 NextResult = {NextAction FailSet VarTable PropTable Hist NextArgs}
	 {DaVin graph(NextResult.graph)}
      else
	 NextResult = Result
      end
      {Loop FailSet VarTable PropTable Hist DaVin Stream.2 NextResult}
   end

   proc {InvestigateConstraints Root}
      Stream Result
      DaVin = {New DaVinci.daVinciClass init(Stream)}
      Hist  = {New History.historyClass init}
      reflect_space(varsTable:  VarTable
		    propTable:  PropTable
		    procTable:  _
		    failedProp: FailPropId)

\ifdef IGNORE_REFERENCE
      = Root
\else
      = {ReflectSpace Root}
\endif
      SolVars = {FS.value.make
\ifdef IGNORE_REFERENCE
		 {Arity VarTable}
\else
		 {Filter {Map {VectorToList Root}
			  fun {$ E}
			     {Record.foldL VarTable
			      fun {$ L var(reference: Ref id: Id ...)}
				 if L == unit then
				    if {VarEq Ref E} then Id else unit end
				 else L end
			      end unit}
			  end}
		  fun {$ E} E \= unit end}
\endif
		}
   in
      {Hist set_sol_vars(SolVars)}

\ifdef DEBUG
      {System.showInfo '\tConstructing Graph ...'}
\endif
      if FailPropId == unit then 
	 What = SolVars
      in
	 {Hist add_action(ParamGraph.make What)}
	 Result = {ParamGraph.make FS.value.empty VarTable PropTable Hist What}
      else
	 What = {FS.value.make FailPropId}
      in
	 {Hist add_action(ParamGraph.make What)}
	 Result = {SingleConstrGraph.make FS.value.empty VarTable PropTable Hist What}
      end
      
\ifdef DEBUG
      {System.showInfo '\tDrawing graph.'}
\endif
      {DaVin graph(Result.graph)}

\ifdef DEBUG
      {System.showInfo '\tDone and looping.'}
\endif

      {Loop FS.value.empty VarTable PropTable Hist DaVin Stream Result}
   end

   local
      proc {InvestigateConstraintsBrowser Root}
	 Stream Result 
	 DaVin = {New DaVinci.daVinciClass init(Stream)}
	 Hist  = {New History.historyClass init}
	 reflect_space(varsTable:  VarTable
		       propTable:  PropTable
		       procTable:  _
		       failedProp: FailPropId)  = {ReflectSpace Root}
      in
	 {Hist add_action(SingleParamGraph.make [Root])}
	 Result = {SingleParamGraph.make FS.value.empty VarTable PropTable Hist [Root]}
	 {DaVin graph(Result.graph)}
	 thread {Loop FS.value.empty VarTable PropTable Hist DaVin Stream Result} end
      end
   in   
      proc {BrowserPluginActivate}
	 {Browser.object add(InvestigateConstraintsBrowser
			     label: 'Investigate Constraints')}
	 {Browser.object set(InvestigateConstraintsBrowser)}
      end
   end
   
   proc {ExplorerPluginActivate}
      {Explorer.object add(information proc {$ Node Sol}
					  {InvestigateConstraints Sol}
				       end
			   label: 'Investigate Constraints')}
   end
   
   proc {InvestigateProcedureGraph Root}
      Stream Result 
      DaVin = {New DaVinci.daVinciClass init(Stream)}
      Hist  = {New History.historyClass init}
      reflect_space(varsTable:  VarTable
		    propTable:  PropTable
		    procTable:  ProcTable
		    failedProp: FailPropId)  = {ReflectSpace Root}
      Elem = {Nth {Arity ProcTable} 3}
/*
      What = {FS.diff 
	      {FS.union ProcTable.Elem.subsumed_props
	       {FS.value.make {Arity ProcTable}}}
	      {FS.value.make Elem}}
*/
      What = {FS.value.make {Arity ProcTable}}
   in
      {Hist add_action(ProcedureGraph.make [Root])}
      Result = {ProcedureGraph.make FS.value.empty VarTable PropTable ProcTable Hist What}
      {DaVin graph(Result.graph)}
   end

   proc {DisplayConstraintGraph Root FailSet}
      Stream Result
      DaVin = {New DaVinci.daVinciClass init(Stream)}
      Hist  = {New History.historyClass init}
      reflect_space(varsTable:  VarTable
		    propTable:  PropTable
		    procTable:  _
		    failedProp: FailProp)

      = Root
      SolVars = {FS.value.make
\ifdef IGNORE_REFERENCE
		 {Arity VarTable}
\else
		 {Filter {Map {VectorToList Root}
			  fun {$ E}
			     {Record.foldL VarTable
			      fun {$ L var(reference: Ref id: Id ...)}
				 if L == unit then
				    if {VarEq Ref E} then Id else unit end
				 else L end
			      end unit}
			  end}
		  fun {$ E} E \= unit end}
\endif
		}
   in
      {Hist set_sol_vars(SolVars)}
      {Hist add_action(ParamGraph.make SolVars)}
      Result = {ConstrGraph.make FailSet VarTable PropTable Hist
		{FS.value.make {Arity PropTable}}}
      {DaVin graph(Result.graph)}
      {Loop FailSet VarTable PropTable Hist DaVin Stream Result}
   end

   {Property.put 'internal.propLocation' true}
   {Property.put 'gc.on' false}

end
