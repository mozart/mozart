/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *     
 *
 *  Contributing authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
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


fun{MagicSequence N}
   proc{$ Seq}
      Cs = {List.number ~1 N-2 1}
   in
      Seq = {GFD.tuple sequence N 0#N-1}
      for I in 0..N-1 do
	 {GFD.count Seq I GFD.rt.'=:' Seq.(I+1) GFD.cl.val}
      end
      {GFD.linear Seq GFD.rt.'=:' N GFD.cl.val}
      {GFD.linear2 Cs Seq GFD.rt.'=:' 0 GFD.cl.val}
      {GFD.distribute ff Seq}
   end
end

{ExploreOne {MagicSequence 10}}
