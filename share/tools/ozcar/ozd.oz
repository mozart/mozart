%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998-2003
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
   OzcarClient(start)
   Profiler(object)
   Pickle(load)
   Property(get put)
   System(printError)
prepare
   ArgSpec = record(help(rightmost char: [&h &?] default: false)
		    mode(rightmost
			 type: atom(debugger remotedebugger profiler)
			 default: debugger)
		    debugger(char: &g alias: mode#debugger)
		    remotedebugger(char: &r alias: mode#remotedebugger)
		    profiler(char: &p alias: mode#profiler)
		    useemacs(rightmost char: &E type: bool default: false)
		    emacs(single type: string default: unit)
		    ticket(single char: &t type: string default: unit)
		    opi(rightmost default: false))

   %% Note: The opi option is not documented here on purpose.
   UsageString =
   '--help, -h, -?  Display this message.\n'#
   '--mode=debugger, --debugger, -g\n'#
   '                Start Ozcar (the default).\n'#
   '--mode=remotedebugger, --remotedebugger, -r\n'#
   '                Connect to a remote Ozcar using the given ticket.\n'#
   '--mode=profiler, --profiler, -p\n'#
   '                Start the Profiler.\n'#
   '--useemacs, -E  Start a subordinate Emacs process.\n'#
   '--nouseemacs    Do not start a subordinate Emacs process.\n'#
   '                This is the default.\n'#
   '--emacs=FILE    Specify the Emacs binary to run\n'#
   '                (Default: $OZEMACS or emacs).\n'#
   '--ticket=TICKET, -t TICKET\n'#
   '                The ticket to use for the remote debugger.\n'
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

   proc {Interactive Target Args} CloseAction in
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

	 %% Try to load some ozrc file
	 local
	    fun {FileExists FileName}
	       try F in
		  F = {New Open.file init(name: FileName flags: [read])}
		  {F close()}
		  true
	       catch _ then false
	       end
	    end
	 in
	    case {OS.getEnv 'HOME'} of false then skip
	    elseof HOME then
	       OZRC = {OS.getEnv 'OZRC'}
	    in
	       if OZRC \= false andthen {FileExists OZRC} then
		  {E enqueue(feedFile(OZRC))}
	       elseif {FileExists {Property.get 'oz.dotoz'}#'/ozrc'} then
		  {E enqueue(feedFile({Property.get 'oz.dotoz'}#'/ozrc'))}
	       elseif {FileExists HOME#'/.oz/ozrc'} then
		  {E enqueue(feedFile(HOME#'/.oz/ozrc'))}
	       elseif {FileExists HOME#'/.ozrc'} then   % note: deprecated
		  {E enqueue(feedFile(HOME#'/.ozrc'))}
	       end
	    end
	 end

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
   end

   try Args in
      Args = {Application.getCmdArgs ArgSpec}
      if Args.help then
	 {Usage "" 0}
      end
      case Args.1 of AppName|AppArgs then F MM in
	 F = {Pickle.load AppName}
	 case Args.mode of debugger then
	    {Interactive Ozcar.object Args}
	 [] profiler then
	    {Interactive Profiler.object Args}
	 [] remotedebugger then
	    case Args.ticket of unit then
	       {Exception.raiseError ap(usage 'missing ticket')}
	    elseof Ticket then
	       {OzcarClient.start Ticket}
	    end
	 end
	 {Property.put 'ozd.args' AppArgs}
	 MM = {New Module.manager init()}
	 {MM enter(name: 'Application' {Adjoin Application MyApplication})}
	 {MM apply(url: AppName
		   {Functor.new F.'import' F.'export'
		    fun {$ IMPORT}
		       thread
			  {Debug.breakpoint}
			  {F.apply IMPORT}
		       end
		    end})}
      [] nil then
	 {Exception.raiseError ap(usage 'missing application argument')}
      end
   catch error(ap(usage VS) ...) then
      {Usage 'Usage error: '#VS#'\n' 2}
   end
end
