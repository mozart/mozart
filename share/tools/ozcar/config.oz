%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Strings
%%
Version                = '0.3 (Nov 96)'
TitleName              = 'Oz Debugger Interface'
IconName               = 'Ozcar'

SourceWindowTitle      = 'Ozcar Source Window'
SourceWindowIcon       = 'Ozcar Source'

TreeTitle              = 'Thread Tree'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread  #'
EnvTitle               = 'Environment'
AltEnvTitle            = 'Environment of Frame  #'

StatusInitText         = 'No current Thread'

InvalidThreadID        = 'Invalid Thread ID in step message'
EarlyThreadDeath       = '...hm, but it has died already?!'
KnownThread            = 'Got known thread'
NewThread              = 'Got new thread'
ID                     = fun {$ I} ' (id ' # I # ')' end

OzcarMessagePrefix     = 'Ozcar: '
				     
/*                       Just for you, you stupid emacs!
end end end
*/

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Geometry
%%
ToplevelGeometry       = '600x400+3+100'
SourceWindowGeometry   = '501x549+620+100'  %% I really hate hardcoding this
                                            %% but window managers seem
                                            %% to be f*cking stupid :-((
SourceWindowTextSize   = 80 # 50

ThreadTreeWidth        = 180
ThreadTreeStretch      = 40
ThreadTreeOffset       = 14

TextWidth              = 30

SmallBorderSize        = 0
BorderSize             = 2

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Fonts
%%
SmallFont              = '6x10'
DefaultFont            = '7x13'
BoldFont               = '7x13bold'
ThreadTreeFont         = fixed
ThreadTreeBoldFont     = '6x13bold'

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Files
%%
BitMapDir              = {System.get home} # '/lib/bitmaps/'
BitMap                 = '@' # BitMapDir # 'debugger.xbm'

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Miscellaneous
TextCursor             = left_ptr

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Colors
%% TODO: distinguish between color & monochrome scheme

DefaultBackground      = '#f0f0f0'
SourceTextBackground   = '#ffffff'
ScrollbarDefaultColor  = '#c05050'

RunningThreadColor     = '#90e0a0'  % some nice green
BlockedThreadColor     = '#f09090'  %  ~    ~   red
DeadThreadColor        = '#404040'  %  ~    ~   black
ZombieThreadColor      = '#f000f0'  % debugging purposes...

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
%%
