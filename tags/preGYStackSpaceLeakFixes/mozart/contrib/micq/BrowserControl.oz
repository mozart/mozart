/**
* Under Unix, the system browser is hard-coded to be 'netscape'.
* Netscape must be in your PATH for this to work. This has been
* tested with the following platforms: AIX, HP-UX and Solaris.
*
* Under Windows, this will bring up the default browser under windows,
* usually either Netscape or Microsoft IE. The default browser is
* determined by the OS. This has been tested under Windows 95/98/NT.
*				
*   {BrowserControl.displayURL "http://www.javaworld.com"}
*   {BrowserControl.displayURL "file://c:\\docs\\index.html"}
*   {BrowserContorl.displayURL "file:///user/joe/index.html"}
*   {BrowserContorl.displayURL "mailto:nilsf@sics.se"}
*				
* Note - you must include the url type -- either "http://" or "file://".
*/

functor
import
   OS
   System
   Property
export
   DisplayUrl
define
   IsWin32=({Property.get 'platform.os'}==win32)
   proc{DisplayUrl URL}
      try
	 if IsWin32 then
	    %% rundll32 url.dll,FileProtocolHandler http://www.mozart-oz.org
	    _={OS.exec "rundll32" ["url.dll,FileProtocolHandler" URL] true}
	 else
	    %% netscape -remote openURL(http://www.mozart-oz.org)
	    ExitCode={OS.system "netscape -raise -remote 'openURL("#URL#")"}
	 in
	    if ExitCode\=0 then
	       %% Command failed, start up the browser with: netscape http://www.mozart-oz.org
	       {System.showInfo "netscape "#URL}
	       _={OS.exec "netscape" [URL] true}
	    end
	 end
      catch EE then
	 {OS.wait _ _}
	 {System.showError "Error bringing up browser\n"#{Value.toVirtualString EE 100 100}}
      end
   end
end
