%%%
%%% Authors:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt and Benjamin Lorenz, 1997-1998
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
   Property(get condGet)
   System(printInfo)
   Error(messageToVirtualString)
   OS(getEnv stat tmpnam)
   Open(socket text file)
   Listener('class')
export
   getOPI:    GetOPI
   condSend:  CondSend
   interface: CompilerInterfaceEmacs
define
   TimeoutToConfigBar = 200
   TimeoutToUpdateBar = TimeoutToConfigBar

   fun {UnknownFile F}
      F == ''
   end

   local
      OsName      = {Property.get 'platform.os'}
      WindowsName = 'win32'

      local
	 FieldSeparator = if OsName == WindowsName then &; else &: end

	 fun {SystemPathList RawPath} H T in % RawPath must be of type string
	    {List.takeDropWhile RawPath fun {$ C} C \= FieldSeparator end H T}
	    if T == nil then [H]
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

	 OzPathEnv = case {OS.getEnv 'OZPATH'} of false then "."
		     elseof X then X
		     end
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
	       case {OS.stat Try}.type of reg then Try
	       else {DoLookupFile SearchListRest F OrigF}
	       end
	    catch system(...) then
	       {DoLookupFile SearchListRest F OrigF}
	    end
	 end
      end
   in
      fun {LookupFile F}
	 S   = {Atom.toString F}
	 Abs = case S                   % absolute path?
	       of     &/|_   then true
	       elseof _|&:|_ then OsName == WindowsName
	       else false end
      in
	 if Abs then
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
      class CompilerInterfaceEmacs from Listener.'class'
	 prop final
	 attr
	    Socket: unit BarSync: _ BarLock: {NewLock} Topped: false
	    lastFile: unit lastLine: unit lastColumn: unit
	 meth init(CompilerObject Host <= unit Print <= System.printInfo)
	    lock Sock Port in
	       thread
		  Sock = {New TextSocket server(port: ?Port)}
	       end
	       {Wait Port}
	       Socket <- Sock
	       {Print '\'oz-socket '#case Host of unit then ""
				     else '"'#Host#'" '
				     end#Port#'\''}
	       Listener.'class', init(CompilerObject Serve)
	    end
	 end
	 meth close()
	    Listener.'class', close()
	    case @Socket of unit then skip
	    elseof S then {S close()}
	    end
	 end
	 meth Write(VS)
	    case @Socket of unit then skip
	    elseof S then
	       try
		  {S write(vs: VS)}
	       catch system(os(os _ 32 ...) ...) then
		  Socket <- unit
	       end
	    end
	 end

	 meth readQueries()
	    case @Socket of unit then skip
	    elseof S then VS0 VS in
	       {S readQuery(?VS0)}
	       VS = case VS0 of ""#'\n'#VS1 then VS1 else VS0 end
	       {Listener.'class', getNarrator($)
		enqueue(feedVirtualString(VS))}
	       CompilerInterfaceEmacs, readQueries()
	    end
	 end

	 meth Serve(Ms)
	    case Ms of M|Mr then
	       case M of info(VS) then
		  CompilerInterfaceEmacs, Write(VS)
	       [] info(VS _) then
		  CompilerInterfaceEmacs, Write(VS)
	       [] message(Record _) then
		  case {Label Record} of error then
		     CompilerInterfaceEmacs, ToTop()
		  else skip
		  end
		  CompilerInterfaceEmacs,
		  Write({Error.messageToVirtualString Record})
	       [] displaySource(_ Ext VS) then Name File in
		  Name = {OS.tmpnam}#Ext
		  File = {New Open.file
			  init(name: Name
			       flags: [write create truncate])}
		  {File write(vs: VS)}
		  {File close()}
		  CompilerInterfaceEmacs, Write({VirtualString.toAtom
						 '\'oz-show-temp '#Name#'\''})
	       [] runQuery(_ _) then
		  Topped <- false
	       [] attention() then
		  CompilerInterfaceEmacs, ToTop()
	       else skip
	       end
	       CompilerInterfaceEmacs, Serve(Mr)
	    end
	 end
	 meth ToTop()
	    if {Property.get 'oz.standalone'} then skip
	    elseif @Topped then skip
	    else
	       CompilerInterfaceEmacs, Write(MSG_ERROR)
	       Topped <- true
	    end
	 end

	 meth bar(file:F line:L column:C state:S)
	    BarSync <- _ = unit
	    if {UnknownFile F} orelse L == unit then
	       CompilerInterfaceEmacs, removeBar()
	    else
	       CompilerInterfaceEmacs, MakeOzBar({LookupFile F} L C S)
	    end
	 end
	 meth delayedBar(file:F line:L column:C state:S<=unchanged) New in
	    BarSync <- New = unit
	    thread
	       {WaitOr New {Alarm TimeoutToUpdateBar}}
	       if {IsDet New} then skip else
		  CompilerInterfaceEmacs, bar(file:F line:L column:C state:S)
	       end
	    end
	 end
	 meth configureBar(State) New in
	    BarSync <- New = unit
	    thread
	       {WaitOr New {Alarm TimeoutToConfigBar}}
	       if {IsDet New} orelse @lastFile == unit then skip else
		  CompilerInterfaceEmacs,
		  MakeOzBar(@lastFile @lastLine @lastColumn State)
	       end
	    end
	 end
	 meth removeBar()
	    BarSync <- _ = unit
	    CompilerInterfaceEmacs, MakeOzBar('' 0 0 hide)
	 end
	 meth exit()
	    BarSync <- _ = unit
	    CompilerInterfaceEmacs, MakeOzBar('' 0 0 exit)
	 end
	 meth MakeOzBar(File Line Column State)
	    lock @BarLock then
	       S = 'oz-bar ' # File # ' ' # Line # ' ' # Column # ' ' # State
	    in
	       CompilerInterfaceEmacs, Write('\'' # S # '\'')
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
