functor

export

   BrowserPluginActivate
   ExplorerPluginActivate
   InvestigateConstraints

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   ConstrGraph
   ParamGraph
   SingleConstrGraph
   SingleParamGraph
   DaVinci at 'x-oz://contrib/DaVinci'
   CollectConstraints
   Tables
   Explorer
   Browser
   Aux(propLocation)
   Emacs
   History
   Config(
      unMarkedPropNodeAttr:   UnMarkedPropNodeAttr
      unMarkedParamNodeAttr:  UnMarkedParamNodeAttr
      )
\ifdef DEBUG
   System
\endif

   Error

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define

   proc {Loop VarTable PropTable Hist DaVin Stream AllVars AllConstrs Result}
      case Stream.1
      of quit then skip
      [] popup_selection_node(_ C) then
         {MakeAction VarTable PropTable Hist DaVin Stream C AllVars AllConstrs Result}
      [] popup_selection_edge(_ C) then
         {MakeAction VarTable PropTable Hist DaVin Stream C AllVars AllConstrs Result}
      [] node_selections_labels([cn(C)]) then
         {MakeAction VarTable PropTable Hist DaVin Stream cn(C) AllVars AllConstrs Result}
      [] error(...) then
         {Error.printException Stream.1}
         {Loop VarTable PropTable Hist DaVin Stream.2 AllVars AllConstrs Result}
      else
         {Loop VarTable PropTable Hist DaVin Stream.2 AllVars AllConstrs Result}
      end
   end

   proc {MakeAction VarTable PropTable Hist DaVin Stream C AllVars AllConstrs Result}
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
         P = {Tables.getProp PropTable C.1}
         LocationProp  = {Aux.propLocation P}
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
            {DaVin sendVS("graph(change_attr([node(\"vn<"#MarkParam#">\",[a(\"BORDER\",\""#UnMarkedParamNodeAttr#"\")])]))")}
         end
         {Hist markup_param(DaVin C.1)}
         unit
      elseof unmarkparam then
         MarkParam = {Hist get_mark_param($)}
      in
         NextArgsType = unit
         Skip         = yes
         if MarkParam == unit then skip else
            {DaVin sendVS("graph(change_attr([node(\"vn<"#MarkParam#">\",[a(\"BORDER\",\""#UnMarkedParamNodeAttr#"\")])]))")}
         end
         {Hist unmark_param}
         unit
      elseof markprop then
         MarkProp = {Hist get_mark_prop($)}
      in
         if MarkProp == unit then skip else
            {DaVin sendVS("graph(change_attr([node(\"cn<"#MarkProp#">\",[a(\"BORDER\",\""#UnMarkedParamNodeAttr#"\")])]))")}
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
            {DaVin sendVS("graph(change_attr([node(\"cn<"#MarkProp#">\",[a(\"BORDER\",\""#UnMarkedPropNodeAttr#"\")])]))")}
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
      else
         {Exception.raiseError vc("Unexpected case" "Loop1")} error
      end
   in
      if Skip == no then
         NextArgs =
         case {Map {Arity C} fun {$ F} C.F end}
         of [all] then
            if NextArgsType == v then AllVars else AllConstrs end
         elseof [sub] then
            if NextArgsType == v then
               {Map {Tables.getVarAllIds VarTable}
                fun {$ Id} {Tables.getVar VarTable Id} end}
            else
               {Map {Tables.getPropAllIds PropTable}
                fun {$ Id} {Tables.getProp PropTable Id} end}
            end
         elseof L then
            if NextArgsType == v then
               {Map L fun {$ Id} {Tables.getVar VarTable Id} end}
            else
               {Map L fun {$ Id} {Tables.getProp PropTable Id} end}
            end
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
      {Loop VarTable PropTable Hist DaVin Stream.2 AllVars AllConstrs NextResult}
   end

   proc {InvestigateConstraints Root}
      Stream Result AllVars AllConstrs
      CC    = {New CollectConstraints.collectConstraintsClass init}
      DaVin = {New DaVinci.daVinciClass init(Stream)}
      Hist  = {New History.historyClass init}
      VarTable = {Tables.makeVarTable}
      PropTable = {Tables.makePropTable}
   in
\ifdef DEBUG
      {System.show 'Collecting space'}
\endif

      {CC collect(Root)}
\ifdef DEBUG
      {System.show 'Getting vars'}
\endif

      AllVars = {CC get_vars($)}
\ifdef DEBUG
      {System.show 'Getting constraints'}
\endif

      AllConstrs = {CC get_props($)}
\ifdef DEBUG
      {System.show 'Adding action'}
\endif

      {Hist add_action(ConstrGraph.make AllConstrs)}
\ifdef DEBUG
      {System.show 'Constructing Graph'}
\endif

      Result = {ConstrGraph.make VarTable PropTable Hist AllConstrs}
%      Result = {ParamGraph.make VarTable PropTable Hist AllVars}
\ifdef DEBUG
      {System.show 'Drawing graph'}
\endif

      {DaVin graph(Result.graph)}
\ifdef DEBUG
      {System.show 'Done and looping'}
\endif

      {Loop VarTable PropTable Hist DaVin Stream AllVars AllConstrs Result}
   end

   local
      proc {InvestigateConstraintsBrowser Root}
         Stream Result AllVars AllConstrs
         CC    = {New CollectConstraints.collectConstraintsClass init}
         DaVin = {New DaVinci.daVinciClass init(Stream)}
         Hist  = {New History.historyClass init}
         VarTable = {Tables.makeVarTable}
         PropTable = {Tables.makePropTable}
      in
         {CC collect(Root)}
         AllVars = {CC get_vars($)}
         AllConstrs = {CC get_props($)}
         {Hist add_action(SingleParamGraph.make [Root])}
         Result = {SingleParamGraph.make VarTable PropTable Hist [Root]}
         {DaVin graph(Result.graph)}
         thread {Loop VarTable PropTable Hist DaVin Stream AllVars AllConstrs Result} end
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
