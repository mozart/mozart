functor
import
   OS(getEnv system)
   Property(get)
export ShellCommand IsWindows Rmdir
define
   IsWindows = ({Property.get 'platform.os'}==win32)
   ShellCommand
   if IsWindows then
      SHELL = {OS.getEnv 'COMSPEC'}
   in
      fun {ShellCommand S}
	 {OS.system SHELL#' /c '#S}
      end
   else
      fun {ShellCommand S}
	 {OS.system S}
      end
   end
   %%
   proc {Rmdir F}
      {ShellCommand 'rmdir '#F}
   end
%    %%
%    % Determine the shell to use:
% %    returns `C:\WINNT\system32\cmd.exe' for NT/2000
% %    returns `C:\WINDOWS\COMMAND.COM' for 95/98
% {System.showInfo {OS.getEnv 'COMSPEC'}}
% 
% % Create a directory
% {Show {OS.system {OS.getEnv 'COMSPEC'}#" /c mkdir C:\\testdir"}}
% 
% % Remove a directory
% {Show {OS.system {OS.getEnv 'COMSPEC'}#" /c rmdir C:\\testdir"}}
% 
% % Remove a file
% {OS.unlink 'C:\\blabla'}
end
