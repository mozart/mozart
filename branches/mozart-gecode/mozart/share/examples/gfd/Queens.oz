/*
 *  Main authors:
 *     Alejandro Arbelaez: <aarbelaez@puj.edu.co>
 *
 *
 *  Contributing authors:
 *
 *
 *  Copyright:
 *     Alejandro Arbelaez, 2006
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

fun{Queens N}
   proc{$ Root}
      C1 = {List.number 1 N 1}
      C2 = {List.number ~1 ~N ~1}
   in
      Root = {GFD.intVarList N 1#N}
      {GFD.distinct Root GFD.cl.val}
      {GFD.distinctOffset Root C1}
      {GFD.distinctOffset Root C2}
      {GFD.distribute ff Root}
   end
end

%{Show {SearchOne {Queens 11}}}
{ExploreAll {Queens 11}}