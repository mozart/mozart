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

functor

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
import
   
   Export_RI at
   'http://www.ps.uni-sb.de/~tmueller/mozart/ri.so{native}'

export
   
   sup:           Sup
   inf:           Inf
   setPrec:       SetPrec
   getLowerBound: GetLowerBound
   getUpperBound: GetUpperBound
   getWidth:      GetWidth  
   var:           Var
   lessEq:        LessEq
   greater:       Greater
   intBounds:     IntBounds
   intBoundsSPP:  IntBoundsSPP
   times:         Times
   plus:          Plus
   distribute:    Distribute
	 
   
define
   
   Sup           = {Export_RI.getSup}
   Inf           = {Export_RI.getInf}
   SetPrec       = Export_RI.setPrecision
   GetLowerBound = Export_RI.getLowerBound
   GetUpperBound = Export_RI.getUpperBound
   GetWidth      = Export_RI.getWidth
   Var           = var(decl:   Export_RI.declVar
		       bounds: Export_RI.newVar)
   LessEq        = Export_RI.lessEq
   Greater       = Export_RI.greater
   IntBounds     = Export_RI.intBounds
   IntBoundsSPP  = Export_RI.intBoundsSPP
   Times         = Export_RI.times
   Plus          = Export_RI.plus
   Distribute    = proc {$ V}
		      choice
			 if {IsDet V} then skip
			 else
			    L = {GetLowerBound V}
			    U = {GetUpperBound V}
			    Mid in
			    Mid = L + (U - L) / 2.0
			    dis
			       {LessEq V Mid} then {Distribute V}
			    []
			       {Greater V Mid} then {Distribute V}
			    end
			 end
		      end
		   end
end
