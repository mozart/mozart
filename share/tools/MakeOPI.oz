%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997
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
   BaseAndStandard = {Adjoin
		      \insert 'Base.env'
		      \insert 'Standard.env'
		     }
in
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

       class TextSocket from Open.socket Open.text
	  prop final
	  meth readQuery($) S in
	     Open.text, getS(?S)
	     case S of false then ""
	     elseof [4] then ""   % ^D
	     elseof [4 13] then ""   % ^D^M
	     else S#'\n'#TextSocket, readQuery($)
	     end
	  end
       end
    in
       proc {StartOPI _ _} Sock OPICompiler CompilerReadEvalLoop in
	  local Port NodeName in
	     thread
		Sock = {New TextSocket server(port: ?Port)}
	     end
\define NODENAME_USE_ADDR
\ifdef NODENAME_USE_ADDR
	     NodeName = {OS.getHostByName {OS.uName}.nodename}.addrList.1
\else
\ifdef NODENAME_USE_UNAME
	     NodeName = {OS.uName}.nodename
\else
\ifdef NODENAME_USE_HOSTENV
	     NodeName = {OS.getEnv 'HOST'}
\else
	     NodeName = 'localhost'
\endif
\endif
\endif
	     {Print {VirtualString.toAtom 'oz-socket "'#NodeName#'" '#Port}}
	  end

	  OPICompiler = {New Compiler.compilerClass init()}
	  local
	     Env = {Record.foldL IMPORT Adjoin BaseAndStandard}
	  in
	     {OPICompiler enqueue(mergeEnv(Env))}
	  end

	  {New Emacs.interface init(OPICompiler Sock) _}

	  local
	     OZVERSION = {System.property.get 'oz.version'}
	     DATE = {System.property.get 'oz.date'}
	  in
	     {System.printError
	      'Mozart Engine '#OZVERSION#' of '#DATE#' playing Oz 3\n\n'}
	  end
	  {System.printError
	   '---------------------------------------------\n'#
	   'MOTD\n\n'#
	   '19 Mar 1998, scheidhr@dfki.de\n'#
	   'SmartSave has been renamed to Save\n'#
	   'Save now only takes to parameters: {Save Value Filename}\n'#
	   'It now raises an exception if any resources are found.\n'#
	   '\n'#
	   '---------------------------------------------\n\n'}
	  {System.property.put 'oz.standalone' false}

	  % Try to load some ozrc file:
	  local
	     FileExists = {`Builtin` ozparser_fileExists 2}
	     OZRC = {OS.getEnv 'OZRC'}
	  in
	     case OZRC \= false then
		{OPICompiler enqueue(feedFile(OZRC))}
	     elsecase {FileExists '~/.oz/ozrc'} then
		{OPICompiler enqueue(feedFile('~/.oz/ozrc'))}
	     elsecase {FileExists '~/.ozrc'} then   % note: deprecated
		{OPICompiler enqueue(feedFile('~/.ozrc'))}
	     else
		skip
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
end
