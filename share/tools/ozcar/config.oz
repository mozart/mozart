%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
Version      = '0.3 (Nov 96)'
TitleName    = 'Oz Debugger Interface'
IconName     = 'Ozcar'

StatusInit   =  TitleName # ' started.'
StatusFont   = '-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*'

BitMapDir    = {System.get home} # '/lib/bitmaps/'
BitMap       = '@' # BitMapDir # 'debugger.xbm'

SmallFont    = '6x10'
DefaultFont  = '7x13'
BoldFont     = '7x13bold'
TextSize     = 80 # 24

SmallBorderSize = 0
BorderSize      = 2

ThreadTreeHeight   = 300
ThreadTreeWidth    = 250
ThreadTreeFont     = fixed
ThreadTreeBoldFont = '6x13bold'

StackTextWidth  = 30
EnvTextWidth    = 20
StackTextHeight = 10
EnvTextHeight   = StackTextHeight

TextCursor = left_ptr
