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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

Pad            = 4

MegaByteF      = 1024.0 * 1024.0
KiloByteI      = 1024
MegaByteI      = KiloByteI * KiloByteI

BitMapDir      = '@' # {System.get home} # '/lib/bitmaps/'

LineColor        #
RunnableColor    # RunnableStipple  #
ThresholdColor   # ThresholdStipple #
SizeColor        # SizeStipple      #
ActiveColor      # ActiveStipple    #
TimeColors       # TimeStipple      #
AboutColor       # CurLoadColor     =
case Tk.isColor then
   gray80 #
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
   black #
   black # (BitMapDir # 'grid-50.xbm')  #
   black # (BitMapDir # 'grid-25.xbm')  #
   black # (BitMapDir # 'grid-50.xbm') #
   black # '' #
   color(run:    black
	 'prop': black
	 copy:   black
	 gc:     black
	 load:   black) #
   stipple(run:    BitMapDir # 'grid-25.xbm'
	   'prop': BitMapDir # 'grid-50.xbm'
	   copy:   BitMapDir # 'lines-lr.xbm'
	   gc:     BitMapDir # 'lines-rl.xbm'
	   load:   BitMapDir # 'zig-zag.xbm') #
   black #
   black
end

AboutFont = '-Adobe-times-bold-r-normal--*-240*'

TitleName = 'Oz Panel'

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

BoldFontFamily   = '-*-helvetica-bold-r-normal--*-'
MediumFontFamily = '-*-helvetica-medium-r-normal--*-'
FontMatch        = '-*-*-*-*-*-*'
BoldFont         = BoldFontFamily   # 120 # FontMatch
MediumFont       = MediumFontFamily # 120 # FontMatch

ZeroTime     = time(copy:      0
		    gc:        0
		    load:      0
		    propagate: 0
		    run:       0
		    system:    0
		    user:      0
		    total:     0)
