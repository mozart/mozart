%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   System(show)
   GTK at 'x-oz://system/gtk/GTK.ozf'
define
   Titles = ["Column 1" "Column 2"]
   Row1  = ["Apple" "Orange"]
   Row2  = ["Car" "Truck"]
   Row3  = ["Airplane" "Bird"]

   class MyTree from GTK.cTree
      meth new
	 N1
      in
	 GTK.cTree, newWithTitles(2 0 Titles)
	 N1 = GTK.cTree, insertNode(unit unit Row1
				    0 unit unit unit unit false false $)
	 GTK.cTree, insertNode(N1 unit Row2
			       0 unit unit unit unit false false _)
	 GTK.cTree, insertNode(unit unit Row3
			       0 unit unit unit unit false false _)
	 GTK.cTree, signalConnect('tree-select-row' myEvent _)
      end
      meth myEvent(Args)
	 %% Arguments are given as a list
	 {System.show 'tree_select_row event:'#Args}
      end
   end

   class MyToplevel from GTK.window
      meth new
	 LObj = {New MyTree new}
      in
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("CList Test")
	 GTK.window, add(LObj)
	 GTK.window, showAll
      end
   end

   _ = {New MyToplevel new}
end
