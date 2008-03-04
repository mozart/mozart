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

proc{Money Root}
    S E N D
    M O R Y
    RootVal
 in
   Root =  [S E N D M O R Y]
   
   RootVal = [1000 100 10 1 1000 100 10 1 ~10000 ~1000 ~100 ~10 ~1]
   Root:::0#9
   {GFD.linearP  post(RootVal
		      [S E N D M O
		       R E M O N E Y]
		      GFD.rt.'=:' 0
		      cl:GFD.cl.bnd)}
   
   {GFD.relP post(S GFD.rt.'\\=:' 0 cl:GFD.cl.bnd)}
   {GFD.relP post(M GFD.rt.'\\=:' 0 cl:GFD.cl.bnd)}
  
   {GFD.distinctP  post(Root cl:GFD.cl.bnd)}
   {GFD.distributeBR ff Root}
end

{Show {SearchOne Money}}




