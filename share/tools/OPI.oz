%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Contributors:
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
import
   Application(getCmdArgs)
   Module(manager)
   System(printError)
   Property(get put)
   OS(getEnv)
   Open(file)
   Compiler(engine)
   Emacs(interface)
export
   compiler: OPICompiler
   interface: CompilerUI
   'import': Import
require
   DefaultURL(functorNames: Modules)
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

		'Module'('Link':  [link]
			 'Apply': [apply])

		%% Tools
		'Browser'('Browse': [browse])

		'Explorer'('ExploreOne':  [one]
			   'ExploreAll':  [all]
			   'ExploreBest': [best])

		'Inspector'('Inspect': [inspect])

		'OPI'('Import': ['import'])]

   fun {Dots M Fs}
      case Fs of nil then M
      [] F|Fr then {Dots M.F Fr}
      end
   end
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

   %% Importing functors
   local
      fun {GuessModuleName Url}
	 %% Takes functor URL and computes module name from it:
	 %% the module name is the basename without filename extension
	 Base = {Reverse {String.token
			  {Reverse {VirtualString.toString Url}} &/ $ _}}
	 H|R = {String.token Base &. $ _}
      in
	 {String.toAtom {Char.toUpper H}|R}#Url
      end
   in
      proc {Import Us}
	 %% Takes list of urls
	 ModMan = {New Module.manager init}
	 %% Compute pairlist of module name and url
	 MNUs   = {Map Us GuessModuleName}
	 %% Compute pairlist of module name and module
	 Ms     = {Map MNUs fun {$ MN#U}
			       MN#{ModMan link(url:U $)}
			    end}
      in
	 %% Make available in compiler environment
	 {OPICompiler enqueue(mergeEnv({List.toRecord env Ms}))}
	 %%--** Print message, is wrong: LEIF, CHECK THAT
	 {System.printError ('% --- Opening functors:\n'#
			     {FoldL MNUs fun {$ V MN#U}
					    V#'%   '#MN#' at '#U#'\n'
					 end ''})}
      end
   end

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
	 if OZRC \= false andthen {FileExists OZRC} then
	    {OPICompiler enqueue(feedFile(OZRC))}
	 elseif {FileExists HOME#'/.oz/ozrc'} then
	    {OPICompiler enqueue(feedFile(HOME#'/.oz/ozrc'))}
	 elseif {FileExists HOME#'/.ozrc'} then   % note: deprecated
	    {OPICompiler enqueue(feedFile(HOME#'/.ozrc'))}
	 end
      end
   end

   thread {CompilerUI readQueries()} end
end
