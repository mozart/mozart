functor
export
   isWindows       : IsWindows
   isWindowsAncient: IsWindowsAncient
   platform        : PLATFORM
import
   OS(getEnv) Property(get)
prepare
   VSToString = VirtualString.toString
   ToLower    = Char.toLower
define
   PLATFORM  = {Property.get 'platform'}
   IsWindows = (PLATFORM.os=='win32')
   IsWindowsAncient =
   IsWindows andthen
   case {Reverse {Map {VSToString {OS.getEnv 'COMSPEC'}} ToLower}}
   of &e|&x|&e|&.|&d|&m|&c|_ then false else true end
end
