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
   {GFD.dom 1#26 Root}
   {GFD.linearP post([B A L L E T] GFD.rt.'=:' 45 cl:GFD.cl.val)}
   {GFD.linearP post([C E L L O] GFD.rt.'=:' 43 cl:GFD.cl.val)}
   {GFD.linearP post([C O N C E R T] GFD.rt.'=:' 74 cl:GFD.cl.val)}
   {GFD.linearP post([F L U T E] GFD.rt.'=:' 30 cl:GFD.cl.val)}
   {GFD.linearP post([F U G U E] GFD.rt.'=:' 50 cl:GFD.cl.val)}
   {GFD.linearP post([G L E E] GFD.rt.'=:' 66 cl:GFD.cl.val)}
   {GFD.linearP post([J A Z Z] GFD.rt.'=:' 58 cl:GFD.cl.val)}
   {GFD.linearP post([L Y R E] GFD.rt.'=:' 47 cl:GFD.cl.val)}
   {GFD.linearP post([O B O E] GFD.rt.'=:' 53 cl:GFD.cl.val)}
   {GFD.linearP post([O P E R A] GFD.rt.'=:' 65 cl:GFD.cl.val)}
   {GFD.linearP post([P O L K A] GFD.rt.'=:' 59 cl:GFD.cl.val)}
   {GFD.linearP post([Q U A R T E T] GFD.rt.'=:' 50 cl:GFD.cl.val)}
   {GFD.linearP post([S A X O P H O N E] GFD.rt.'=:' 134 cl:GFD.cl.val)}
   {GFD.linearP post([S C A L E] GFD.rt.'=:' 51 cl:GFD.cl.val)}
   {GFD.linearP post([S O L O] GFD.rt.'=:' 37 cl:GFD.cl.val)}
   {GFD.linearP post([S O N G] GFD.rt.'=:' 61 cl:GFD.cl.val)}
   {GFD.linearP post([S O P R A N O] GFD.rt.'=:' 82 cl:GFD.cl.val)}
   {GFD.linearP post([T H E M E] GFD.rt.'=:' 72 cl:GFD.cl.val)}
   {GFD.linearP post([V I O L I N] GFD.rt.'=:' 100 cl:GFD.cl.val)}
   {GFD.linearP post([W A L T Z] GFD.rt.'=:' 34 cl:GFD.cl.val)}

   {GFD.distinctP post(Root cl:GFD.cl.val)}
   {GFD.distribute ff Root}
end

{Show {SearchOne Alpha}}
