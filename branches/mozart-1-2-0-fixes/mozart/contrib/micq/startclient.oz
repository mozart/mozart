%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
require
   DSettings(url:DefaultURL) at 'defaultsettings.ozf'
import
   Tk
   System
   Property(get put)
   OS(getEnv)
   Application(getCmdArgs exit)
   Client(start:StartClient) at 'client.ozf'
define
   DefaultHome=case {OS.getEnv 'HOME'} of false then "" elseof X then X#"/" end
   DefaultLogin=case {OS.getEnv 'USER'} of false then "" elseof X then X end
   Spec=record('url'(single char:&u type:string default:DefaultURL)
	       'home'(single char:&h type:string default:DefaultHome)
	       'login'(single char:&l type:string default:DefaultLogin)
	       'passwd'(single char:&p type:string default:"" ))
      
   proc{StartMIM Args}
      T={New Tk.toplevel tkInit(title:"Settings for Client...")}
      V1 V2 V3 
      B1 B2 BF={New Tk.frame tkInit(parent:T)}
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=start(login:{V1 tkReturnAtom($)}
		 ticketURL:{V2 tkReturnString($)}
		 passwd:{V3 tkReturnAtom($)}
		 newuser:GO==newuser) 
      in
	 {Wait A.login} {Wait A.ticketURL} {Wait A.passwd} 
	 {T tkClose}
	 {StartClient A}
	 raise quit end
      end
	 
      proc{NewEntry Title Value Secret V}
	 O N E L={New Tk.label tkInit(parent:T text:Title)}
      in
	 {Exchange Index O N} N=O+1
	 V={New Tk.variable tkInit(Value)}
	 if Secret then
	    E={New Tk.entry tkInit(parent:T width:50 show:'*' textvariable:V)}
	 else
	    E={New Tk.entry tkInit(parent:T width:50 textvariable:V)}
	 end
	 
	 {Tk.batch [grid(L row:N column:0 sticky:e)
		    grid(E row:N column:1 sticky:w)]}
	 {E tkBind(event:'<Return>' action:proc{$} GO=unit end)}
	 if N==2 then {Tk.send focus(E)} else skip end
      end
   in
      V2={NewEntry "URL:" Args.url false}
      V1={NewEntry "Login:" Args.login false}
      V3={NewEntry "Password:" Args.passwd true}
      B1={New Tk.button tkInit(parent:BF text:"New User" action:proc{$} GO=newuser end)}
      B2={New Tk.button tkInit(parent:BF text:"Login" action:proc{$} GO=unit end)}
      {Tk.batch [grid(BF row:20 column:0 columnspan:2 sticky:we)
		 grid(B1 row:0 column:0 sticky:we)
		 grid(B2 row:0 column:1 sticky:we)
		 grid(columnconfigure T 0 weight:1)
		 grid(columnconfigure BF 0 weight:1)
		 grid(columnconfigure BF 1 weight:1)
		 wm(resizable T 0 0)]}
      
      {Wait GO}
      {Start2}
   end 
in
   {Property.put 'errors.toplevel' proc {$} skip end}
   try Args = {Application.getCmdArgs Spec} in
      {StartMIM Args}
   catch X then
      case X of quit then
	 {Application.exit 0}
      elseof error(ap(usage M) ...) then
	 {System.printError
	  'Command line option error: '#M#'\n'#
	  'Usage: '#{Property.get 'application.url'}#' [options]\n'#
	  '   --login=<Name>       Alias: -l <Name>\n'#
	  '   --passwd=<Password>  Alias: -p <Password>\n'#
	  '   --url=<URL>          URL to the ICQ server. Alias: -u <Url>\n'}
	 {Application.exit 2}
      elseof E then
	 raise E end
      end
   end
end


