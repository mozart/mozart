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



proc{Alpha Root}
   A B C D E F G H I J K L M
   N O P Q R S T U V W X Y Z
in
   Root = [A B C D E F G H I J K L M
	   N O P Q R S T U V W X Y Z]
   %{GFD.dom 1#26 Root}
   Root ::: 1#26
   %{GFD.linearP post([B A L L E T] '=:' 45 cl:GFD.cl.val)}
   {GFD.sum [B A L L E T] '=:' 45}
   {GFD.sum [C E L L O] '=:' 43}
   {GFD.sum [C O N C E R T] '=:' 74}
   {GFD.sum [F L U T E] '=:' 30}
   {GFD.sum [F U G U E] '=:' 50}
   {GFD.sum [G L E E] '=:' 66}
   {GFD.sum [J A Z Z] '=:' 58}
   {GFD.sum [L Y R E] '=:' 47}
   {GFD.sum [O B O E] '=:' 53}
   {GFD.sum [O P E R A]'=:' 65}
   {GFD.sum [P O L K A] '=:' 59}
   {GFD.sum [Q U A R T E T] '=:' 50}
   {GFD.sum [S A X O P H O N E] '=:' 134}
   {GFD.sum [S C A L E] '=:' 51}
   {GFD.sum [S O L O] '=:' 37}
   {GFD.sum [S O N G] '=:' 61}
   {GFD.sum [S O P R A N O] '=:' 82}
   {GFD.sum [T H E M E] '=:' 72}
   {GFD.sum [V I O L I N] '=:' 100}
   {GFD.sum [W A L T Z] '=:' 34}

   {GFD.distinctP post(Root cl:GFD.cl.val)}
   {GFD.distribute ff Root}
end

{Show {SearchOne Alpha}}
