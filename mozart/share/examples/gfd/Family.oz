%%%
%%% Authors:
%%%   Andres Felipe Barco <anfelbar@univalle.edu.co>
%%%
%%% Copyright:
%%%   Andres Felipe Barco, 2008
%%%
%%% Last change:
%%%   $Date: $ by $Author: $
%%%   $Revision: $
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

%%% A Family Puzzle
%%% 
%%% Maria and Clara are both heads of households,
%%% and both families have three boys and three
%%% girls.  Neither family includes any children
%%% closer in age than one year, and all children
%%% are under age 10.  The youngest child in
%%% Maria's family is a girl, and Clara has just
%%% given birth to a little girl.
%%% 
%%% In each family, the sum of the ages of the
%%% boys equals the sum of the ages of the girls,
%%% and the sum of the squares of the ages of the
%%% boys equals the sum of the the squares of
%%% ages of the girls.  The sum of the ages of
%%% all children is 60.
%%% 
%%% What are the ages of the children in each
%%% family?

declare
proc {Family Root}
   proc {FamilyC Name F}
      Coeffs = [1 1 1 ~1 ~1 ~1]
      Ages
   in
      F = Name(boys:{AgeList} girls:{AgeList})
      Ages = {Append F.boys F.girls}
      {GFD.distinctP post(Ages)}
      {GFD.sumC Coeffs Ages '=:' 0}
      {GFD.sumCN Coeffs {Map Ages fun {$ A} [A A] end} '=:' 0}
   end
   proc {AgeList L}
      {GFD.list 3 0#9 L}
      {GFD.relP post({Nth L 1} GFD.rt.'>:' {Nth L 2})}
      {GFD.relP post({Nth L 2} GFD.rt.'>:' {Nth L 3})}
   end
   Maria = {FamilyC maria}
   Clara = {FamilyC clara}
   AgeOfMariasYoungestGirl = {Nth Maria.girls 3}
   AgeOfClarasYoungestGirl = {Nth Clara.girls 3}
   Ages = {FoldR [Clara.girls Clara.boys Maria.girls Maria.boys] Append nil}
in
   Root = Maria#Clara
   {ForAll Maria.boys proc {$ A} {GFD.relP post(A GFD.rt.'>:' AgeOfMariasYoungestGirl)} end}
   AgeOfClarasYoungestGirl =: 0
   {GFD.sum Ages '=:' 60}
   {GFD.distribute split Ages}
end


%{ExploreOne Family}
{Show {SearchAll Family}}


%%% Having Clara's kids first prunes better
%%% split distribution is better than first failure
%%% together this reduces search tree by factor 3
%%% The redundant constraint AgeOfMariasYoungestGirl <: 5
%%% doubles the search space


