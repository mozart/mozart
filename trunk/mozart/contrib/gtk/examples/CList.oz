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
   GTK
define
   Titles = ["Column 1" "Column 2"]
   Row1  = ["Apple" "Orange"]
   Row2  = ["Car" "Truck"]
   Row3  = ["Airplane" "Bird"]
   
   class MyList from GTK.cList
      meth new
	 GTK.cList, newWithTitles(2 {GTK.makeStrArr Titles})
	 GTK.cList, append({GTK.makeStrArr Row1} _)
	 GTK.cList, append({GTK.makeStrArr Row2} _)
	 GTK.cList, append({GTK.makeStrArr Row3} _)
      end
      meth getPos(X Y $)
	 Arr = {GTK.allocStr 1024}
      in
	 {self getText(X Y Arr _)}
	 {String.toAtom {GTK.getStr}}
      end
   end

   class MyToplevel from GTK.window
      meth new
	 LObj = {New MyList new}
      in
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("CList Test")
	 
	 GTK.window, add(LObj)
	 GTK.window, showAll
	 {System.show {LObj getPos(0 0 $)}}
      end
   end

   _ = {New MyToplevel new}
end
