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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

%%
%% This functor defines a class encapsulating the whole of
%% the DICT protocol.
%%
%% Not implemented yet:
%% -- UTF-8
%% -- Authentication
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
   %% Default server host to connect to
   DEFAULT_SERVER = 'dict.org'
   %% Default port to connect to
   DEFAULT_PORT = 2628

   %% String send by the client to identify itself
   CLIENT_TEXT = 'Mozart client, http://mozart.ps.uni-sb.de/'

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
         elseof S then {DropCR S}
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
      meth writeLine(S)
         %% Write a command S to the server.
         %% Append the required return/linefeed character sequence.
         %% Raise an exception if the connection has been closed.
         try
            TextSocket, write(vs: S#'\r\n')
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
      meth 'define'(Word db: DB <= '*' $)
         %% Query for definitions for Word in database DB.
         lock
            case @socket of unit then
               {Exception.raiseError netdict(notConnected)} unit
            elseof Socket then
               {Socket writeLine('DEFINE '#DB#' '#{Escape Word})}
               case {Socket expect([150 552] $ _)} of 150 then
                  NetDictionary, GetDefinitions($)
               [] 552 then unit
               end
            end
         end
      end
      meth GetDefinitions($) Rest in
         case {@socket expect([151 250] $ ?Rest)} of 151 then Body in
            {@socket getTextual(?Body)}
            case {Argify Rest} of [Word DB DBName] then
               definition(word: Word db: DB dbname: DBName body: Body)|
               NetDictionary, GetDefinitions($)
            else
               {Exception.raiseError netdict(malformedDefinition Rest)} unit
            end
         [] 250 then nil
         end
      end
      meth match(Word db: DB <= '*' strategy: Strategy <= '.' $)
         %% Query for matches for Word in database DB using Strategy.
         lock
            case @socket of unit then
               {Exception.raiseError netdict(notConnected)} unit
            elseof Socket then
               {Socket writeLine('MATCH '#DB#' '#Strategy#' '#{Escape Word})}
               case {Socket expect([152 552] $ _)} of 152 then
                  NetDictionary, GetPairList($)
               [] 552 then unit
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
