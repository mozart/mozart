functor
export
   comspec  : COMSPEC
   isWin    : IsWin
   isOldWin : IsOldWin
import
   OS(getEnv) Property(get)
define
   IsWin = ({Property.get 'platform.os'}=='win32')
   COMSPEC IsOldWin
   if IsWin then
      COMSPEC={OS.getEnv 'COMSPEC'}
      case {Reverse {Map {VirtualString.toString COMSPEC} Char.toLower}}
      of &e|&x|&e|&.|&d|&m|&c|_ then IsOldWin=false
      else IsOldWin=true end
   else
      COMSPEC=unit
      IsOldWin=false
   end
end
