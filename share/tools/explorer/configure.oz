%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

%% Get our colormodel
IsColor          = ({Tk.depth} > 1)


%% Global default settings

DefLayoutOptions = o(wait:   !False
		     hide:   !False
		     update: 10)

DefSearchOptions = o(dist: 1)

DefInfoOptions   = o(dist: 5
		     keep: !True)

FactorsToCm      =cTo(i: 2.54
		      c: 1.00
		      m: 10.0
		      p: 0.035277778)

DefPostscriptOptions = o(width:  6.5 * FactorsToCm.i
			 height: 9.5 * FactorsToCm.i
			 size:   '6.5ix9i'
			 color:  case IsColor then color else mono end
			 orient: 0)

DefDepthNumbers     = n(set: [10 50 100 250 ~1]
			inc: [5 25 50 100 250]
			def: ~1)
DefNodesNumbers     = n(set: [500 1000 5000 10000 ~1]
			inc: [250 1000 2500 5000]
			def: 5000)

DefRecomputeMax   = 100


ErrorAspect   = 250
StatusUpdateCnt = 50

TitleName    = 'Oz Explorer'
BitMapDir    = System.ozHome # '/lib/bitmaps/'
BitMap       = '@' # BitMapDir # 'explorer.xbm'

MinSizeX     = 580   CanvasWidth  = 500.0
MinSizeY     = 380   CanvasHeight = 240.0 


%% Configuration of the scale bar
MinScalePercent =   2
MaxScalePercent = 200
DefScalePercent = 100
InitialScale    = 1.0

AboutFont       = '-Adobe-times-bold-r-normal--*-240*'
FontFamily      = '-*-helvetica-medium-r-normal--*-'
BoldFontFamily  = '-*-helvetica-bold-r-normal--*-'
FontMatch       = '-*-*-*-*-*-*'
NumberFonts     = [font(name:FontFamily#180#FontMatch scale:1.8)
		   font(name:FontFamily#140#FontMatch scale:1.4)
		   font(name:FontFamily#120#FontMatch scale:1.2)
		   font(name:FontFamily#100#FontMatch scale:1.0)
		   font(name:FontFamily# 80#FontMatch scale:0.8)]
StatusFont      = FontFamily#100#FontMatch
BoldStatusFont  = BoldFontFamily#100#FontMatch

%% Width of the scroller, applies also to the width of the scale
ScrollerWidth   = 12
ScaleWidth      = 12

%% Some parameters used in packing the canvas
Border       = 2
BigBorder    = 4
Pad          = 2
BigPad       = 4
HugePad      = 8

%% Distance between nodes
HorSpace         = 35.0
VerSpace         = 40.0

HalfHorSpace     = HorSpace / 2.0
HalfVerSpace     = VerSpace / 2.0

%% Initial coordinates of the root of the tree
RootX            = 0.0
RootY            = HalfVerSpace


%% Get our colormodel
IsColor          = ({Tk.depth} > 1)


%% Sizes for the nodes and links
CircleWidth         = 10.0
TriangleWidth       =  8.0
RectangleWidth      =  8.0
SmallRectangleWidth =  6.0
RhombeWidth         = 10.0
ImageSize           = 18.0
ImageCenter         = ImageSize / 2.0
ImageScale          = 0.6
ImageBorder         = 1.0
MaxExtent           = 12.0

NodeBorderWidth     #
TermNodeBorderWidth = case IsColor then 1#1 else 1#2 end
LinkWidth           = 1

%% How big and how far removed should the cursor shade be?
ShadeWidth          = case IsColor then 4.00 else 5.00 end
ShadeScale          = case IsColor then 1.05 else 1.10 end

%% Set up some colors
ChoiceColor          #
ChoiceTermColor      #
EntailedColor        #
StableColor          #
FailedColor          #
UnstableColor        #
PartialFailedColor   #
LineColor            #
BackColor            #
EntryColor           #
CursorColor          = case IsColor then
			  'LightSlateBlue' # % ChoiceColor
			  'DarkSlateBlue'  # % ChoiceTermColor
			  'MediumSeaGreen' # % EntailedColor
			  'DarkSeaGreen'   # % StableColor
			  'firebrick2'     # % FailedColor
			  'orange'         # % UnstableColor
			  'purple1'        # % PartialFailedColor
			  black            # % LineColor
			  white            # % BackColor
			  wheat            # % EntryColor
			  gray60             % CursorColor
		       else
			  white # % ChoiceColor
			  white # % ChoiceTermColor
			  white # % EntailedColor
			  white # % StableColor
			  white # % FailedColor
			  white # % UnstableColor
			  white # % PartialFailedColor
			  black # % LineColor
			  white # % BackColor
			  white # % EntryColor
			  black   % CursorColor 
		       end



