%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%   Benjamin Lorenz, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% TODO:
%% -- synchronize environment of created compiler with Ozcar's stack display
%% -- closing Ozcar should terminate the application
%% -- map Application.exit of the debugged application to the following
%%    actions:
%%     * remove emacs bar
%%     * ask `Exit Emacs?' in Emacs
%% -- is it possible to attach to a running Emacs instead of starting
%%    a new one?  Alternatively, provide for a means to start ozd from
%%    the OPI
%% -- attach Emacs queries does not work with the Emacs sub-process
%%    (the created Emacs interface is not entered as opi.compiler property)
%% -- the environment of the created compiler should be richer (cf. OPI),
%%    especially, it is not yet possible to set breakpoints since Ozcar is
%%    not in there
%%

functor
import
   Application(getCmdArgs exit)
   Compiler(engine)
   Debug(breakpoint) at 'x-oz://boot/Debug'
   Emacs(interface)
   Module(manager)
   OS(getEnv system)
   Open(socket)
   Ozcar(object)
   Pickle(load)
   Property(get put)
   System(printError)
prepare
   ArgSpec = record(help(rightmost char: [&h &?] default: false)
		    useemacs(rightmost char: &E type: bool default: false)
		    emacs(single type: string default: unit))

   UsageString =
   '--help, -h, -?  Display this message.\n'#
   '--useemacs, -E  Start a subordinate Emacs process.\n'#
   '--nouseemacs    Do not start a subordinate Emacs process.\n'#
   '                This is the default.\n'#
   '--emacs=FILE    Specify the Emacs binary to run\n'#
   '                (Default: $OZEMACS or emacs).\n'
define
   proc {Usage VS Status}
      {System.printError
       VS#'Usage: '#{Property.get 'application.url'}#
       ' <options> <appfunctor> -- <appargs>\n'#UsageString}
      {Application.exit Status}
   end

   try Args in
      Args = {Application.getCmdArgs ArgSpec}
      if Args.help then
	 {Usage "" 0}
      end
      case Args.1 of AppName|AppArgs then AppFunc MM in
	 {Property.put 'ozd.args' AppArgs}
	 {Ozcar.object on()}
	 if Args.useemacs then Socket Port E EMACS I in
	    thread
	       Socket = {New Open.socket server(port: ?Port)}
	    end
	    E = {New Compiler.engine init()}
	    EMACS = case Args.emacs of unit then
		       case {OS.getEnv 'OZEMACS'} of false then 'emacs'
		       elseof X then X
		       end
		    elseof X then X
		    end
	    {OS.system
	     EMACS#' -L '#{Property.get 'oz.home'}#
	     '/share/elisp -l oz -f oz-attach '#Port#' \&' _}
	    I = {New Emacs.interface
		 init(E unit
		      proc {$ V}
			 {Socket write(vs: V)}
		      end)}
	    {Ozcar.object conf(emacsInterface: I)}
	    thread {I readQueries()} end
	 end
	 local
	    F = {Pickle.load AppName}
	 in
	    AppFunc = {Functor.new F.'import' F.'export'
		       fun {$ IMPORT}
			  thread
			     {Debug.breakpoint}
			     {F.apply IMPORT}
			  end
		       end}
	 end
	 {Property.put 'errors.toplevel' proc {$} skip end}
	 {Property.put 'errors.subordinate' proc {$} fail end}
	 MM = {New Module.manager init()}
	 {Wait {MM apply(url: AppName AppFunc $)}}
      [] nil then
	 {Exception.raiseError ap(usage 'missing application argument')}
      end
   catch error(ap(usage VS) ...) then
      {Usage 'Usage error: '#VS#'\n' 2}
   end
end
