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
   GDK at 'x-oz://system/gtk/GDK.ozf'
   GTK at 'x-oz://system/gtk/GTK.ozf'
define
   %% Create Toplevel window class
   class MyToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Hello GTK")
      end
   end

   %% Set up the Colors
   Black = {GDK.makeColor '#000000'}
   White = {GDK.makeColor '#FFFFFF'}

   %% Create TextWidget class
   class MyText from GTK.text
      meth new
	 GTK.text, new(unit unit)
      end
   end

   Toplevel = {New MyToplevel new}
   Text     = {New MyText new}

   %% Make TextWidget child of Toplevel
   {Toplevel add(Text)}
   {Text insert(unit Black White "Hallo, Leute!" ~1)}
   %% Make it all visible
   {Toplevel showAll}
end
