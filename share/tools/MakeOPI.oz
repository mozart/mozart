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
       \insert Compiler.env
       = IMPORT.'Compiler'

       class TextFile from Open.file Open.text
	  prop final
	  meth readQuery($) S in
	     Open.text, getS(?S)
	     case S of false then ""
	     elseof [4] then ""   % ^D
	     elseof [4 13] then ""   % ^D^M
	     else S#'\n'#TextFile, readQuery($)
	     end
	  end
       end

       local
	  MSG_ERROR = [17]
	  EMU_OUT_START = [5]
	  EMU_OUT_END = [6]
       in
	  class CompilerInterfaceEmacs from Compiler.genericInterface
	     prop final
	     meth init(CompilerObject)
		Compiler.genericInterface, init(CompilerObject Serve)
	     end
	     meth Serve(Ms)
		case Ms of M|Mr then
		   case M of info(VS) then
		      {System.printInfo EMU_OUT_END#VS#EMU_OUT_START}
		   [] info(VS _) then
		      {System.printInfo EMU_OUT_END#VS#EMU_OUT_START}
		   [] message(Record _) then
		      {Error.msg
		       proc {$ X}
			  {System.printInfo
			   EMU_OUT_END#{Error.formatLine X}#EMU_OUT_START}
		       end
		       Record}
		   [] displaySource(Title Ext VS) then Name File in
		      Name = {OS.tmpnam}#Ext
		      File = {New Open.file
			      init(name: Name
				   flags: [write create truncate])}
		      {File write(vs: VS)}
		      {File close()}
		      {Print {String.toAtom
			      {VirtualString.toString 'oz-show-temp '#Name}}}
		   [] toTop() then
		      {System.printInfo EMU_OUT_END#MSG_ERROR#EMU_OUT_START}
		   else skip
		   end
		   CompilerInterfaceEmacs, Serve(Mr)
		end
	     end
	  end
       end
    in
       proc {StartOPI _ _} OPICompiler CompilerReadEvalLoop in
	  OPICompiler = {New Compiler.compilerClass init()}
	  local
	     Env = {Record.foldL IMPORT Adjoin BaseAndStandard}
	  in
	     {OPICompiler enqueue(mergeEnv(Env))}
	  end

	  {{`Builtin` setOPICompiler 1}
	   {New CompilerInterfaceEmacs init(OPICompiler)}}

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

	  proc {CompilerReadEvalLoop} File VS0 VS in
	     File = {New TextFile init(name: stdin flags: [read])}
	     {File readQuery(?VS0)}
	     {File close()}
	     VS = case VS0 of ""#'\n'#VS1 then VS1 else VS0 end
	     {OPICompiler enqueue(feedVirtualString(VS))}
	     {CompilerReadEvalLoop}
	  end

	  {CompilerReadEvalLoop}
       end
    end

    plain}
end
