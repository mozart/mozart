%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Christian Schulte <schulte@dfki.de>
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%   Christian Schulte, 1998
%%%   Denys Duchier, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
require
   DefaultURL(functorNames: Modules)
   
import
   Application(getCmdArgs)
   Module(manager)
   System(printError)
   Property(get put)
   OS(getEnv)
   Open(file)
   Compiler(engine)
   Emacs(interface)
prepare
   Spec = record(host(single type: string default: unit))

   ShortCuts = [%% Library
		'Pickle'('Load': [load]
			 'Save': [save])

		'Search'('SearchOne':  [base one]
			 'SearchAll':  [base all]
			 'SearchBest': [base best])

		'System'('Show':  [show]
			 'Print': [print])

		%% Tools
		'Browser'('Browse': [browse])

		'Explorer'('ExploreOne':  [one]
			   'ExploreAll':  [all]
			   'ExploreBest': [best])

		'Inspector'('Inspect': [inspect])]

   fun {Dots M Fs}
      case Fs of nil then M
      [] F|Fr then {Dots M.F Fr}
      end
   end
export
   compiler: OPICompiler
   interface: CompilerUI
define
   Args = {Application.getCmdArgs Spec}

   local
      OZVERSION = {Property.get 'oz.version'}
      DATE      = {Property.get 'oz.date'}
   in
      {System.printError
       'Mozart Engine '#OZVERSION#' of '#DATE#' playing Oz 3\n\n'}
   end

   {Property.put 'oz.standalone' false}

   OPICompiler = {New Compiler.engine init()}
   {OPICompiler enqueue(setSwitch(warnunused true))}

   local
      ModMan = {New Module.manager init}
   in
      %% Get system modules
      local
	 Env = {List.toRecord env
		{Map Modules
		 fun {$ ModName}
		    ModName#{ModMan link(name:ModName $)}
		 end}}
      in
	 {OPICompiler enqueue(mergeEnv(Env))}
      end

      %% Provide shortcuts
      {ForAll ShortCuts
       proc {$ SC}
	  Module = {ModMan link(name:{Label SC} $)}
	  Env    = {Record.map SC
		    fun lazy {$ Fs}
		       {Dots Module Fs}
		    end}
       in
	  {OPICompiler enqueue(mergeEnv(Env))}
       end}
   end

   CompilerUI = {New Emacs.interface init(OPICompiler Args.host)}
   {Property.put 'opi.compiler' CompilerUI}

   %% Make the error handler non-halting
   {Property.put 'errors.toplevel'    proc {$} skip end}
   {Property.put 'errors.subordinate' proc {$} fail end}

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
	 case OZRC \= false andthen {FileExists OZRC} then
	    {OPICompiler enqueue(feedFile(OZRC))}
	 elsecase {FileExists HOME#'/.oz/ozrc'} then
	    {OPICompiler enqueue(feedFile(HOME#'/.oz/ozrc'))}
	 elsecase {FileExists HOME#'/.ozrc'} then   % note: deprecated
	    {OPICompiler enqueue(feedFile(HOME#'/.ozrc'))}
	 else
	    skip
	 end
      end
   end

   thread {CompilerUI readQueries()} end
end
