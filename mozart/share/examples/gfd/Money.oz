%%%
%%% Authors:
%%%     Alejandro Arbelaez <aarbelaez@puj.edu.co>
%%%
%%% Copyright:
%%%     Alejandro Arbelaez, 2006
%%%
%%% Last change:
%%%   $Date: 2006-10-19T01:44:35.108050Z $ by $Author: ggutierrez $
%%%   $Revision: 2 $
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

declare

%T = {GFD.int  0#9}

proc{Money Root}
    S E N D
    M O R Y
    RootVal
 in
   Root =  [S E N D M O R Y]
   
   RootVal = [1000 100 10 1 1000 100 10 1 ~10000 ~1000 ~100 ~10 ~1]
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

{Show {SearchOne Money}}

proc {SPCommit2 S BL}
   {Show spcommit2}
   {Space.inject S proc {$ _}
		      {Space.branch BL}
		   end}
end


fun {OneDepthNR S}
   case {Space.ask S}
   of failed then {Show faillll} nil
   [] succeeded then {Show succeeded} S
   [] branch([B]) then
      {Show commit1}
      {Space.commitB S B}
      {OneDepthNR S}
   [] branch(B|Br) then C={Space.clone S} in
      {Space.commitB S B}
      {Show B}
      {Show Br}
      case {OneDepthNR S}
      of nil then {SPCommit2 C Br} {OneDepthNR C}
      elseof O then O
      end
   end
end


%S={Space.new Money}
%W = {OneDepthNR S}



