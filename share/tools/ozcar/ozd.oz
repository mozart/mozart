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
%% -- is it possible to attach to a running Emacs instead of starting
%%    a new one?
%% -- provide for a means to start ozd from the OPI
%% -- synchronize environment of created compiler with Ozcar's stack display?
%%

functor
import
   Application
   Compiler(engine)
   Debug(breakpoint) at 'x-oz://boot/Debug'
   Emacs(interface)
   Module(manager)
   OPIEnv(full)
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

   local
      Application =
      'export'(exit:
		  proc {$ Status}
		     %--** display this in a dialog box indicating Status
		     {System.printError
		      'Application exited with status '#Status#'.\n'}
		  end)
   in
      MyApplication = Application
   end

   try Args in
      Args = {Application.getCmdArgs ArgSpec}
      if Args.help then
	 {Usage "" 0}
      end
      case Args.1 of AppName|AppArgs then CloseAction MM F in
	 {Ozcar.object on()}
	 {Property.put 'errors.toplevel' proc {$} skip end}
	 {Property.put 'errors.subordinate' proc {$} fail end}
	 if Args.useemacs then Socket Port E EMACS I in
	    thread
	       Socket = {New Open.socket server(port: ?Port)}
	    end
	    E = {New Compiler.engine init()}
	    {E enqueue(mergeEnv(OPIEnv.full))}
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
	    {Property.put 'opi.compiler' I}
	    {Ozcar.object conf(emacsInterface: I)}
	    thread {I readQueries()} end
	    proc {CloseAction}
	       {I exit()}
	       {Application.exit 0}
	    end
	 else
	    proc {CloseAction}
	       {Application.exit 0}
	    end
	 end
	 {Ozcar.object conf(closeAction: CloseAction)}
	 {Property.put 'ozd.args' AppArgs}
	 F = {Pickle.load AppName}
	 MM = {New Module.manager init()}
	 {MM enter(name: 'Application' {Adjoin Application MyApplication})}
	 {Wait {MM apply(url: AppName
			 {Functor.new F.'import' F.'export'
			  fun {$ IMPORT}
			     thread
				{Debug.breakpoint}
				{F.apply IMPORT}
			     end
			  end} $)}}
      [] nil then
	 {Exception.raiseError ap(usage 'missing application argument')}
      end
   catch error(ap(usage VS) ...) then
      {Usage 'Usage error: '#VS#'\n' 2}
   end
end
