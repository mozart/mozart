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

%X
%X::2#3
%proc {Search S}
%S Y
%S
%S = {Space.new {Queens 10}}
%{Space.inject S proc{$ _} X in  X = {Space.new {Queens 8}} end}

%C
%_ = {Space.clone C}

%end

%W
%W::1#10

%X = {Space.new Search}
%Y = {Space.new Search}
for X in 1..3 do
   {Show {SearchAll {Queens 11}}}
%{ExploreAll {Queens 6}}
end


%{System.gcDo}