%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

%% Global default settings

FactorsToCm      =cTo(i: 2.54
		      c: 1.00
		      m: 10.0
		      p: 0.035277778)

Options = o(drawing:    o(hide:   !True
			  scale:  !False
			  update: 10)
	    search:     o(search:      1
			  information: 5
			  failed:      !True)
	    postscript: o(color:       case Tk.isColor then color else mono end
			  width:       6.5 * FactorsToCm.i
			  height:      9.5 * FactorsToCm.i
			  size:        '6.5ix9i'
			  orientation: !False))

ErrorAspect     = 250
StatusUpdateCnt = 50

TitleName    = 'Oz Explorer'
BitMapDir    = {System.get home} # '/lib/bitmaps/'
BitMap       = '@' # !BitMapDir # 'explorer.xbm'

MinSizeX     = 500
MinSizeY     = 300
DepthWidth   = 120 
DepthHeight  = 8

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
NumberFonts     = [font(name:!FontFamily#180#!FontMatch scale:1.8/FloatScaleBase)
		   font(name:!FontFamily#140#!FontMatch scale:1.4/FloatScaleBase)
		   font(name:!FontFamily#120#!FontMatch scale:1.2/FloatScaleBase)
		   font(name:!FontFamily#100#!FontMatch scale:1.0/FloatScaleBase)
		   font(name:!FontFamily# 80#!FontMatch scale:0.8/FloatScaleBase)]
StatusFont      = !FontFamily     # 100 # !FontMatch
BoldStatusFont  = !BoldFontFamily # 100 # !FontMatch

%% Width of the scroller, applies also to the width of the scale
ScrollerWidth   = 12
ScaleWidth      = 12

%% Some parameters used in packing the canvas
Border          = 2
BigBorder       = 4
Pad             = 2
BigPad          = 4
HugePad         = 8
LargeEntryWidth = 20
SmallEntryWidth = 6

%% Distance between nodes
HorSpaceI        = 32 * IntScaleBase
VerSpaceI        = 38 * IntScaleBase
HorSpaceF        = {IntToFloat HorSpaceI}
VerSpaceF        = {IntToFloat VerSpaceI}

HalfHorSpaceI    = HorSpaceI div 2
HalfVerSpaceI    = VerSpaceI div 2
HalfHorSpaceF    = {IntToFloat HalfHorSpaceI}
HalfVerSpaceF    = {IntToFloat HalfVerSpaceI}

%% Initial coordinates of the root of the tree
RootX            = 0
RootY            = !HalfVerSpaceI
SentinelX        = !RootX
SentinelY        = RootY - VerSpaceI

%% Sizes for the nodes and links
CircleWidthI         = 10 * IntScaleBase
CircleWidthF         = {IntToFloat CircleWidthI}
TriangleWidthI       =  8 * IntScaleBase
TriangleWidthF       = {IntToFloat TriangleWidthI}
RectangleWidthI      =  8 * IntScaleBase
RectangleWidthF      = {IntToFloat RectangleWidthI}
SmallRectangleWidthI =  6 * IntScaleBase
SmallRectangleWidthF = {IntToFloat SmallRectangleWidthI}
RhombeWidthI         = 10 * IntScaleBase
RhombeWidthF         = {IntToFloat RhombeWidthI}

ImageSize           = 18.0
ImageCenter         = ImageSize / 2.0
ImageScale          = 0.6 / FloatScaleBase
ImageBorder         = 1
MaxExtent           = 12.0 * FloatScaleBase

NodeBorderWidth      #
ThickNodeBorderWidth = case Tk.isColor then 1#1 else 1#2 end
LinkWidth            = 1

%% How big and how far removed should the cursor shade be?
ShadeWidth          = case Tk.isColor then 4 else 5 end * IntScaleBase
ShadeScale          = case Tk.isColor then 1.05 else 1.10 end

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
EntryColor           #
DepthColor           #
CursorColor          = case Tk.isColor then
			  'lightskyblue3'   # % ChooseColor
			  'LightSlateBlue'  # % ChooseTermColor
			  'MediumSeaGreen'  # % EntailedColor
			  'olivedrab1'      # % SuspendedColor
			  'firebrick2'      # % FailedColor
			  'orange'          # % BlockedColor
			  'purple1'         # % PartialFailedColor
			  black             # % LineColor
			  white             # % BackColor
			  wheat             # % EntryColor
			  wheat             # % DepthColor
			  gray60              % CursorColor
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
			  white # % EntryColor
			  black # % DepthColor
			  black   % CursorColor 
		       end


NodePrefix = {String.toAtom {Tk.getPrefix}}
TreePrefix = {String.toAtom {Tk.getPrefix}}
LinkPrefix = {String.toAtom {Tk.getPrefix}}

	    
