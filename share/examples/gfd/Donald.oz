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


proc{Donald Root}
   D O N A L
   G E R B T
   RootArgs
in
   Root = [D O N A L G E R B T]
   RootArgs = [100000 10000 1000 100 10 1
	       100000 10000 1000 100 10 1
	       ~100000 ~10000 ~1000 ~100 ~10 ~1]
   {GFD.dom1 0#9 Root}
   {GFD.rel D GFD.rt.'\\=:' O GFD.cl.val}
   {GFD.rel G GFD.rt.'\\=:' 0 GFD.cl.val}
   {GFD.rel R GFD.rt.'\\=:' 0 GFD.cl.val}
   {GFD.linear2 RootArgs
    [D O N A L D G E R A L D R O B E R T]
    GFD.rt.'=:' 0 GFD.cl.val}
   {GFD.distinct Root GFD.cl.val}
   {GFD.distribute ff Root}
end

{Show {SearchOne Donald}}
