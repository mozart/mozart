%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% This functor defines a class encapsulating the whole of
%% the DICT protocol.
%%
%% Not implemented yet:
%% -- interpretation of the server banner
%% -- authentication
%% -- OPTION MIME
%%

functor
import
   Error(registerFormatter)
   Open(socket text)
export
   'class': NetDictionary
   defaultServer: DEFAULT_SERVER
   defaultPort: DEFAULT_PORT
prepare
   %% Name of default server to connect to
   DEFAULT_SERVER = 'dict.org'
   %% Default port to connect to
   DEFAULT_PORT = 2628

   %% String sent by the client to identify itself
   CLIENT_TEXT = 'Mozart client, http://www.mozart-oz.org/'

   fun {DropCR S}
      %% Discard the final return character of a line.
      case S of "\r" then ""
      elseof C1|Cr then C1|{DropCR Cr}
      [] nil then ""
      end
   end

   fun {DropSpace S}
      %% Discard leading whitespace.
      {List.dropWhile S
       fun {$ C} C == &  orelse C == &\t end}
   end

   %%
   %% Converting between UTF-8 and UCS-4 [RFC2044]
   %%
   %% UCS-4 range (hex.)    UTF-8 octet sequence (binary)
   %% 0000 0000-0000 007F   0xxxxxxx
   %% 0000 0080-0000 07FF   110xxxxx 10xxxxxx
   %% 0000 0800-0000 FFFF   1110xxxx 10xxxxxx 10xxxxxx
   %% 0001 0000-001F FFFF   11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
   %% 0020 0000-03FF FFFF   111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
   %% 0400 0000-7FFF FFFF   1111110x 10xxxxxx ... 10xxxxxx
   %%

   local
      SixBits = 0b1000000

      fun {CharToSeq C Acc}
	 case C of 0 then Acc
	 else
	    {CharToSeq (C div SixBits) (C mod SixBits)|Acc}
	 end
      end

      fun {AppendSeq Seq N Rest}
	 case Seq of I|Ir then
	    (I + N)|{AppendSeq Ir 0b10000000 Rest}
	 else
	    Rest
	 end
      end
   in
      fun {UCS4toUTF8 S}
	 case S of C|Cr then
	    if     C =< 0x0000007F then
	       C|{UCS4toUTF8 Cr}
	    elseif C =< 0x000007FF then
	       {AppendSeq {CharToSeq C nil} 0b11000000 {UCS4toUTF8 Cr}}
	    elseif C =< 0x0000FFFF then
	       {AppendSeq {CharToSeq C nil} 0b11100000 {UCS4toUTF8 Cr}}
	    elseif C =< 0x001FFFFF then
	       {AppendSeq {CharToSeq C nil} 0b11110000 {UCS4toUTF8 Cr}}
	    elseif C =< 0x03FFFFFF then
	       {AppendSeq {CharToSeq C nil} 0b11111000 {UCS4toUTF8 Cr}}
	    elseif C =< 0x7FFFFFFF then
	       {AppendSeq {CharToSeq C nil} 0b11111100 {UCS4toUTF8 Cr}}
	    else
	       {Exception.raiseError netdict(nonUCS4character C)} unit
	    end
	 [] nil then nil
	 end
      end
   end

   local
      SixBits = 0b1000000

      fun {SeqToChar N Seq Acc ?Rest}
	 case N of 0 then
	    Rest = Seq
	    Acc
	 elsecase Seq of I|Ir then
	    if I < 0b1000000 orelse I >= 0b11000000 then
	       {Exception.raiseError netdict(nonUTF8element I)}
	    end
	    {SeqToChar N - 1 Ir Acc * SixBits + (I - 0b10000000) ?Rest}
	 [] nil then
	    {Exception.raiseError netdict(tooShortUTF8character)} unit
	 end
      end
   in
      fun {UTF8toUCS4 Seq}
	 case Seq of I|Ir then
	    if     I >= 0b11111100 then Rest in
	       {SeqToChar 5 Ir I - 0b11111100 ?Rest}|{UTF8toUCS4 Rest}
	    elseif I >= 0b11111000 then Rest in
	       {SeqToChar 4 Ir I - 0b11111000 ?Rest}|{UTF8toUCS4 Rest}
	    elseif I >= 0b11110000 then Rest in
	       {SeqToChar 3 Ir I - 0b11110000 ?Rest}|{UTF8toUCS4 Rest}
	    elseif I >= 0b11100000 then Rest in
	       {SeqToChar 2 Ir I - 0b11100000 ?Rest}|{UTF8toUCS4 Rest}
	    elseif I >= 0b11000000 then Rest in
	       {SeqToChar 1 Ir I - 0b11000000 ?Rest}|{UTF8toUCS4 Rest}
	    elseif I >= 0b10000000 then
	       {Exception.raiseError netdict(nonUTF8character Seq)} unit
	    else
	       I|{UTF8toUCS4 Ir}
	    end
	 [] nil then nil
	 end
      end
   end
