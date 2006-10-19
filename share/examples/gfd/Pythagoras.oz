/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *     
 *
 *  Contributing authors:
 *		   
 *
 *  Copyright:
 *     Alejandro Arbelaez, 2006
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

proc{Square X S}
   {GFD.mult X X S}
end

proc{Pythagoras Root}
   [A B C] = Root
   AA BB CC
in

   {GFD.dom1 1#1000 Root}
   {GFD.dom1 0#FD.sup [AA BB CC]}
   {Square A AA}
   {Square B BB}
   {Square C CC}
   {GFD.linear [AA BB] GFD.rt.'=:' CC GFD.cl.val}
   {GFD.rel A GFD.rt.'=<:' B GFD.cl.val}
   {GFD.rel B GFD.rt.'=<:' C GFD.cl.val}

   {GFD.distribute ff Root}
end

{System.show {SearchAll Pythagoras}}

