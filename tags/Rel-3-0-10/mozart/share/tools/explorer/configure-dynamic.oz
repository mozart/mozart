%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

DefOptions = o(drawing:    o(hide:   true
			     scale:  false
			     update: 10)
	       search:     o(search:      1
			     information: 5
			     failed:      true)
	       postscript: o(color:       if Tk.isColor then color
					  else mono
					  end
			     width:       6.5 * FactorsToCm.i
			     height:      9.5 * FactorsToCm.i
			     size:        '6.5ix9i'
			     orientation: false))

NodeBorderWidth      #
ThickNodeBorderWidth = if Tk.isColor then 1#1 else 1#2 end
LinkWidth            = 1

%% How big and how far removed should the cursor shade be?
ShadeWidth          = if Tk.isColor then 4 else 5 end * IntScaleBase
ShadeScale          = if Tk.isColor then 1.05 else 1.10 end

%% Set up some colors
ChooseColor          #
ChooseTermColor      #
EntailedColor        #
SuspendedColor       #
FailedColor          #
BlockedColor         #
PartialFailedColor   #
LineColor            #
BackColor            #
CursorColor          #
PopupBgColor         = if Tk.isColor then
			  'lightskyblue3'   # % ChooseColor
			  'LightSlateBlue'  # % ChooseTermColor
			  'MediumSeaGreen'  # % EntailedColor
			  'olivedrab1'      # % SuspendedColor
			  'firebrick2'      # % FailedColor
			  'orange'          # % BlockedColor
			  'purple1'         # % PartialFailedColor
			  black             # % LineColor
			  white             # % BackColor
			  gray60            # % CursorColor
			  ivory               % PopupBgColor
		       else
			  white # % ChooseColor
			  white # % ChooseTermColor
			  white # % EntailedColor
			  white # % SuspendedColor
			  white # % FailedColor
			  white # % BlockedColor
			  white # % PartialFailedColor
			  black # % LineColor
			  white # % BackColor
			  black # % CursorColor
			  white   % PopupBgColor
		       end


   
