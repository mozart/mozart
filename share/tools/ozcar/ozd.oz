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

%% TODO:
%% -- synchronize environment of created compiler with Ozcar's stack display

functor
import
   Application(getCmdArgs exit)
   Compiler(engine)
   Debug(breakpoint) at 'x-oz://boot/Debug'
   Emacs(interface)
   Module(manager)
   OS(system)
   Open(socket)
   Ozcar(object)
   Pickle(load)
   Property(get put)
   System(showError)
prepare
   ArgSpec = record(emacs(rightmost char: &E type: bool default: false))
define
   try Args in
      Args = {Application.getCmdArgs ArgSpec}
      case Args.1 of AppName|AppArgs then AppFunc MM in
	 {Property.put 'ozd.argv' AppArgs}
	 {Ozcar.object on()}
	 if Args.emacs then Socket Port E I in
	    thread
	       Socket = {New Open.socket server(port: ?Port)}
	    end
	    E = {New Compiler.engine init()}
	    {OS.system
	     'emacs -L '#{Property.get 'oz.home'}#
	     '/share/elisp -l oz -f oz-attach '#Port#' \&' _}
	    I = {New Emacs.interface
		 init(E unit
		      proc {$ V}
			 {Socket write(vs: {Value.toVirtualString V 0 0})}
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
	 MM = {New Module.manager init()}
	 {Wait {MM apply(url: AppName AppFunc $)}}
      else
	 {Exception.raiseError ap(usage 'missing application argument')}
      end
   catch error(ap(usage VS) ...) then
      {System.showError 'Usage error: '#VS}
      {Application.exit 2}
   end
end
