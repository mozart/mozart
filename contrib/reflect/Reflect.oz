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

   fun {SpaceReflect Vs}
\ifdef VERBOSE
      {System.showInfo '\t collecting ... '}
\endif
      ReflectTables = {BIspaceReflect Vs}
\ifdef VERBOSE
      {System.showInfo '\t preparing '#{Width ReflectTables.vars}
       #' variables ... '}
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
                         type:           Type
                         reference:      Reference
                         susplists:      SL
                         propagators:    PS
                         connected_vars: CV)
                  end}
\ifdef VERBOSE
      {System.showInfo '\t preparing '#{Width ReflectTables.props}
       #' propagators ... '}
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
                              Params %{FS.reflect.lowerBoundList PS}
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
      {System.showInfo '\t done. '}
\endif

      reflect_space(varsTable: VarTable
                    propTable: PropTable)
   end

end
