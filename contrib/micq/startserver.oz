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
   DSettings(url:DefaultURL
	     ticketfile:DefaultTicket
	     systemdir:DefaultDir) at 'defaultsettings.ozf'
import
   Tk
   System(printError)
   Property(get)
   Application(getCmdArgs exit)
   Server(start:StartServer) at 'server.ozf'

define
   Spec=record('ticket'(single char:&t type:string default:DefaultTicket)
	       'dir'(single char:&d type:string default:DefaultDir)
	       'url'(single char:&u type:string default:DefaultURL)
	       'quickstart'(single char:&q type:bool default:false))
   proc{StartICQ Args}
      T={New Tk.toplevel tkInit(title:"Settings for Server...")}
      V1 V2 %V3
      Index={NewCell 0}
      GO
      proc{Start2}
	 A=start(ticketSave:{V1 tkReturnString($)}
		 dbdir:{V2 tkReturnString($)}
%		 ticketURL:{V3 tkReturnString($)
		)
      in
	 {Wait A.ticketSave} {Wait A.dbdir} %{Wait A.ticketURL} 
	 {T tkClose}
	 {StartServer A}
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
      V1={NewEntry "Ticket Save File:" Args.ticket}
      V2={NewEntry "System Directory:" Args.dir}
%      V3={NewEntry "URL:" Args.url}

      {Wait GO}
      {Start2}
   end 
in
   try
      Args = {Application.getCmdArgs Spec}
   in
      if Args.quickstart then
	 {Tk.send tk_bisque}
	 {StartServer start(dbdir: Args.dir ticketSave: Args.ticket
			    url: Args.url)}
	 raise quit end
      else
	 {Tk.send tk_bisque}
	 {StartICQ Args}
      end
   catch X then
      case X of quit then
	 {Application.exit 0}
      elseof error(ap(usage M) ...) then
	 {System.printError
	  'Command line option error: '#M#'\n'#
	  'Usage: '#{Property.get 'application.url'}#' [options]\n'#
	  '   --ticket=<File>\tAlias: -t <File>\n'#
	  '   --dir=<Dir>\t\tAlias: -d <Dir>\n'#
	  '   --url=<URL>\t\tURL to the Server. Alias: -u <Url>\n'#
	  '   --quickstart\t\tDon\'t show the start window. Alias: -q\n'}
	 {Application.exit 2}
      elseof E then
	 raise E end
      end
   end
end


