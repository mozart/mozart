%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%% Global default settings

FactorsToCm      =cTo(i: 2.54
		      c: 1.00
		      m: 10.0
		      p: 0.035277778)

StatusUpdateCnt = 50

StartSizeX   = 500
StartSizeY   = 300
MinSizeX     = 360
MinSizeY     = 260

%% Configuration of the scale bar
IntScaleBase    = 256
FloatScaleBase  = {IntToFloat IntScaleBase}
MinScale = 0.05 / FloatScaleBase
MaxScale = 2.00 / FloatScaleBase
DefScale = 1.00 / FloatScaleBase

AboutFont       = '-Adobe-times-bold-r-normal--*-240*'
FontFamily      = '-*-helvetica-medium-r-normal--*-'
BoldFontFamily  = '-*-helvetica-bold-r-normal--*-'
FontMatch       = '-*-*-*-*-*-*'
NumberFonts     = [font(name:FontFamily#180#FontMatch scale:1.8/FloatScaleBase)
		   font(name:FontFamily#140#FontMatch scale:1.4/FloatScaleBase)
		   font(name:FontFamily#120#FontMatch scale:1.2/FloatScaleBase)
		   font(name:FontFamily#100#FontMatch scale:1.0/FloatScaleBase)
		   font(name:FontFamily# 80#FontMatch scale:0.8/FloatScaleBase)]
StatusFont      = FontFamily     # 100 # FontMatch
BoldStatusFont  = BoldFontFamily # 100 # FontMatch

%% Width of the scroller, applies also to the width of the scale
ScrollerWidth   = 13

%% Some parameters used in packing the canvas
Pad             = 2
BigPad          = 4
HugePad         = 6
LargeEntryWidth = 20
SmallEntryWidth = 6

%% Distance between nodes
HorSpaceI        = 32 * IntScaleBase
VerSpaceI        = 38 * IntScaleBase
VerSpaceF        = {IntToFloat VerSpaceI}

HalfHorSpaceI    = HorSpaceI div 2
HalfVerSpaceI    = VerSpaceI div 2
HalfHorSpaceF    = {IntToFloat HalfHorSpaceI}
HalfVerSpaceF    = {IntToFloat HalfVerSpaceI}

%% Initial coordinates of the root of the tree
RootX            = 0
RootY            = HalfVerSpaceI
SentinelX        = RootX
SentinelY        = RootY - VerSpaceI

%% Sizes for the nodes and links
CircleWidthI         = 10 * IntScaleBase
CircleWidthF         = {IntToFloat CircleWidthI}
RectangleWidthI      =  8 * IntScaleBase
RectangleWidthF      = {IntToFloat RectangleWidthI}
SmallRectangleWidthI =  6 * IntScaleBase
SmallRectangleWidthF = {IntToFloat SmallRectangleWidthI}
RhombeWidthI         = 10 * IntScaleBase
RhombeWidthF         = {IntToFloat RhombeWidthI}

ImageSize           = 16.0
ImageCenter         = ImageSize / 2.0
ImageScale          = 0.6 / FloatScaleBase
MaxExtent           = 12.0 * FloatScaleBase

PopUpDelay = 2000

   
NoLabel       = {NewName}
ManagerClosed = {NewName}

ActionKinds   = [information compare statistics]
ActionTypes   = [root space procedure]
ActionArities = a(information: [2 3]
		  compare:     [4 5]
		  statistics:  [2 3])

   
