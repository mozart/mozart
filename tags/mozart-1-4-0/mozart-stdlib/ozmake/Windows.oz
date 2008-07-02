functor
export
   comspec  : COMSPEC
   isWin    : IsWin
   isOldWin : IsOldWin
   ExecHeader
import
   OS(getEnv) Property(get)
   Utils at 'Utils.ozf'
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
   ExecHeader =
   if IsWin then
      {ByNeed
       fun {$}
	  {Utils.slurpFile {Property.get 'oz.home'}#'/bin/ozwrapper.bin'}
       end}
   else
      '#! /bin/sh\nexec ozengine $0 "$@"\n'
   end
end
