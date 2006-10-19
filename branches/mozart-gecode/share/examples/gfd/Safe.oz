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



proc{Safe C}
   C1 C2 C3 C4 C5 C6 C7 C8 C9
in
   C1 = {GFD.int 1#9} C2 = {GFD.int 1#9}
   C3 = {GFD.int 1#9} C4 = {GFD.int 1#9}
   C5 = {GFD.int 1#9} C6 = {GFD.int 1#9}
   C7 = {GFD.int 1#9} C8 = {GFD.int 1#9}
   C9 = {GFD.int 1#9}
   C = [C1 C2 C3 C4 C5 C6 C7 C8 C9]
   
   {GFD.distinct  C GFD.cl.bnd}

   {GFD.linear2 [1 ~1 ~1] [C4 C6 C7] GFD.rt.'=:' 0 GFD.cl.val}
   local Tmp1 Tmp2 in
      %Tmp1 = {GFD.int ~100000#100000}
      %Tmp2 = {GFD.int ~100000#100000}
      Tmp1 = {GFD.decl}
      Tmp2 = {GFD.decl}
      {GFD.mult C1 C2 Tmp1}
      {GFD.mult Tmp1 C3 Tmp2}
      {GFD.linear2 [1 ~1 ~1] [Tmp2 C8 C9] GFD.rt.'=:' 0 GFD.cl.val}
   end
   {GFD.linear2 [1 1 1 ~1] [C2 C3 C6 C8] GFD.rt.'<:' 0 GFD.cl.val}
   {GFD.linear2 [1 ~1] [C9 C8] GFD.rt.'<:' 0 GFD.cl.val}
   for I in 1..9 do
      {GFD.rel {List.nth C I} GFD.rt.'\\=:' I GFD.cl.val}
   end
%   {GSpace.status SP _}
   {GFD.distribute ff C}
end

{Show {SearchAll Safe}}

%% S = {GSpace.new}
%% X = {GFD.int S 0#1}
%% Y = {GFD.int S 0#1}
%% Z = {GFD.int S 0#1}
%% {GFD.bool_and X Y Z GFD.cl.dom}
%% {GFD.rel X GFD.rt.'=:' 1 GFD.cl.dom}
%% {GFD.rel Y GFD.rt.'=:' 0 GFD.cl.dom}
%% {GSpace.status S _}
%% {Show X#Y#Z}
