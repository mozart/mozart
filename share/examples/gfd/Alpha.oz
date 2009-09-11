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
   Root ::: 1#26
   {GFD.linearP post([B A L L E T] '=:' 45 cl:val)}
   {GFD.linearP post([C E L L O] '=:' 43 cl:val)}
   {GFD.linearP post([C O N C E R T] '=:' 74 cl:val)}
   {GFD.linearP post([F L U T E] '=:' 30 cl:val)}
   {GFD.linearP post([F U G U E] '=:' 50 cl:val)}
   {GFD.linearP post([G L E E] '=:' 66 cl:val)}
   {GFD.linearP post([J A Z Z] '=:' 58 cl:val)}
   {GFD.linearP post([L Y R E] '=:' 47 cl:val)}
   {GFD.linearP post([O B O E] '=:' 53 cl:val)}
   {GFD.linearP post([O P E R A]'=:' 65 cl:val)}
   {GFD.linearP post([P O L K A] '=:' 59 cl:val)}
   {GFD.linearP post([Q U A R T E T] '=:' 50 cl:val)}
   {GFD.linearP post([S A X O P H O N E] '=:' 134 cl:val)}
   {GFD.linearP post([S C A L E] '=:' 51 cl:val)}
   {GFD.linearP post([S O L O] '=:' 37 cl:val)}
   {GFD.linearP post([S O N G] '=:' 61 cl:val)}
   {GFD.linearP post([S O P R A N O] '=:' 82 cl:val)}
   {GFD.linearP post([T H E M E] '=:' 72 cl:val)}
   {GFD.linearP post([V I O L I N] '=:' 100 cl:val)}
   {GFD.linearP post([W A L T Z] '=:' 34 cl:val)}

   {GFD.distinctP post(Root cl:val)}
   {GFD.distribute ff Root}
end

{Show {SearchOne Alpha}}
