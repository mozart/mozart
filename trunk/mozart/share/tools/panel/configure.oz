%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


PanelWidth   = 640
FrameWidth   = PanelWidth - 30
PanelHeight  = 370

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


BigPad = 4

BitMap = '@' # {System.get home} # '/lib/bitmaps/panel.xbm'