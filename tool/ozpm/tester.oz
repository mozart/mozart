functor
import
   LS(fileTree)
   Application(getArgs exit)
   System(showInfo)
define
   Args = {Application.getArgs record}
   DirN = {NewCell 0}
   FileN = {NewCell 0}
   proc {IncrFileN}
      O N
   in
      {Exchange FileN O N}
      N=O+1
   end
   proc {IncrDirN}
      O N
   in
      {Exchange DirN O N}
      N=O+1
   end
   fun {Size R N}
      if R.type==dir then
	 {IncrDirN}
	 {SizeN R.entries N+R.size}
      else {IncrFileN} N+R.size end
   end
   fun {SizeN L N}
      case L of nil then N
      [] H|T then {SizeN T {Size H N}} end
   end
   Total = {SizeN {Map Args.1 LS.fileTree} 0}
   {System.showInfo 'dirs='#{Access DirN}#' files='#{Access FileN}#' total='#Total}
   {Application.exit 0}
end
