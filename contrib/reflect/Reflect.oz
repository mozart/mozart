/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     $MOZARTURL$
 *
 *  See the file "LICENSE" or
 *     $LICENSEURL$
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

\define VERBOSE

functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
export

   VarReflect
   VarEq
   PropReflect
   PropEq
   PropName
   PropLocation
   PropIsFailed
   SpaceReflect

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import

   ReflectExport at 'reflect.so{native}'
   System
   FS
   CTB at 'x-oz://boot/CTB'

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
define

   VarReflect   = ReflectExport.variableReflect
   VarEq        = System.eq
   PropReflect  = ReflectExport.propagatorReflect
   PropEq       = ReflectExport.propagatorEq
   PropName     = ReflectExport.propagatorName
   PropLocation = ReflectExport.propagatorCoordinates
   PropIsFailed = ReflectExport.propagatorIsFailed

   BIspaceReflect = ReflectExport.spaceReflect

   GetCtVarNameAsAtom       = CTB.getNameAsAtom
   GetCtVarConstraintAsAtom = CTB.getConstraintAsAtom

   fun {VariableToVirtualString V}
      case {Value.status V}
      of kinded(free)  then {Value.toVirtualString V 1 1}
      [] kinded(int)   then {Value.toVirtualString V 1 1}
      [] kinded(fset)  then {Value.toVirtualString V 1 1}
      [] kinded(other) then {Value.toVirtualString V 1 1}
         #'<'#{GetCtVarNameAsAtom V}#':'#{GetCtVarConstraintAsAtom V}#'>'
      else
         {Exception.raiseError vc("Unexpected case" "VariableToVirtualString")}
         error
      end
   end

   fun {SpaceReflect Vs}
\ifdef VERBOSE
      {System.showInfo '\t collecting ...'}
\endif
      ReflectTables = {BIspaceReflect Vs}
\ifdef VERBOSE
      {System.showInfo '\t preparing '#{Width ReflectTables.vars}
       #' variables ...'}
\endif
      VarTable = {Record.map
                  ReflectTables.vars
                  fun {$ var(id:        Id
                             name:      Name
                             susplists: SuspLists
                             type:      Type
                             reference: Reference)}
                     SL = {Record.map SuspLists FS.value.make}
                     PS = {FS.var.decl} = {FS.unionN SL}
                     CV = {FS.diff {FS.unionN
                                    {Map
                                     {FS.reflect.lowerBoundList PS}
                                     fun {$ PI}
                                        {FS.value.make
                                         ReflectTables.props.PI.params}
                                     end}}
                           {FS.value.make Id}}
                  in
                     var(id:             Id
                         name:           Name
                         nameconstraint: {VariableToVirtualString Reference}
                         type:           Type
                         reference:      Reference
                         susplists:      SL
                         propagators:    PS
                         connected_vars: CV)
                  end}
\ifdef VERBOSE
      {System.showInfo '\t preparing '#{Width ReflectTables.props}
       #' propagators ...'}
\endif
      PropTable = {Record.map
                   ReflectTables.props
                   fun {$ propagator(id:        Id
                                     location:  Location
                                     name:      Name
                                     params:    Params
                                     reference: Reference)}
                      PS = {FS.value.make Params}
                      CP = {FS.diff
                            {FS.unionN
                             {Map
                              Params
                              fun {$ PI}
                                 VarTable.PI.propagators
                              end}}
                            {FS.value.make Id}}

                   in
                      propagator(id:              Id
                                 name:            Name
                                 location:        Location
                                 reference:       Reference
                                 parameters:      PS
                                 connected_props: CP)
                   end}
   in
\ifdef VERBOSE
      {System.showInfo '\t done.'}
\endif

      reflect_space(varsTable:  VarTable
                    propTable:  PropTable
                    failedProp: {Record.foldL PropTable
                                 fun {$ L propagator(reference: Ref
                                                     id: Id
                                                     ...)}
                                    if L == unit then
                                       if {PropIsFailed Ref}
                                       then Id else unit end
                                    else L end
                                 end unit})
   end

end
