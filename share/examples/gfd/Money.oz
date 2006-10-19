/*
 *  Main authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *     
 *
 *  Contributing authors:
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>     
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

T = {GFD.int  0#9}

proc{Money Root}
    S E N D
    M O R Y
    RootVal
 in
    Root =  [S E N D M O R Y]
   RootVal = [1000 100 10 1 1000 100 10 1 ~10000 ~1000 ~100 ~10 ~1]
%     S = {GFD.int  0#9}
%     E = {GFD.int  0#9}
%     N = {GFD.int  0#9} D = {GFD.int  0#9}
%     M = {GFD.int  0#9} O = {GFD.int  0#9}
%     R = {GFD.int  0#9} Y = {GFD.int  0#9}

   Root:::0#9
   {GFD.linear2  RootVal
    [S E N D M O
     R E M O N E Y]
    GFD.rt.'=:' 0
    GFD.cl.bnd}
   
   {GFD.rel S GFD.rt.'\\=:' 0 GFD.cl.bnd}
   {GFD.rel M GFD.rt.'\\=:' 0 GFD.cl.bnd}
  
   {GFD.distinct  Root GFD.cl.bnd}
   
   {GFD.distribute ff Root}
end

%S = {Space.new Money}
%{System.show {Space.ask S}}
%%%{Show {SearchAll Money}}
{ExploreAll Money}
T <: 3
{Browse T}
%{Show {SearchOne Money}}
%%X Y
%%X = {GFD.int 0#1}
%%Y = {GFD.int 0#1}
%{Show X}








