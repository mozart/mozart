%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
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

functor
import
   Tk
export
   display:Display
define
   proc{Display Title Mess CloseMess}
      T={New Tk.toplevel tkInit(title:Title withdraw:true)}
      M={New Tk.message tkInit(parent:T aspect:400
			       relief:groove bd:2
			       text:Mess)}
      B1={New Tk.button tkInit(parent:T relief:groove
			       text:CloseMess action:T#tkClose)}
   in
      {Tk.batch [grid(M row:0 column:0 sticky:news)
		 grid(B1 row:1 column:0 sticky:we)
		 grid(columnconfigure T 0 weight:1)
		 wm(resizable T 1 0)
		 wm(deiconify T)
	     ]}
   end
in
   skip
end

/*

{Display "Micq Help "
 "Hello!\nThis is a cool small display window....\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "bla************************************************************\n"#
 "I don't know how many lines of text we can display\n\n!\n\n/Nils"
 "Close Help"}

*/

