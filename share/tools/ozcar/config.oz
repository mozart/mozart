%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Strings
%%
Version                = '0.9 (Dec 96)'
TitleName              = 'Oz Debugger Interface'
IconName               = 'Ozcar'

SourceWindowTitle      = 'Ozcar Source Window'
SourceWindowIcon       = 'Ozcar Source'

TreeTitle              = 'Thread Tree'
StackTitle             = 'Stack'
AltStackTitle          = 'Stack of Thread  #'
EnvTitle               = 'Environment'
AltEnvTitle            = 'Environment of Frame  #'

StatusInitText         = 'No current thread'
StatusEndText          = 'See you again...'

InvalidThreadID        = 'Invalid Thread ID in step message'
NoFileInfo             = 'step message without line number information, ' #
                         'continuing thread #'
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
ThreadTreeStretchX     = 11
ThreadTreeStretchY     = 13
ThreadTreeOffset       = 4

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
%%
TextCursor             = left_ptr

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Colors
%% TODO: distinguish between color & monochrome scheme

DefaultBackground      = '#f0f0f0'
SourceTextForeground   = '#000000'
SourceTextInvForeground= '#ffffff'
SourceTextBackground   = '#ffffff'

ScrollbarApplColor     = '#f07070'
ScrollbarStackColor    = '#7070f0'
ScrollbarColors        = colors(appl  : ScrollbarApplColor
				stack : ScrollbarStackColor)
				
RunningThreadColor     = '#00b000'  % some nice green
BlockedThreadColor     = '#e07070'  %  ~    ~   red
DeadThreadColor        = '#505050'  %  ~    ~   black
ZombieThreadColor      = '#f000f0'  % debugging purposes...
TrunkColor             = '#a00000'  % brown...
/* hello emacs: end */
ProcColor              = '#0000c0'
BuiltinColor           = '#c00000'


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% the config object to read/write changeable options
%% first, some initial values...


ConfigVerbose          = false


Config =
{New
 class
    
    attr
       verbose : ConfigVerbose         %% debug messages in Emulator buffer?
    
    meth init
       skip
    end
    
    meth toggleVerbose
       verbose <- {Not @verbose}
    end
    
    meth get(What $)
       @What
    end
    
 end init}

fun {Cget What}
   {Config get(What $)}
end

%%
%%%%%%%%%%%%%%%%%%%%%%%% End of config.oz %%%%%%%%%%%%%%%%%%%%%%
%%
