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

declare
MM = {New Module.manager init()}

DSettings = {MM link(url:'/home/nilsf/source/mozart/contrib/micq/defaultsettings.ozf' $)}
Client = {MM link(url:'/home/simon/ozdevel/mozart/contrib/micq/client.ozf' $)}

DefaultHome=case {OS.getEnv 'HOME'} of false then "" elseof X then X#"/" end
DefaultLogin=case {OS.getEnv 'USER'} of false then "" elseof X then X end
DefaultSave=if {OS.uName}.sysname == "WIN32" then DefaultHome#"micq" else
	       DefaultHome#".micq" end

Spec=record('file'(single char:&f type:string default:DefaultSave)
	    'url'(single char:&u type:string default:DSettings.url)
	    'home'(single char:&h type:string default:DefaultHome)
	    'login'(single char:&l type:string default:DefaultLogin)
	    'passwd'(single char:&p type:string default:"" ))

proc{StartICQ Args}
   T={New Tk.toplevel tkInit(title:"Settings for Client...")}
   V1 V2 V3 V4
   Index={NewCell 0}
   GO
   proc{Start2}
      A=start(file:{V4 tkReturnAtom($)}
	      login:{V1 tkReturnAtom($)}
	      ticketURL:{V2 tkReturnString($)}
	      passwd:{V3 tkReturnAtom($)}) 
   in
      {Wait A.file} {Wait A.login} {Wait A.ticketURL} {Wait A.passwd} 
      {T tkClose}
      {Client.start {Adjoin q(newuser:false) A}}
      {Save A.file}
      raise quit end
   end

   proc{NewEntry Title Value V}
      O N E L={New Tk.label tkInit(parent:T text:Title)}
   in
      {Exchange Index O N} N=O+1
      V={New Tk.variable tkInit(Value)}
      E={New Tk.entry tkInit(parent:T width:50 textvariable:V)}
      {Tk.batch [grid(L row:N column:0 sticky:e)
		 grid(E row:N column:1 sticky:w)]}
      {E tkBind(event:'<Return>' action:proc{$} GO=unit end)}
      if N==1 then {Tk.send focus(E)} else skip end
   end
in
   V2={NewEntry "URL:" Args.url}
   V1={NewEntry "Login:" Args.login}
   V3={NewEntry "Password:" Args.passwd}
   V4={NewEntry "Init file:" Args.file}
   {Wait GO}
   {Start2}
end 

{Property.put 'errors.toplevel' proc {$} skip end}
try Args = {Application.getCmdArgs Spec} in
   {StartICQ Args}
catch X then
   case X of quit then
      {System.show closingClient}
   elseof error(ap(usage M) ...) then
      {System.printError
       'Command line option error: '#M#'\n'#
       'Usage: '#{Property.get 'application.url'}#' [options]\n'#
       '   --login=<Name>       Alias: -l <Name>\n'#
       '   --file=<Name>       Alias: -f <Initfile>\n'#
       '   --passwd=<Password>  Alias: -p <Password>\n'#
       '   --url=<URL>          URL to the ICQ server. Alias: -u <Url>\n'}
      {Application.exit 2}
   elseof E then
      raise E end
   end
end





