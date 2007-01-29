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
skip
%X::2#5
/*
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
*/
%X
%X::2#3
%proc {Search S}
%S Y
%local S in
%S = {Space.new proc{$ Root} Root::1#5 end}
%S = {Space.new proc{$ Root} R in R::1#5  Root::1#19 end}
%end

%{Space.inject S proc{$ R} R>:4 end}
%{Show {Space.askVerbose S}}
%C
%_ = {Space.clone C}

%end

%W
%W::1#10
%local X in  X = {Space.new {Queens 100}} end
%local X in  X = {Space.new proc {$ _} skip end} end
%Y = {Space.new Search}
%{System.gcDo}
%for X in 1..15 do
  %    {System.gcDo}
 %  {Show {SearchAll {Queens 11}}}
%{ExploreAll {Queens 6}}
%end
%{System.gcDo}

