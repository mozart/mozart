%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@dfki.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
%%%   Christian Schulte, 1998
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

\ifdef LILO

%% THATS STILL ALL HACKED

local
   MozartUrl = 'http://www.ps.uni-sb.de/ozhome/'

   LibNames = ['Application'
	       'Search' 'FD' 'FS'
	       'System' 'Error' 'Debug' 'Finalize' 'Foreign'
	       'Connection' 'Remote'
	       'OS' 'Open' 'Component'
	       'Tk' 'TkTools'
	       'Compiler']

   ToolNames = ['Panel' 'Browser' 'Explorer' 'CompilerPanel'
		'Emacs' 'Ozcar' 'Profiler' 'Gump' 'GumpScanner'
		'GumpParser']

   local
      fun {IsPrintName A}
	 S={Atom.toString A}
      in
	 case S of nil then false
	 [] C|_ then {Char.isUpper C} orelse C==&`
	 end
      end
   in
      fun {GetPrintNames E}
	 {Filter {Arity E} IsPrintName}
      end
   end
   
   PrintNames =
   {FoldL LibNames
    fun {$ PNs A}
       Ns={GetPrintNames {Load MozartUrl#'lib/'#A#'.ozf'}.'export'}
    in
       case Ns==nil then PNs else A#Ns|PNs end
    end 
    {FoldL ToolNames
     fun {$ PNs A}
	Ns={GetPrintNames {Load MozartUrl#'tools/'#A#'.ozf'}.'export'}
     in
	case Ns==nil then PNs else A#Ns|PNs end
     end nil}}


   functor OPI

   import
      LILO.{loadByFeatures}
      
      System.{property
	      show
	      printError}
      
      OS.{getEnv}

      Open.{file}

      Compiler.{engine}

      Emacs.{interface}

   body

      local
	 OZVERSION = {System.property.get 'oz.version'}
	 DATE = {System.property.get 'oz.date'}
      in
	 {System.printError
	  'Mozart Engine '#OZVERSION#' of '#DATE#' playing Oz 3\n\n'}
      end

      {System.property.put 'oz.standalone' false}

      OPICompiler = {New Compiler.engine init()}

      local
	 Env = {List.toRecord env
		{Map LibNames
		 fun {$ A}
		    {System.show load(A)}
		    A#{LILO.loadByFeatures nil A}
		 end}}
      in
	 {OPICompiler enqueue(mergeEnv(Env))}
      end

      {ForAll PrintNames
       proc {$ Key#Feats}
	  Env = {LILO.loadByFeatures Feats Key}
       in
	  {OPICompiler enqueue(mergeEnv(Env))}
       end}

      {OPICompiler enqueue(mergeEnv(env('LILO':
					   {LILO.loadByFeatures nil 'LILO'})))}
      
      CompilerUI = {New Emacs.interface init(OPICompiler)}
      Sock = {CompilerUI getSocket($)}
      {{`Builtin` setOPICompiler 1} CompilerUI}

       % Try to load some ozrc file:
      local
	 HOME = {OS.getEnv 'HOME'}
      in
	 case HOME == false then skip
	 else
	    fun {FileExists FN}
	       try
		  F = {New Open.file init(name:FN)}
	       in
		  {F close} true
	       catch _ then false
	       end
	    end
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

in
   {Application.syslet
    'opi'
    OPI
    plain}
end

\else

{Application.syslet
 'opi'
 full('SP':            eager
      'OP':            eager
      'AP':            lazy
      'CP':            eager
      'DP':            lazy
      'WP':            lazy
      'Panel':         lazy
      'Browser':       lazy
      'Explorer':      lazy
      'Compiler':      eager
      'CompilerPanel': lazy
      'Emacs':         lazy
      'Ozcar':         lazy
      'Profiler':      lazy
      'Gump':          lazy
      'GumpScanner':   lazy
      'GumpParser':    lazy
      'Misc':          lazy)

 proc instantiate {$ IMPORT ?StartOPI}
    \insert SP.env
    = IMPORT.'SP'
    \insert OP.env
    = IMPORT.'OP'
    \insert Emacs.env
    = IMPORT.'Emacs'
    \insert Compiler.env
    = IMPORT.'Compiler'
 in
    proc {StartOPI _ _} OPICompiler CompilerUI Sock CompilerReadEvalLoop in
       local
	  OZVERSION = {System.property.get 'oz.version'}
	  DATE = {System.property.get 'oz.date'}
       in
	  {System.printError
	   'Mozart Engine '#OZVERSION#' of '#DATE#' playing Oz 3\n\n'}
       end
       {System.property.put 'oz.standalone' false}

       OPICompiler = {New Compiler.engine init()}
       local
	  Env = {Record.foldL IMPORT Adjoin env()}
       in
	  {OPICompiler enqueue(mergeEnv(Env))}
       end
       CompilerUI = {New Emacs.interface init(OPICompiler)}
       {CompilerUI getSocket(?Sock)}
       {{`Builtin` setOPICompiler 1} CompilerUI}

       % Try to load some ozrc file:
       local
	  HOME = {OS.getEnv 'HOME'}
       in
	  case HOME == false then skip
	  else
	     fun {FileExists FN}
		try
		   F = {New Open.file init(name:FN)}
		in
		   {F close} true
		catch _ then false
		end
	     end
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

 plain}


\endif

