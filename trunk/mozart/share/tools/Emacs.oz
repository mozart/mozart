%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Christian Schulte <schulte@dfki.de>
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
%%%   http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%   http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL WARRANTIES.
%%%

functor
import
   Property.{get condGet}
   System.{showInfo valueToVirtualString print}
   Error.{formatLine msg}
   OS.{getEnv stat uName getHostByName tmpnam}
   Open.{socket text file}
   Compiler.genericInterface

export
   getOPI:    GetOPI
   condSend:  CondSend
   interface: CompilerInterfaceEmacs

body
   TimeoutToConfigBar = 200
   TimeoutToUpdateBar = TimeoutToConfigBar

   fun {UnknownFile F}
      F == ''
   end

   local
      fun {V2VS X}
	 P = {Property.get errors}
      in
	 {System.valueToVirtualString X P.depth P.width}
      end

      proc {Trace M}
	 case {GetOPI} of false then skip
	 elseof OPI then
	    case {OPI isTrace($)} then
	       {System.showInfo 'Emacs: ' # M}
	    else skip
	    end
	 end
      end

      Platform = {Property.get platform}.1
      WindowsPlatform = 'win32'

      local
	 FieldSeparator = case Platform == WindowsPlatform then &; else &: end

	 fun {SystemPathList RawPath} H T in % RawPath must be of type string
	    {List.takeDropWhile RawPath fun {$ C} C \= FieldSeparator end H T}
	    case T == nil then [H]
	    else H|{SystemPathList T.2}
	    end
	 end

	 fun {HomePathList Home PrefixList}
	    %% some heuristics where to find the source files
	    {Append
	     {Map PrefixList
	      fun {$ P}
		 Home # P # '/mozart/share/lib'
	      end}
	     {Map PrefixList
	      fun {$ P}
		 Home # P # '/mozart/share/tools'
	      end}}
	 end

	 OzPathEnv = {OS.getEnv 'OZPATH'}
      in
	 OzPath = case {OS.getEnv 'HOME'} of false then
		     {SystemPathList OzPathEnv}
		  elseof HomeEnv then
		     {Append
		      {HomePathList HomeEnv
		       ['' '/Src' '/src' '/Devel' '/devel']}
		      {SystemPathList OzPathEnv}}
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
	 attr
	    Socket BarSync: _ BarLock: {NewLock} Trace: false
	    lastFile: unit lastLine: unit lastColumn: unit
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
	       {System.print {VirtualString.toAtom 'oz-socket "'#NodeName#'" '#Port}}
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
		  case {Property.get 'oz.standalone'} then skip
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
	       case {IsDet New} orelse @lastFile == unit then skip else
		  CompilerInterfaceEmacs,
		  MakeOzBar(@lastFile @lastLine @lastColumn State)
	       end
	    end
	 end
	 meth removeBar()
	    BarSync <- _ = unit
	    CompilerInterfaceEmacs, MakeOzBar('' 0 0 hide)
	 end
	 meth MakeOzBar(File Line Column State)
	    lock @BarLock then
	       S = 'oz-bar ' # File # ' ' # Line # ' ' # Column # ' ' # State
	    in
	       {@Socket write(vs: '\'' # S # '\'')}
	       lastFile <- File
	       lastLine <- Line
	       lastColumn <- Column
	       {Delay 1}   % this is needed for Emacs
	    end
	 end
      end
   end

   fun {GetOPI}
      {Property.condGet 'opi.compiler' false}
   end

   CondSend = condSend(interface:
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
				{{OPI getNarrator($)} M}
			     end
			  end)

end
