%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Version                = '0.3 (Nov 96)'
TitleName              = 'Oz Debugger Interface'
IconName               = 'Ozcar'
ToplevelGeometry       = '600x400+3+100'

SourceWindowTitle      = 'Ozcar Source Window'
SourceWindowIcon       = 'Ozcar Source'
SourceWindowGeometry   = '501x549+620+100'  %% I really hate hardcoding this
                                            %% but window managers seem
                                            %% to be f*cking stupid :-((
SourceWindowTextSize   = 80 # 50
ScrollbarDefaultColor  = '#c05050'

BitMapDir              = {System.get home} # '/lib/bitmaps/'
BitMap                 = '@' # BitMapDir # 'debugger.xbm'

SmallFont              = '6x10'
DefaultFont            = '7x13'
BoldFont               = '7x13bold'

DefaultBackground      = '#f0f0f0'
SourceTextBackground   = white

SmallBorderSize        = 0
BorderSize             = 2

ThreadTreeWidth        = 180
ThreadTreeFont         = fixed
ThreadTreeBoldFont     = '6x13bold'

TextWidth              = 30
TextCursor             = left_ptr
