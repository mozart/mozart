\define DEBUG
functor

export

   BrowserPluginActivate
   ExplorerPluginActivate
   InvestigateConstraints

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   FS
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

   Aux(vectorToList: VectorToList varEq: VarEq)

   Error

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define

   proc {Loop VarTable PropTable Hist DaVin Stream Result}
      case Stream.1
      of quit then skip
      [] popup_selection_node(_ C) then
         {MakeAction VarTable PropTable Hist DaVin Stream C Result}
      [] popup_selection_edge(_ C) then
         {MakeAction VarTable PropTable Hist DaVin Stream C Result}
      [] node_selections_labels([cn(C)]) then
         {MakeAction VarTable PropTable Hist DaVin Stream cn(C) Result}
      [] error(...) then
         {Error.printException Stream.1}
         {Loop VarTable PropTable Hist DaVin Stream.2 Result}
      else
         {Loop VarTable PropTable Hist DaVin Stream.2 Result}
      end
   end

   proc {MakeAction VarTable PropTable Hist DaVin Stream C Result}
      Skip
      NextResult
      NextArgs
      NextArgsType
      NextAction =
      case {Label C}
      of     cg  then NextArgsType = c Skip = no  ConstrGraph.make
      elseof scg then NextArgsType = c Skip = no  SingleConstrGraph.make
      elseof vg  then NextArgsType = v Skip = no  ParamGraph.make
      elseof svg then NextArgsType = v Skip = no  SingleParamGraph.make
      elseof cn  then
         LocationProp  = PropTable.(C.1).location
      in
         {Emacs.condSend.interface
          bar(file:   {VirtualString.toAtom LocationProp.path#"/"
                       #LocationProp.file}
              line:   LocationProp.line
              column: LocationProp.column
              state:  runnable)}

         NextArgsType = unit
         Skip         = yes
         unit
      elseof markparam then
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
         NextArgsType = unit
         Skip         = display
         {Hist get_prev_action($ NextArgs)}
      elseof next then
         NextArgsType = unit
         Skip         = display
         {Hist get_next_action($ NextArgs)}
      elseof corrcg then
         CurrVars = {Hist get_curr_action(_ $)} % add assertion
         NextAction = ConstrGraph.make
      in
         Skip         = display
         NextArgs     = {FoldR {FS.reflect.lowerBoundList CurrVars}
                         fun {$ L R}
                            {FS.union VarTable.L.propagators R}
                         end {FS.value.make nil}}
         {Hist add_action(NextAction NextArgs)}
         NextAction
      elseof corrvg then
         CurrProps = {Hist get_curr_action(_ $)} % add assertion
         NextAction = ParamGraph.make
      in
         Skip         = display
         NextArgs     = {FoldR {FS.reflect.lowerBoundList CurrProps}
                         fun {$ L R}
                            {FS.union PropTable.L.parameters R}
                         end {FS.value.make nil}}
         {Hist add_action(NextAction NextArgs)}
         NextAction
      elseof addconcg then
         CurrProps    = {Hist get_curr_action(_ $)} % add assertion
         NextAction   = ConstrGraph.make
         ConnectProps = PropTable.(C.1).connected_props
      in
         Skip         = display
         NextArgs     = {FS.union ConnectProps CurrProps}
         {Hist add_action(NextAction NextArgs)}
         NextAction
      elseof addconvg then
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
         NextResult = {NextAction VarTable PropTable Hist NextArgs}
         {DaVin graph(NextResult.graph)}
      elseif Skip == display then
         NextResult = {NextAction VarTable PropTable Hist NextArgs}
         {DaVin graph(NextResult.graph)}
      else
         NextResult = Result
      end
      {Loop VarTable PropTable Hist DaVin Stream.2 NextResult}
   end

   proc {InvestigateConstraints Root}
      Stream Result
      DaVin = {New DaVinci.daVinciClass init(Stream)}
      Hist  = {New History.historyClass init}
      reflect_space(varsTable:  VarTable
                    propTable:  PropTable
                    failedProp: FailPropId) = {ReflectSpace Root}

      SolVars = {FS.value.make
                 {Map {VectorToList Root}
                  fun {$ E}
                     {Record.foldL VarTable
                      fun {$ L var(reference: Ref id: Id...)}
                         if L == unit then
                            if {VarEq Ref E} then Id else unit end
                         else L end
                      end unit}
                  end}}
   in
      {Hist set_sol_vars(SolVars)}

\ifdef DEBUG
      {System.showInfo '\tConstructing Graph ...'}
\endif
      if FailPropId == unit then
         What = SolVars
      in
         {Hist add_action(ParamGraph.make What)}
         Result = {ParamGraph.make VarTable PropTable Hist What}
      else
         What = {FS.value.make FailPropId}
      in
         {Hist add_action(ParamGraph.make What)}
         Result = {SingleConstrGraph.make VarTable PropTable Hist What}
      end

\ifdef DEBUG
      {System.showInfo '\tDrawing graph.'}
\endif
      {DaVin graph(Result.graph)}

\ifdef DEBUG
      {System.showInfo '\tDone and looping.'}
\endif

      {Loop VarTable PropTable Hist DaVin Stream Result}
   end

   local
      proc {InvestigateConstraintsBrowser Root}
         Stream Result
         DaVin = {New DaVinci.daVinciClass init(Stream)}
         Hist  = {New History.historyClass init}
         reflect_space(varsTable:  VarTable
                       propTable:  PropTable
                       failedProp: FailPropId)  = {ReflectSpace Root}
      in
         {Hist add_action(SingleParamGraph.make [Root])}
         Result = {SingleParamGraph.make VarTable PropTable Hist [Root]}
         {DaVin graph(Result.graph)}
         thread {Loop VarTable PropTable Hist DaVin Stream Result} end
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

end
