%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@dfki.de>
%%%   Denys Duchier, <duchier@ps.uni-sb.de>
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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   local
      UrlDefaults = \insert ../url-defaults
   in
      FunExt    = UrlDefaults.'functor'
      MozartUrl = UrlDefaults.'home'
   end

   FuncDefaults = \insert ../functor-defaults

   local
      fun {IsPrintName A} S in
	 S = {Atom.toString A}
	 case S of nil then false
	 [] C|_ then {Char.isUpper C} orelse C == &`
	 end
      end
   in
      fun {GetPrintNames E}
	 {Filter {Arity E} IsPrintName}
      end
   end

   PrintNames =
   {FoldL {Append FuncDefaults.lib FuncDefaults.tools}
    fun {$ PNs A}
       Ns = {GetPrintNames
	     {Pickle.load MozartUrl#A#FunExt}.'export'}
    in
       case Ns == nil then PNs
       else A#Ns|PNs
       end
    end nil}

   proc {LazyAdapt M1 Fs ?M2}
      Request
      proc {FwdRequest _}
	 Request = unit
      end
   in
      M2 = {MakeRecord 'export' Fs}
      {Record.forAll M2
       fun {$}
	  {ByNeed FwdRequest}
       end}
      thread
	 {Wait Request}
	 {ForAll Fs proc {$ F} M1.F = M2.F end}
      end
   end
in
   functor prop once
   import
      Module.load
      System.printError
      Property.{get put}
      OS.getEnv
      Open.file
      Compiler.engine
      Emacs.interface
   body
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
	 Env = {List.toRecord env
		{Map FuncDefaults.lib
		 fun {$ A}
		    A#{Module.load A unit}
		 end}}
      in
	 {OPICompiler enqueue(mergeEnv(Env))}
      end

      {ForAll PrintNames
       proc {$ Key#Feats}
	  Env={LazyAdapt {Module.load Key unit} Feats}
       in
	  {OPICompiler enqueue(mergeEnv(Env))}
       end}

      {OPICompiler
       enqueue(mergeEnv({List.toRecord env
			 {Map FuncDefaults.volatile
			  fun {$ A}
			     A#{ByNeed fun {$} {Module.load A unit} end}
			  end}}))}

      %% QUICK HACK, I WILL TAKE CARE OF IT SOON: CS

      local
	 System = {Module.load 'System' unit}
      in
	 {OPICompiler
	  enqueue(mergeEnv(env('Show':   System.show
			       'Print':  System.print)))}
      end

      CompilerUI = {New Emacs.interface init(OPICompiler)}
      Sock = {CompilerUI getSocket($)}
      {Property.put 'opi.compiler' CompilerUI}

      % Try to load some ozrc file:
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

      proc {CompilerReadEvalLoop} VS0 VS in
	 {Sock readQuery(?VS0)}
	 VS = case VS0 of ""#'\n'#VS1 then VS1 else VS0 end
	 {OPICompiler enqueue(feedVirtualString(VS))}
	 {CompilerReadEvalLoop}
      end

      {CompilerReadEvalLoop}
   end
end
