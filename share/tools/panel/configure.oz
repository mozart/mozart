%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


PanelWidth       = 530
FrameWidth       = PanelWidth - 20
FullPanelHeight  = 335
PartPanelHeight  = 240

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

EnterColor     = wheat

RunnableColor  = lightslateblue

ThresholdColor = lightslateblue
SizeColor      = mediumvioletred
ActiveColor    = mediumaquamarine

TimeColors   = colors(run:  lightslateblue
		      prop: mediumvioletred
		      copy: mediumaquamarine
		      gc:   mediumseagreen
		      load: wheat)

AboutColor = blue

AboutFont       = '-Adobe-times-bold-r-normal--*-240*'

TitleName = 'Oz Panel'


BitMap = '@' # {System.get home} # '/lib/bitmaps/panel.xbm'

SampleTimes = [500   # '500ms'
	       1000  # '1s'
	       2000  # '2s'
	       5000  # '5s'
	       10000 # '10s'
	       60000 # '1m']

BoldFontFamily  = '-*-helvetica-bold-r-normal--*-'
FontMatch       = '-*-*-*-*-*-*'
ScaleFont   = BoldFontFamily#100#FontMatch

