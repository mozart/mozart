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
   Application(exit)
   GDK    at 'x-oz://system/gtk/GDK.ozf'
   GTK    at 'x-oz://system/gtk/GTK.ozf'
   Canvas at 'x-oz://system/gtk/GTKCANVAS.ozf'
define
   %% Create Toplevel window class
   class CanvasToplevel from GTK.window
      meth new
	 GTK.window, new(GTK.'WINDOW_TOPLEVEL')
	 GTK.window, setBorderWidth(10)
	 GTK.window, setTitle("Hello Image")
	 {self signalConnect('delete-event' deleteEvent _)}
      end
      meth deleteEvent(Args)
	 {self gtkClose}
	 {Application.exit 0}
      end
   end
   Toplevel = {New CanvasToplevel new}

   %% Prepare to load image
   %% Static Methods are also encapulated into objects
   %% Therefore, a GDK.imlib object is created using noop constructor
   %% Hack Alert: Image must reside in current directory
   Image = {{New GDK.imlib noop} loadImage("mozart-259x112.jpg" $)}

   %% Retrieve Image Dimension
   ImageX = {Image imlibImageGetFieldRgbWidth($)}
   ImageY = {Image imlibImageGetFieldRgbHeight($)}

   %% Setup canvas with Image Support
   %% This yields implicit pushVisual call
   MyCanvas = {New Canvas.canvas new(true)}
   Root     = {MyCanvas root($)}

   %% Setup appropriate Canvas Dimensions
   {MyCanvas setUsize(ImageX ImageY)}
   {MyCanvas setScrollRegion(0.0 0.0
			     {Int.toFloat ImageX}
			     {Int.toFloat ImageY})}

   %% Make Canvas child of toplevel
   {Toplevel add(MyCanvas)}
   %% Create our item (member of root group); ignore item object
   %% Note: The canvas is able to scale the image
   _ = {MyCanvas newImageItem(Root Image
			      0 0 ImageX ImageY GTK.'ANCHOR_NORTH_WEST' $)}
   %% Pop the visual stuff after all image items have been created
   {MyCanvas popVisual}
   
   %% Make it all visible
   {Toplevel showAll}
end
