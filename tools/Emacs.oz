%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte
%%%
%%% Copyright:
%%%   Leif Kornstaedt and Benjamin Lorenz, 1997-1998
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL WARRANTIES.
%%%

\ifdef LILO

functor $

import
   SP.{System = 'System'
       Print  = 'Print'
       Error  = 'Error'}

   OP.{OS   = 'OS'
       Open = 'Open'}

   Compiler

export
   'Emacs' : Emacs

body

\else

fun instantiate {$ IMPORT}
   \insert 'SP.env'
   = IMPORT.'SP'
   \insert 'OP.env'
   = IMPORT.'OP'
   \insert 'Compiler.env'
   = IMPORT.'Compiler'

\endif

   TimeoutToConfigBar = 100
   TimeoutToUpdateBar = TimeoutToConfigBar

   fun {UnknownFile F}
      F == nofile orelse F == ''
   end

   local
      fun {V2VS X}
	 P = {System.get errors}
      in
	 {System.valueToVirtualString X P.depth P.width}
      end

      proc {Trace M}
	 case {Emacs.getOPI} of false then skip
	 elseof OPI then
	    case {OPI isTrace($)} then
	       {System.showInfo 'Emacs: ' # M}
	    else skip
	    end
	 end
      end

      Platform = {System.get platform}.1
      WindowsPlatform = 'win32'

      local
	 FieldSeparator = case Platform == WindowsPlatform then &; else &: end

	 fun {PathList RawPath} H T in   % RawPath must be of type string
	    {List.takeDropWhile RawPath fun {$ C} C \= FieldSeparator end H T}
	    case T == nil then [H]
	    else H|{PathList T.2}
	    end
	 end

	 OzPathEnv = {OS.getEnv 'OZPATH'}
      in
	 OzPath = case {OS.getEnv 'HOME'} of false then {PathList OzPathEnv}
		  elseof HomeEnv then HomeEnv#'/Oz/lib'|{PathList OzPathEnv}
		  end
      end

      fun {DoLookupFile SearchList F OrigF}
	 case SearchList of nil then
	    %% must have been the name of an unsaved file or buffer in Emacs:
	    OrigF
	 elseof Path|SearchListRest then Try = Path # '/' # F in
	    try
	       case {OS.stat Try}.type == reg then
		  {Trace F # ' is ' # Try}
		  Try
	       else
		  {Trace F # ' is not ' #
		   Try # ': ' # {V2VS {OS.stat Try}}}
		  {DoLookupFile SearchListRest F OrigF}
	       end
	    catch system(...) then
	       {Trace F # ' is not ' # Try # ': file not found'}
	       {DoLookupFile SearchListRest F OrigF}
	    end
	 end
      end
   in
      fun {LookupFile F}
	 S   = {Atom.toString F}
	 Abs = case S                   % absolute path?
	       of     &/|_   then true
	       elseof _|&:|_ then Platform == WindowsPlatform
	       else false end
      in
	 case Abs then
	    %% the file doesn't need to exist, since it may be the name of
	    %% an unsaved buffer or file in Emacs:
	    F
	 else                           % ...no!
	    %% strip "./" or "././"
	    Suffix = case S of &.|&/|T then
			case T of &.|&/|R then R
			else T end
		     else S end
	 in
	    {DoLookupFile OzPath Suffix F}
	 end
      end
   end

   local
      MSG_ERROR = [17]

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
      class CompilerInterfaceEmacs from Compiler.genericInterface
	 prop final
	 attr Socket BarSync: _ BarLock: {NewLock} Trace: false
	 meth init(CompilerObject)
	    lock Port NodeName in
	       Compiler.genericInterface, init(CompilerObject Serve)
	       thread
		  @Socket = {New TextSocket server(port: ?Port)}
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
	 end
	 meth getSocket($)   %--** should be replaced by readQuery
	    @Socket
	 end
	 meth Serve(Ms)
	    case Ms of M|Mr then
	       case M of info(VS) then
		  {@Socket write(vs: VS)}
	       [] info(VS _) then
		  {@Socket write(vs: VS)}
	       [] message(Record _) then
		  {Error.msg
		   proc {$ X}
		      {@Socket write(vs: {Error.formatLine X})}
		   end
		   Record}
	       [] displaySource(Title Ext VS) then Name File in
		  Name = {OS.tmpnam}#Ext
		  File = {New Open.file
			  init(name: Name
			       flags: [write create truncate])}
		  {File write(vs: VS)}
		  {File close()}
		  {@Socket write(vs: {VirtualString.toAtom
				      '\'oz-show-temp '#Name#'\''})}
	       [] toTop() then
		  case {System.get standalone} then skip
		  else
		     {@Socket write(vs: MSG_ERROR)}
		  end
	       else skip
	       end
	       CompilerInterfaceEmacs, Serve(Mr)
	    end
	 end

	 meth setTrace(B)
	    Trace <- B
	 end
	 meth isTrace($)
	    @Trace
	 end

	 meth bar(file:F line:L column:C state:S)
	    BarSync <- _ = unit
	    case {UnknownFile F} orelse L == unit then
	       CompilerInterfaceEmacs, removeBar()
	    else
	       CompilerInterfaceEmacs, MakeOzBar({LookupFile F} L C S)
	    end
	 end
	 meth delayedBar(file:F line:L column:C state:S<=unchanged) New in
	    BarSync <- New = unit
	    thread
	       {WaitOr New {Alarm TimeoutToUpdateBar}}
	       case {IsDet New} then skip else
		  CompilerInterfaceEmacs, bar(file:F line:L column:C state:S)
	       end
	    end
	 end
	 meth configureBar(State) New in
	    BarSync <- New = unit
	    thread
	       {WaitOr New {Alarm TimeoutToConfigBar}}
	       case {IsDet New} then skip else
		  CompilerInterfaceEmacs, MakeOzBar(unchanged 0 0 State)
	       end
	    end
	 end
	 meth removeBar()
	    BarSync <- _ = unit
	    CompilerInterfaceEmacs, MakeOzBar(nofile 0 0 hide)
	 end
	 meth MakeOzBar(File Line Column State)
	    lock @BarLock then
	       C = case Column == unit then 0 else Column end
	       S = 'oz-bar ' # File # ' ' # Line # ' ' # C # ' ' # State
	    in
	       {@Socket write(vs: '\'' # S # '\'')}
	       {Delay 1}   % this is needed for Emacs
	    end
	 end
      end
   end

   GetOPI = {`Builtin` getOPICompiler 1}

\ifdef LILO
in
\endif
   
   Emacs = emacs(getOPI: GetOPI
		 condSend: condSend(interface:
				       proc {$ M}
					  case {GetOPI} of false then skip
					  elseof OPI then
					     {OPI M}
					  end
				       end
				    compiler:
				       proc {$ M}
					  case {GetOPI} of false then skip
					  elseof OPI then
					     {{OPI getCompiler($)} M}
					  end
				       end)
		 interface: CompilerInterfaceEmacs)

\ifndef LILO
in
   \insert 'Emacs.env'
\endif
   
end
