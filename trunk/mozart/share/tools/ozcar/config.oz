%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Version      = "0.1 (Oct 96)"
TitleName    = "Oz Debugger Interface (ozcar@" # {Unix.getEnv "HOST"} # ")"
IconName     = "Ozcar"

StatusInit   =  TitleName # ' started.'
StatusFont   = '-*-helvetica-*-r-*-*-12-*-*-*-*-*-*-*'

BitMapDir    = {System.get home} # '/lib/bitmaps/'
BitMap       = '@' # BitMapDir # 'debugger.xbm'

DefaultFont  = '7x13bold'
TextSize     = 80 # 24

SmallBorderSize = 0
BorderSize      = 2
