%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%
%%% Last Change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%   http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor $
import
   Open
export
   'prepare' : Prepare
define
   local
      class TextFile from Open.file Open.text end

      local
	 fun {SkipSpaces S}
	    case S
	    of nil then nil
	    [] H|T then
	       if {Char.isSpace H} then {SkipSpaces T}
	       else {GetToken T [H]} end
	    end
	 end
	 fun {GetToken S Prefix}
	    case S
	    of nil then [{Reverse Prefix}]
	    [] H|T then
	       if {Char.isSpace H} then
		  {Reverse Prefix}|{SkipSpaces T}
	       else
		  {GetToken T H|Prefix}
	       end
	    end
	 end
      in
	 fun {Tokens S} {SkipSpaces S} end
      end

      local
	 C = {Cell.new 0}
      in
	 proc {IncCounter}
	    {Cell.assign C ({Cell.access C} + 1)}
	 end
	 fun {DecCounter}
	    N = {Cell.access C}
	 in
	    if N > 0 then {Cell.assign C (N - 1)} true else false end
	 end
      end
      
      fun {KeyFilter Ks}
	 case Ks
	 of "("|"(noreturn)"|")"|";"|Kr then {KeyFilter Kr}
	 [] K|Kr then
	    case K
	    of "__extension__"              then ""
	    [] "__signed"                   then ""
	    [] "__volatile"                 then "volatile"
	    [] "((noreturn));"              then ""
	    [] "__ssize_t"                  then "unsigned int"
	    [] "(__const"                   then "(const"
	    [] "__const"                    then "const"
	    [] "*__const"                   then "* "
	    [] "__restrict"                 then ""
	    [] "*__restrict"                then "* "
	    [] "**__restrict"               then "** "
	    [] "__attribute__"              then ";"
	    [] "__attribute__((format"      then {IncCounter} ";"
	    [] "__attribute__(("            then {IncCounter} ""
	    [] "__attribute__((__cdecl__))" then ""
	    [] "__attribute__((dllimport))" then ""
	    [] "dllimport"                  then ""
	    [] "))"                         then
	       if {DecCounter} then "" else "))" end
	    [] K                            then K
	    end|{KeyFilter Kr}
	 [] nil then nil
	 end
      end

      fun {RebuildLine Ts}
	 case Ts
	 of nil   then nil
	 [] T|nil then T
	 [] T|Tr  then T#' '#{RebuildLine Tr}
	 end
      end
      
      fun {FilterLine Line}
	 case Line
	 of ""   then ""
	 [] &#|_ then ""
	 [] Line then {VirtualString.toString
		       {RebuildLine {KeyFilter {Tokens Line}}}}
	 end
      end
      
      proc {FilterLines IF OF}
	 case {IF getS($)}
	 of false then skip
	 [] Line  then
	    case {FilterLine Line}
	    of ""   then skip
	    [] Line then {OF putS(Line)}
	    end
	    {FilterLines IF OF}
	 end
      end
   in
      proc {Prepare InFile OutFile}
	 IF = {New TextFile init(name:InFile flags:[read])}
	 OF = {New TextFile init(name:OutFile flags:[write create truncate])}
      in
	 {FilterLines IF OF}
	 {IF close}
	 {OF close}
      end
   end
end
