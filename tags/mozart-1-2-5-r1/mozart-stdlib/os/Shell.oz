functor
export
   ExecuteProgram
   ExecuteCommand
   ToUserVS ToProgramVS ToCommandVS
import
   Property(get)
   OS(system getEnv)
   Open(pipe)
   Misc(
      isWindows        : IsWindows
      isWindowsAncient : IsWindowsAncient)
   at 'x-oz://system/os/Misc.ozf'
define
   QUOTE = if Misc.isWindows then '"' else '\'' end
   fun {Quote CMD Q}
      {FoldL CMD
       fun {$ VS I} VS#' '#Q#I#Q end nil}
   end
   fun {ToUserVS CMD} {Quote CMD ''} end
   ToProgramVS
   if IsWindowsAncient then
      fun {ToProgramVS CMD}
	 'COMMAND.COM /C'#{Quote CMD QUOTE}
      end
   else
      fun {ToProgramVS CMD}
	 {Quote CMD QUOTE}
      end
   end
   ToCommandVS
   if IsWindowsAncient then
      fun {ToCommandVS CMD|Args}
	 'COMMAND.COM /C '#CMD#{Quote Args QUOTE}
      end
   elseif IsWindows then
      fun {ToCommandVS CMD|Args}
	 CMD|{Quote Args QUOTE}
      end
   else
      fun {ToCommandVS CMD}
	 {Quote CMD QUOTE}
      end
   end
   proc {Execute VS}
      if {OS.system VS}\=0 then
	 {Exception.raiseError shell(VS)}
      end
   end
   proc {ExecuteProgram CMD} {Execute {ToProgramVS CMD}} end
   proc {ExecuteCommand CMD} {Execute {ToCommandVS CMD}} end
end