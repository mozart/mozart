%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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

require
   DemoUrls(image: ImageDir) at '../DemoUrls.ozf'

import
   Tk
   TkTools
   
export
   Images
   Capacity
   Delta
   delay: DelayMove
   Colors
   Title
   Fonts
   Goods
   
prepare

   Title = 'Transportation'
   
   Capacity  = 100 % Capacity of a truck
   Delta     =   5 % How many pixels per movement
   DelayMove = 100 % Delay in milliseconds between each move
   

   FullColors = colors(back:   aquamarine
		       street: black
		       city:   red
		       truck:  [red blue yellow green
				plum cyan tan bisque]
		       frame:  black
		       window: steelblue
		       good:   brown
		       textBg: wheat)

   BwColors   = colors(back:   white
		       street: black
		       city:   black
		       truck:  [white]
		       frame:  black
		       window: white
		       good:   black
		       textBg: white)

   fun {Namify B}
      ImageDir#'transport/'#B#'.xbm'
   end

   %% Predefined goods
   Goods = bananas # cheese # cream # computers # rye # oil # sugar # salt #
   vinegar # apples # whisky # beer # garbage # ketchup # coffee # ham #
   umbrellas # paper # books # yoghurt # engines # oranges # juice #
   tea # jam # grease # cigarettes # toys # corn # hamburgers


define
   
   %% Fonts
   Fonts = fonts(text:
		    {New Tk.font tkInit(family:helvetica size:~12)}
		 about:
		    {New Tk.font tkInit(family:times size:~24 weight:bold)})
   

   Colors = if Tk.isColor then FullColors else BwColors end

   %% Where to find the bitmaps?
   local
      RI  = {TkTools.images {Map [down
				  truck_frame_left truck_frame_right
				  truck_win_left truck_win_right]
			     Namify}}
      {RI.truck_frame_left  tk(configure foreground:Colors.frame)}
      {RI.truck_frame_right tk(configure foreground:Colors.frame)}
      {RI.truck_win_left    tk(configure foreground:Colors.window)}
      {RI.truck_win_right   tk(configure foreground:Colors.window)}
      CI  = {List.toRecord c
	     {Map Colors.truck
	      fun {$ C}
		 I = {TkTools.images {Map [truck_fill_right
					   truck_fill_left] Namify}}
	      in
		 {I.truck_fill_left  tk(configure foreground:C)}
		 {I.truck_fill_right tk(configure foreground:C)}
		 C#c(left:  I.truck_fill_left
		     right: I.truck_fill_right)
	      end}}
   in
      Images = images(down:  RI.down
		      truck: t(fill:  CI
			       frame: f(left:  RI.truck_frame_left
					right: RI.truck_frame_right)
			       win:   w(left:  RI.truck_win_left
					right: RI.truck_win_right)))
      
   end

end

