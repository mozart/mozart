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
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% TODO:
%% -- is it possible to attach to a running Emacs instead of starting
%%    a new one?
%% -- synchronize environment of created compiler with Ozcar's stack display?
%%

functor
import
   Win32 at 'x-oz://boot/Win32'
   Application
   Compiler(engine)
   Debug(breakpoint) at 'x-oz://boot/Debug'
   Emacs(interface)
   Module(manager)
   OPIEnv(full)
   OS(getEnv)
   Open(file pipe socket)
   Ozcar(object)
   Profiler(object)
   Pickle(load)
   Property(get put)
   System(printError)
prepare
   ArgSpec = record(help(rightmost char: [&h &?] default: false)
		    mode(rightmost
			 type: atom(debugger profiler)
			 default: debugger)
		    debugger(char: &g alias: mode#debugger)
		    profiler(char: &p alias: mode#profiler)
		    useemacs(rightmost char: &E type: bool default: false)
		    emacs(single type: string default: unit)
		    opi(rightmost default: false))

   %% Note: The opi option is not documented here on purpose.
   UsageString =
   '--help, -h, -?  Display this message.\n'#
   '--mode=debugger, --debugger, -g\n'#
   '                Start Ozcar (the default).\n'#
   '--mode=profiler, --profiler, -p\n'#
   '                Start the Profiler.\n'#
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

   fun {GetRegistryEmacs}
      case {Property.get 'platform.os'} of win32 then
	 case {Win32.getRegistryKey 'HKEY_LOCAL_MACHINE'
	       'SOFTWARE\\GNU\\Emacs' 'emacs_dir'}
	 of unit then unit
	 [] false then unit
	 elseof S then S#'/bin/emacs.exe'
	 end
      else unit
      end
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
      case Args.1 of AppName|AppArgs then Target CloseAction MM F in
	 Target = case Args.mode of debugger then Ozcar.object
		  [] profiler then Profiler.object
		  end
	 {Target on()}
	 {Property.put 'errors.toplevel' proc {$} skip end}
	 {Property.put 'errors.subordinate' proc {$} fail end}
	 if Args.useemacs then File E EMACS I in
	    E = {New Compiler.engine init()}
	    {E enqueue(mergeEnv(OPIEnv.full))}
	    if Args.opi then
	       OZVERSION = {Property.get 'oz.version'}
	       DATE      = {Property.get 'oz.date'}
	    in
	       {System.printError ('Mozart Engine '#OZVERSION#' ('#DATE#
				   ') playing Oz 3\n\n')}
	       File = {New Open.file init(name: stdout flags: [write])}
	    else Port in
	       thread
		  File = {New Open.socket server(port: ?Port)}
	       end
	       EMACS = case Args.emacs of unit then
			  case {OS.getEnv 'OZEMACS'} of false then
			     case {GetRegistryEmacs} of unit then 'emacs'
			     elseof X then X
			     end
			  elseof X then X
			  end
		       elseof X then X
		       end
	       _ = {New Open.pipe
		    init(cmd: EMACS
			 args: ['-L' {Property.get 'oz.home'}#'/share/elisp'
				'-l' 'oz' '-f' 'oz-attach' Port])}
	    end
	    I = {New Emacs.interface
		 init(E unit
		      proc {$ V}
			 {File write(vs: V)}
		      end)}
	    {Property.put 'opi.compiler' I}
	    {Target conf(emacsInterface: I)}
	    thread {I readQueries()} end
	    proc {CloseAction}
	       if Args.opi then skip
	       else {I exit()}
	       end
	       {Application.exit 0}
	    end
	 else
	    proc {CloseAction}
	       {Application.exit 0}
	    end
	 end
	 {Target conf(closeAction: CloseAction)}
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
