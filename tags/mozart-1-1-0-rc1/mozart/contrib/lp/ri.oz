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

declare RI LP
local
   Path = '/home/ps-home/tmueller/Programming/Oz/lib/'
   ExportRI = {{New Module.manager init}
	       link(url: Path#'ri.so{native}' $)}
   
in
   RI = ri(sup:           {ExportRI.getSup}
	   inf:           {ExportRI.getInf}
	   setPrec:       ExportRI.setPrecision
	   getLowerBound: ExportRI.getLowerBound
	   getUpperBound: ExportRI.getUpperBound
	   getWidth:      ExportRI.getWidth
	   var:           var(decl:   ExportRI.declVar
			      bounds: ExportRI.newVar)
	   lessEq:        ExportRI.lessEq
	   lessEqMeta:    ExportRI.lessEqMeta
	   greater:       ExportRI.greater
	   intBounds:     ExportRI.intBounds
	   intBoundsSPP:  ExportRI.intBoundsSPP
	   times:         ExportRI.times
	   plus:          ExportRI.plus
	   distribute:    proc {$ V}
			     choice
				if {IsDet V} then skip
				else
				   L = {RI.getLowerBound V}
				   U = {RI.getUpperBound V}
				   Mid in
				   Mid = L + (U - L) / 2.0
				   dis
				      {RI.lessEq V Mid} then {RI.distribute V}
				   []
				      {RI.greater V Mid} then {RI.distribute V}
				   end
				end
			     end
			  end
	  )

   LP = lp(solve:   ExportRI.lpsolve
	   config:  ExportRI.lpsolve_conf)

end

/*
{Browse RI}

declare Vs [A B C D] = Vs R S in
{Browse lp(vs:Vs sol: S res: R)}


{ForAll Vs proc {$ V} V = {RI.mkVar 0.0 10.0} end}
{LP.solve
 Vs
 objfn(row: [1.0 2.0 3.0 4.0] opt: max)
 a(
  constr(row: [1.0 2.0 3.0 4.0] type: '=<' rhs:10.0) 
  constr(row: [1.0 2.0 3.0 4.0] type: '==' rhs:10.0) 
  constr(row: [1.0 2.0 3.0 4.0] type: '>=' rhs:10.0) 
  )
 S
 R}


{Browse {LP.config get}}
{Browse tobias}
{LP.config put conf(mode: verbose)} 
{LP.config put conf(mode: quiet)}
{LP.config put conf(solver: cplex_primopt)}
{LP.config put conf(solver: cplex_dualopt)}
{LP.config put conf(solver: lpsolve)}

\switch +debuginfocontrol
{Property.put 'internal.debug' true}
declare [R] = {Module.link [{OS.getEnv 'HOME'}#'/Programming/Oz/VisualizeConstraints/ReflectConstraints.ozf']}

declare X Y C in {ForAll [X Y] proc {$ V} V = {RI.var.bounds 0.0 10.0} end}

{RI.lessEqMeta X Y C}


{Show {R.propName C}}
{Show {R.propCoordinates C}}
{Show C}

*/
