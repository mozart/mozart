%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
Version      = "0.2 (Oct 96)"
TitleName    = "Oz Debugger Interface (ozcar@" # {Unix.getEnv "HOST"} # ")"
IconName     = "Ozcar"

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
