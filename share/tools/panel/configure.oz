%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

SmallPad     = 2
Pad          = 4

KiloByteF      = 1024.0
MegaByteF      = KiloByteF * KiloByteF
KiloByteI      = 1024
MegaByteI      = KiloByteI * KiloByteI

SecondI        = 1000
SecondF        = {IntToFloat SecondI}
MinuteI        = 60 * SecondI
MinuteF        = {IntToFloat MinuteI}
HourI          = 60 * MinuteI
HourF          = {IntToFloat HourI}

BitMapDir      = '@' # {System.get home} # '/lib/bitmaps/'

DashLine       = BitMapDir # 'dash-line.xbm'

EnterColor       #
RunnableColor    # RunnableStipple  #
ThresholdColor   # ThresholdStipple #
SizeColor        # SizeStipple      #
ActiveColor      # ActiveStipple    #
TimeColors       # TimeStipple      # 
AboutColor       # CurLoadColor     =
case Tk.isColor then
   wheat #
   lightslateblue   # '' #
   lightslateblue   # '' #
   mediumvioletred  # '' #
   mediumaquamarine # '' #
   color(run:  yellow4
	 'prop': mediumvioletred
	 copy: mediumaquamarine
	 gc:   mediumseagreen
	 load: wheat) #
   stipple(run:    ''
	   'prop': ''
	   copy:   ''
	   gc:     ''
	   load:   '') #
   blue #
   lightslateblue 
else
   white #
   black # (BitMapDir # 'grid-50.xbm')  #
   black # (BitMapDir # 'grid-25.xbm')  #
   black # (BitMapDir # 'grid-50.xbm') #
   black # '' #
   color(run:  black
	 'prop': black
	     copy: black
	     gc:   black
	     load: black) #
   stipple(run:  BitMapDir # 'grid-25.xbm'
	   'prop': BitMapDir # 'grid-50.xbm'
	       copy: BitMapDir # 'lines-lr.xbm'
	       gc:   BitMapDir # 'lines-rl.xbm'
	       load: BitMapDir # 'zig-zag.xbm') #
   black #
   black
end

AboutFont       = '-Adobe-times-bold-r-normal--*-240*'

TitleName = 'Oz Panel'


BitMap = BitMapDir # 'panel.xbm'

UpdateTimes         = [500   # '500ms'
		       1000  # '1s'
		       5000  # '5s'
		       10000 # '10s']
DefaultUpdateTime   = 1000

HistoryRanges       = [10000  # '10s'
		       30000  # '30s'
		       60000  # '1m'
		       120000 # '2m']
DefaultHistoryRange = 60000

LoadWidth           = 240

BoldFontFamily  = '-*-helvetica-bold-r-normal--*-'
FontMatch       = '-*-*-*-*-*-*'
ScaleFont       = BoldFontFamily#100#FontMatch

ZeroTime     = time(copy:      0 
		    gc:        0 
		    load:      0
		    propagate: 0 
		    run:       0
		    system:    0
		    user:      0
		    total:     0)


