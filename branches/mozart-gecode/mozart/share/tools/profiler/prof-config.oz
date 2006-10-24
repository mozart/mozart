%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

S2A = String.toAtom

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Text
%%

TitleName              = 'Oz Profiler'
IconName               = 'Profiler'

Platform               = {Property.get 'platform.name'}
WindowsPlatform        = 'win32-i486'

NameOfRalf             = 'Ralf Scheidhauer'
NameOfBenni            = 'Benjamin Lorenz'

EmailOfBoth            = '{scheidhr,lorenz}@ps.uni-sb.de'
EmailOfBenni           = 'lorenz@ps.uni-sb.de'

BarCanvasTitle         = 'Procedures'
ProcTextTitle          = 'Proc Info'
SumTextTitle           = 'Summary'

UpdateButtonText       = 'update'
ResetButtonText        = 'reset'
SortButtonText         = 'Sort By:'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Geometry
%%
ToplevelGeometry       = '450x284'

BarCanvasWidth         = 300
ProcTextWidth          = 24
ProcTextHeight         = 7
SumTextWidth           = 24
SumTextHeight          = 5

PadXButton             = 5
PadYButton             = 3

ScrollbarWidth         = 10


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Fonts
%%

SmallFont # SmallBoldFont #
DefaultFont # BoldFont =
if Platform == WindowsPlatform then
   {New Tk.font tkInit(family:courier size:12)} #
   {New Tk.font tkInit(family:courier weight:bold size:12)} #
   SmallFont #
   SmallBoldFont
else
   '6x13' # '6x13bold' # '7x13' # '7x13bold'
end

ButtonFont             = {New Tk.font tkInit(family:helvetica size:10)}
TitleFont              = {New Tk.font tkInit(family:helvetica size:10
					     weight:bold)}
StatusFont             = TitleFont
HelpTitleFont          = {New Tk.font tkInit(family:helvetica size:18
					     weight:bold)}
HelpFont               = {New Tk.font tkInit(family:helvetica size:12)}


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Miscellaneous
%%

TextCursor             = left_ptr
MaxEntries             = 50
TimeoutToStatus        = 210
UpdateTimes            = [0     # 'never'
			  2000  # '2s'
			  5000  # '5s'
			  10000 # '10s'
			  30000 # '30s']
HelpEvent              = '<3>'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Colors and colormodel related stuff
%%

DefaultBackground
DefaultForeground
SelectedBackground
SelectedForeground

if Tk.isColor andthen Platform \= WindowsPlatform then
   DefaultBackground       = '#f0f0f0'
   DefaultForeground       = black
   SelectedBackground      = '#7070c0'
   SelectedForeground      = white
else
   DefaultBackground       = white
   DefaultForeground       = black
   SelectedBackground      = black
   SelectedForeground      = white
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% the config object to read/write changeable options
%% first, some initial values... (read from a config file someday?)

ConfigEmacs     = {Emacs.getOPI} \= false % should we use Emacs?
ConfigUpdate    = UpdateTimes.1.1  % Automatic update interval
ConfigThreshold = t(calls:2 closures:2 samples:2 heap:256)
EmacsInterface  = {Emacs.getOPI}

Config =
{New
 class

    feat
       ConfAllowed: confAllowed(emacsInterface: true
				closeAction:    true)

    attr
       emacs          : ConfigEmacs
       update         : ConfigUpdate
       threshold      : ConfigThreshold
       emacsInterface : EmacsInterface
       closeAction    : unit

    meth init
       skip
    end

    meth confAllowed(F $)
       {CondSelect self.ConfAllowed F false}
    end

    meth toggle(What)
       What <- {Not @What}
    end

    meth set(What Value)
       What <- Value
    end

    meth get(What $)
       @What
    end

 end init}

proc {Ctoggle What}
   {Config toggle(What)}
end

proc {Cset What Value}
   {Config set(What Value)}
end

fun {Cget What}
   {Config get(What $)}
end

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
