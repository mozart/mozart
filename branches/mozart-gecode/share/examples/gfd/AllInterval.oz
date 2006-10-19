/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *     
 *
 *  Contributing authors:
 *		  
 *
 *  Copyright:
 *     Alejandro Arbelaez
 *     
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */
declare

fun{AllInterval N}
   proc{$ Root}
      D X
   in
      X = {GFD.tuple x N 0#N-1}
      D = {GFD.tuple d N-1 1#N-1}
      X = Root
      for I in 1..(N-1) do Tmp1 in
	 Tmp1 = {GFD.decl}
	 {GFD.minus X.(I+1) X.I Tmp1}
	 {GFD.abs Tmp1 D.I GFD.cl.bnd}
      end
      {GFD.distinct X GFD.cl.bnd}
      {GFD.distinct D GFD.cl.bnd}
      
      {GFD.rel X.1 GFD.rt.'=<:' X.2 GFD.cl.bnd}
      {GFD.rel D.1 GFD.rt.'>=:' D.2 GFD.cl.bnd}
      
      {GFD.distribute naive Root}
   end
end
%{Show {AllInterval}}
{Show {SearchOne {AllInterval 5}}}
