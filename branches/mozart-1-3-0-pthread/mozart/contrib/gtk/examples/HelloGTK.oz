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
   Application
   System(show)
   GTK at 'x-oz://system/gtk/GTK.ozf'
define
   %% Create Toplevel window class
   class MyToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, signalConnect('delete_event' deleteEvent nil _)
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Hello GTK")
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

   %% Create Button class
   class MyButton from GTK.button
      meth new
	 GTK.button, newWithLabel("Hello, GTK!")
	 GTK.button, signalConnect('clicked' clickedEvent nil _)
      end
      meth clickedEvent(Args)
	 {System.show 'ClickedEvent occured'}
      end
   end

   Toplevel = {New MyToplevel new}
   Button   = {New MyButton new}

   %% Make Butten child of Toplevel
   {Toplevel add(Button)}
   %% Make it all visible
   {Toplevel showAll}
end
