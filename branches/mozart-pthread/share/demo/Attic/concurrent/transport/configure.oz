%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%


%% Trucks
Capacity  = 100 % Capacity of a truck
Delta     =   5 % How many pixels per movement
DelayMove = 100 % Delay in milliseconds between each move
   

%% Randomizer
local
   Speed = (1000 * Delta) div DelayMove % Relative speed
in
   MedLowTime   = 20 * Speed        % Lower bound for random time interval
   MedHighTime  = 3 * MedLowTime    % Upper bound for random time interval
   SlowLowTime  = MedLowTime div 2  % Lower bound for random time interval
   SlowHighTime = MedHighTime div 2 % Upper bound for random time interval
   FastLowTime  = 2 * MedLowTime    % Lower bound for random time interval
   FastHighTime = 2 * MedHighTime   % Upper bound for random time interval
end
   
LowGood  = Capacity div 10 % Lower bound for random weight of goods
HighGood = Capacity        % Upper bound for random weight of goods


%% Colors
BackColor # StreetColor # CityColor #
TruckColors # FrameColor # WinColor # GoodColor =  
if Tk.isColor then
   aquamarine # black # red #
   [red blue yellow green plum cyan tan bisque] #
   black # steelblue # brown
else
   white # black # black #
   [white] #
   black # white # black
end

local
   %% Somehow append is buggy, check!
   fun {Link Cs}
      case Cs of nil then CycColors
      [] C|Cr then C|{Link Cr}
      end
   end
   CycColors = {Link TruckColors}
   ColMan    = {New class $ from BaseObject
		       prop final
		       attr ColS: CycColors
		       meth get(?Col) ColR in Col|ColR=(ColS<-ColR) end
		    end noop}
in
   fun {GetFillColor}
      {ColMan get($)}
   end
end

%% Where to find the bitmaps?
local
   Url = 'http://www.ps.uni-sb.de/ozhome/demo/images/transport/'
   RI  = {TkTools.images
	  {Map [down
		truck_frame_left truck_frame_right
		truck_win_left truck_win_right]
	   fun {$ B}
	      Url#B#'.xbm'
	   end}}
   {RI.truck_frame_left  tk(configure foreground:FrameColor)}
   {RI.truck_frame_right tk(configure foreground:FrameColor)}
   {RI.truck_win_left  tk(configure foreground:WinColor)}
   {RI.truck_win_right tk(configure foreground:WinColor)}
   CI  = {List.toRecord c
	  {Map TruckColors
	   fun {$ C}
	      I = {TkTools.images [Url#'truck_fill_right.xbm'
				   Url#'truck_fill_left.xbm']}
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


LoadLeftX   =  ~3.0
LoadRightX  = ~24.0
LoadY       =  ~8.0
LoadHeight  =   6.0
LoadWidth   =  27.0

%% Dialogs
TitleName = 'Transportation'
AboutFont = '-adobe-times-bold-r-normal-*-*-240*'


%% Misc
Pad           = 2
BigPad        = 4
TextWidth     = 4
BigTextWidth  = 17
TextHeight    = 6
TextFont      = '-adobe-helvetica-medium-r-normal-*-*-120*'
TextBg        = wheat
HistoryWidth  = 60
HistoryHeight = 8
HistoryFont   = '-adobe-helvetica-medium-r-normal-*-*-100*'
HistoryBg     = '#fffff0'

%% Predefined goods
Goods = bananas # cheese # cream # computers # rye # oil # sugar # salt #
        vinegar # apples # whisky # beer # garbage # ketchup # coffee # ham #
        umbrellas # paper # books # yoghurt # engines # oranges # juice #
        tea # jam # grease # cigarettes # toys # corn # hamburgers


%% Default companies and drivers

DefaultScenario = d('Disney': ['Mickey'('Düsseldorf') 'Goofy'('Berlin')]
		    'Oz':     ['Tinman'('München') 'Toto'('Saarbrücken')])

