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
   Application
   GTK at 'x-oz://system/gtk/GTK.ozf'
define
   Titles = ["Column 1" "Column 2"]
   Row1  = ["Apple" "Orange"]
   Row2  = ["Car" "Truck"]
   Row3  = ["Airplane" "Bird"]

   %% Every Event needs an Argument Description.
   %% Legal Values are
   %% int -> Argument is a Integer
   %% float -> Argument is a Double (Float)
   %% pointer -> Argument is a arbitrary pointer
   %% obj(Class) -> Argument is a object of Class
   %% This is only appropriate if an Oz Wrapper class is present
   %% Otherwise use plain pointer instead.
   RowEventDesc = [pointer int]
   
   class MyTree from GTK.cTree
      meth new
	 N1
      in
	 GTK.cTree, newWithTitles(2 0 {GTK.makeStrArr Titles})
	 N1 = GTK.cTree, insertNode(unit unit {GTK.makeStrArr Row1}
				    0 unit unit unit unit 0 0 $)
	 GTK.cTree, insertNode(N1 unit {GTK.makeStrArr Row2}
			       0 unit unit unit unit 0 0 _)
	 GTK.cTree, insertNode(unit unit {GTK.makeStrArr Row3}
			       0 unit unit unit unit 0 0 _)
	 GTK.cTree, signalConnect('tree_select_row' myEvent RowEventDesc _)
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
	 GTK.window, signalConnect('delete_event' deleteEvent nil _)
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("CList Test")
	 
	 GTK.window, add(LObj)
	 GTK.window, showAll
      end
      meth deleteEvent(Args)
	 %% Caution: At this time, the underlying GTK object
	 %% Caution: has been destroyed already
	 %% Caution: Destruction also includes all attached child objects.
	 %% Caution: This event is solely intended to do OZ side
	 %% Caution: cleanup via calling close
	 {System.show 'delete Event occured'}
	 {self close}
	 {Application.exit 0}
      end
   end

   _ = {New MyToplevel new}
end
