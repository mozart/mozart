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



proc{Alpha Root}
   A B C D E F G H I J K L M
   N O P Q R S T U V W X Y Z
in
   Root = [A B C D E F G H I J K L M
	   N O P Q R S T U V W X Y Z]
   {GFD.dom1 1#26 Root}
   {GFD.linear [B A L L E T] GFD.rt.'=:' 45 GFD.cl.val}
   {GFD.linear [C E L L O] GFD.rt.'=:' 43 GFD.cl.val}
   {GFD.linear [C O N C E R T] GFD.rt.'=:' 74 GFD.cl.val}
   {GFD.linear [F L U T E] GFD.rt.'=:' 30 GFD.cl.val}
   {GFD.linear [F U G U E] GFD.rt.'=:' 50 GFD.cl.val}
   {GFD.linear [G L E E] GFD.rt.'=:' 66 GFD.cl.val}
   {GFD.linear [J A Z Z] GFD.rt.'=:' 58 GFD.cl.val}
   {GFD.linear [L Y R E] GFD.rt.'=:' 47 GFD.cl.val}
   {GFD.linear [O B O E] GFD.rt.'=:' 53 GFD.cl.val}
   {GFD.linear [O P E R A] GFD.rt.'=:' 65 GFD.cl.val}
   {GFD.linear [P O L K A] GFD.rt.'=:' 59 GFD.cl.val}
   {GFD.linear [Q U A R T E T] GFD.rt.'=:' 50 GFD.cl.val}
   {GFD.linear [S A X O P H O N E] GFD.rt.'=:' 134 GFD.cl.val}
   {GFD.linear [S C A L E] GFD.rt.'=:' 51 GFD.cl.val}
   {GFD.linear [S O L O] GFD.rt.'=:' 37 GFD.cl.val}
   {GFD.linear [S O N G] GFD.rt.'=:' 61 GFD.cl.val}
   {GFD.linear [S O P R A N O] GFD.rt.'=:' 82 GFD.cl.val}
   {GFD.linear [T H E M E] GFD.rt.'=:' 72 GFD.cl.val}
   {GFD.linear [V I O L I N] GFD.rt.'=:' 100 GFD.cl.val}
   {GFD.linear [W A L T Z] GFD.rt.'=:' 34 GFD.cl.val}

   {GFD.distinct Root GFD.cl.val}
   {GFD.distribute ff Root}
end

{Show {SearchOne Alpha}}
