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

%%% Adapted from a finite domain example in Mozart-Oz version 1.3.2 by 
%%% Gert Smolka, 1998.

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
   {GFD.dom 0#9 Root}
   D  \=: O
   G \=: 0
   R \=: 0
   {GFD.sumC RootArgs
		 [D O N A L D
		 G E R A L D
		 R O B E R T]
		   '=:' 0}
   {GFD.distinctP post(Root cl:GFD.cl.val)}
   {GFD.distributeBR naive Root}
end

{Show {SearchOne Donald}}