define
   %%
   %% Extended Socket Class for Protocol Basics
   %%

   class TextSocket from Open.socket Open.text
      prop final
      feat crash   % nullary procedure to invoke when server closes connection
      meth getS($)
	 %% Override `Open.socket,getS' to discard the final return character.
	 case Open.text, getS($) of false then false
	 elseof S then {DropCR {UTF8toUCS4 S}}
	 end
      end
      meth getTextLine($)
	 %% Read a single line of a (multi-line) text response.
	 %% A single period on a line has special meaning; return 'period'.
	 %% Other periods at the beginning of the line are doubled.
	 %% If the connection has been closed, return 'closed'.
	 case TextSocket, getS($) of false then closed
	 elseof "." then period
	 elseof &.|(S=&.|_) then S
	 elseof S then S
	 end
      end
      meth getTextual($)
	 %% Read a multi-line text response.
	 case TextSocket, getTextLine($) of closed then
	    {Exception.raiseError netdict(serverClosed unit)}
	    {self.crash}
	    unit
	 [] period then ""
	 elseof S then
	    S#'\n'#TextSocket, getTextual($)
	 end
      end
      meth expect(Ns ?N ?Rest)
	 %% Read a status response from the server.
	 %% A status response is a line starting with a three-digit
	 %% response code.  Ns is a list of the handled response codes;
	 %% return the actual response code in N and the rest of the
	 %% line in Rest.
	 case TextSocket, getS($) of false then
	    {Exception.raiseError netdict(serverClosed unit)}
	    {self.crash}
	 elseof S=(A|_) andthen {Char.isDigit A} then
	    N = {String.toInt {List.takeDropWhile S Char.isDigit $ ?Rest}}
	    if N == 420 orelse N == 421 then   % general error codes
	       {Exception.raiseError netdict(serverClosed 'Error '#N)}
	       {self.crash}
	    elseif {Member N Ns} then skip
	    else
	       {Exception.raiseError netdict(unexpectedResponse Ns N
					     {DropSpace S})}
	    end
	 elseof S then
	    {Exception.raiseError netdict(unexpectedResponse Ns unit S)}
	 end
      end
      meth writeLine(S) V in
	 %% Write a command S to the server.
	 %% Append the required return/linefeed character sequence.
	 %% Raise an exception if the connection has been closed.
	 V = {UCS4toUTF8 {VirtualString.toString S#'\r\n'}}
	 try
	    TextSocket, write(vs: V)
	 catch system(os(os 4: Text ...) ...) then
	    {self.crash}
	    {Exception.raiseError netdict(serverClosed Text)}
	 end
      end
   end

   %%
   %% Parsing a Status Response
   %%

   local
      proc {GetArg S Quote ?Arg ?Rest}
	 case S of C1|Cr then
	    if C1 == &\\ then
	       case Cr of C2|Crr then Argr in
		  Arg = C2|Argr
		  {GetArg Crr Quote ?Argr ?Rest}
	       [] nil then {Raise error}
	       end
	    elseif C1 == &" orelse C1 == &' then
	       if C1 == Quote then
		  {GetArg Cr unit ?Arg ?Rest}
	       elseif Quote == unit then
		  {GetArg Cr C1 ?Arg ?Rest}
	       else Argr in
		  Arg = C1|Argr
		  {GetArg Cr Quote ?Argr ?Rest}
	       end
	    elseif (C1 == &  orelse C1 == &\t) andthen Quote == unit then
	       Arg = nil
	       Rest = Cr
	    else Argr in
	       Arg = C1|Argr
	       {GetArg Cr Quote ?Argr ?Rest}
	    end
	 [] nil then
	    Arg = nil
	    Rest = nil
	 end
      end

      fun {GetArgs S} T in
	 T = {DropSpace S}
	 case T of nil then nil
	 else Arg Rest in
	    {GetArg T unit ?Arg ?Rest}
	    Arg|{GetArgs Rest}
	 end
      end
   in
      fun {Argify S}
	 try
	    {GetArgs S}
	 catch error then
	    {Exception.raiseError netdict(illegalArgString S)} unit
	 end
      end
   end

   %%
   %% Escaping Strings in Commands
   %%

   local
      fun {IsAtomChar C}
	 case C of &" then false
	 [] &' then false
	 [] &\\ then false
	 else &  < C
	 end
      end

      fun {EscapeSub S}
	 case S of C|Cr then
	    if C == &" orelse C == &\\ orelse C < &  then
	       &\\|C|{EscapeSub Cr}
	    else
	       C|{EscapeSub Cr}
	    end
	 [] nil then "\""
	 end
      end
   in
      fun {Escape VS} S in
	 S = {VirtualString.toString VS}
	 if S \= "" andthen {All S IsAtomChar} then S
	 else &"|{EscapeSub S}
	 end
      end
   end

   %%
   %% Main Class Encapsulating the DICT Protocol
   %%

   class NetDictionary
      prop locking
      attr socket serverBanner
      meth init(Server <= unit Port <= DEFAULT_PORT)
	 %% If Server is non-unit, open a connection to it at Port.
	 socket <- unit
	 serverBanner <- ""
	 if Server \= unit then
	    NetDictionary, connect(Server Port)
	 end
      end
      meth connect(Server <= DEFAULT_SERVER Port <= DEFAULT_PORT) Socket in
	 %% Open a connection to Server at Port.
	 %% If a connection is currently open, close it before.
	 lock
	    if @socket \= unit then
	       NetDictionary, close()
	    end
	    Socket = {New TextSocket client(host: Server port: Port)}
	    Socket.crash = proc {$} NetDictionary, Crash() end
	    try
	       serverBanner <- {Socket expect([220] _ $)}
	       {Socket writeLine('CLIENT '#CLIENT_TEXT)}
	       {Socket expect([250] _ _)}
	       socket <- Socket
	    catch E then
	       {Socket close()}
	       {Raise E}
	    end
	 end
      end
      meth getBanner($)
	 %% Return the banner sent by the server upon connection.
	 case @socket of unit then ""
	 else @serverBanner
	 end
      end
      meth close()
	 %% Close the current connection (if any).
	 %% Send a QUIT command and wait for the status response.
	 lock
	    case @socket of unit then skip
	    elseof Socket then
	       {Socket writeLine('QUIT')}
	       try
		  {Socket expect([221] _ _)}
	       finally
		  {Socket close()}
		  socket <- unit
	       end
	    end
	 end
      end
      meth Crash()
	 socket <- unit
      end
      meth status($)
	 %% Send a STATUS command and return the status response.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)} unit
	    elseof Socket then
	       {Socket writeLine('STATUS')}
	       {DropSpace {Socket expect([210] _ $)}}
	    end
	 end
      end
      meth showServer(?Text)
	 %% Send a SHOW SERVER command and return the text reponse.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)}
	    elseof Socket then
	       {Socket writeLine('SHOW SERVER')}
	       {Socket expect([114] _ _)}
	       {Socket getTextual(?Text)}
	       {Socket expect([250] _ _)}
	    end
	 end
      end
      meth showInfo(DBName ?Text)
	 %% Send a SHOW INFO command and return the text reponse.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)}
	    elseof Socket then
	       {Socket writeLine('SHOW INFO '#DBName)}
	       {Socket expect([112] _ _)}
	       {Socket getTextual(?Text)}
	       {Socket expect([250] _ _)}
	    end
	 end
      end
      meth 'define'(Word db: DB <= '*' count: Count <= _ $)
	 %% Query for definitions for Word in database DB.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)} unit
	    elseof Socket then Rest in
	       {Socket writeLine('DEFINE '#DB#' '#{Escape Word})}
	       case {Socket expect([150 552] $ ?Rest)} of 150 then
		  try
		     Count = {String.toInt {Argify Rest}.1}
		  catch error(...) then
		     {Exception.raiseError netdict(malformedResponse 150 Rest)}
		  end
		  NetDictionary, GetDefinitions($)
	       [] 552 then
		  Count = 0
		  unit
	       end
	    end
	 end
      end
      meth GetDefinitions(?Ds) Rest in
	 case {@socket expect([151 250] $ ?Rest)} of 151 then
	    case {Argify Rest} of [Word DB DBName] then Dr Body in
	       Ds = definition(word: Word db: DB dbname: DBName body: Body)|Dr
	       {@socket getTextual(?Body)}
	       NetDictionary, GetDefinitions(?Dr)
	    else
	       Ds = nil
	       {Exception.raiseError netdict(malformedDefinition Rest)}
	    end
	 [] 250 then
	    Ds = nil
	 end
      end
      meth match(Word db: DB <= '*' strategy: Strategy <= '.'
		 count: Count <= _ $)
	 %% Query for matches for Word in database DB using Strategy.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)} unit
	    elseof Socket then Rest in
	       {Socket writeLine('MATCH '#DB#' '#Strategy#' '#{Escape Word})}
	       case {Socket expect([152 552] $ ?Rest)} of 152 then
		  try
		     Count = {String.toInt {Argify Rest}.1}
		  catch error(...) then
		     {Exception.raiseError netdict(malformedResponse 152 Rest)}
		  end
		  NetDictionary, GetPairList($)
	       [] 552 then
		  Count = 0
		  unit
	       end
	    end
	 end
      end
      meth showDatabases($)
	 %% Send a SHOW DATABASES command and return the text response.
	 %% This consists of a list of pairs ID#Name.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)} unit
	    elseof Socket then
	       {Socket writeLine('SHOW DATABASES')}
	       {Socket expect([110] _ _)}
	       NetDictionary, GetPairList($)
	    end
	 end
      end
      meth showStrategies($)
	 %% Send a SHOW STRATEGIES command and return the text response.
	 %% This consists of a list of pairs ID#Name.
	 lock
	    case @socket of unit then
	       {Exception.raiseError netdict(notConnected)} unit
	    elseof Socket then
	       {Socket writeLine('SHOW STRATEGIES')}
	       {Socket expect([111] _ _)}
	       NetDictionary, GetPairList($)
	    end
	 end
      end
      meth GetPairList($)
	 case {@socket getTextLine($)} of closed then
	    {Exception.raiseError netdict(serverClosed unit)}
	    NetDictionary, Crash()
	    unit
	 [] period then
	    {@socket expect([250] _ _)} nil
	 elseof S then
	    case {Argify S} of [A B] then A#B|NetDictionary, GetPairList($)
	    else
	       {Exception.raiseError netdict(malformedPair S)} unit
	    end
	 end
      end
   end

   %%
   %% Formatting Error Exceptions
   %%

   {Error.registerFormatter netdict
    fun {$ E} T in
       T = 'net dictionary error'
       case E of netdict(serverClosed Reason) then
	  error(kind: T
		msg: 'Server closed connection'
		items: case Reason of unit then nil
		       else [hint(l: 'Reason' m: Reason)]
		       end)
       elseof netdict(illegalArgString ArgString) then
	  error(kind: T
		msg: 'Illegal argument string received from server'
		items: [hint(l: 'Got' m: ArgString)])
       elseof netdict(serverError Response) then
	  error(kind: T
		msg: 'Server error'
		items: [hint(l: 'Response' m: Response)])
       elseof netdict(notConnected) then
	  error(kind: T
		msg: 'Not connected')
       elseof netdict(unexpectedResponse Expected N Response) then
	  error(kind: T
		msg: 'Unexpected response from server'
		items: [case Expected of I1|Ir then
			   hint(l: 'Expected one of'
				m: {FoldL Ir
				    fun {$ In I} In#' or '#I end I1})
			else hint(l: 'Expected' m: Expected)
			end
			hint(l: 'Response' m: case N of unit then Response
					      else N#' '#Response
					      end)])
       elseof netdict(malformedResponse Code Rest) then
	  error(kind: T
		msg: 'Malformed response'
		items: [hint(l: 'Response code' m: Code)
			hint(l: 'Response text' m: Rest)])
       elseof netdict(malformedDefinition Rest) then
	  error(kind: T
		msg: 'Malformed definition response'
		items: [hint(l: 'Response' m: Rest)])
       elseof netdict(malformedPair String) then
	  error(kind: T
		msg: 'Malformed pair'
		items: [hint(l: 'Got' m: String)])
       else
	  error(kind: T
		items: [line(oz(E))])
       end
    end}
end
