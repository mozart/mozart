%%%
%%% Author:
%%%   Thorsten Brunklaus <bruni@ps.uni-sb.de>
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Thorsten Brunklaus, 2001
%%%   Denys Duchier, 2003
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

      %% this is the tokenizer. punctuation tokens are "(){};,"
      %% character tokens are enclosed in single quotes. a sequence
      %% of alphanum or _ chars is considered to form an identifier
      %% token, although that also includes integers in decimal and
      %% hexadecimal notation. spaces are between tokens. any
      %% sequence of contiguous other chars is considered to be a
      %% token, since it doesn't matter.

      local
	 fun {IsPunct C} {Member C "(){};,"} end
	 fun {IsAlpha C} C==&_ orelse {Char.isAlNum C} end
	 %% this function is lazy so that the entire tokenizer
	 %% becomes lazy
	 fun lazy {SkipSpaces S}
	    case S
	    of nil then nil
	    [] &'|T then {GetCharToken T [&']}
	    [] H|T then
	       if {Char.isSpace H} then {SkipSpaces T}
	       elseif {IsPunct  H} then [H]|{SkipSpaces T}
	       elseif {IsAlpha  H} then {GetIdentifier T [H]}
	       else {GetToken T [H]} end
	    end
	 end
	 fun {GetCharToken S Prefix}
	    case S
	    of &'|T then {Reverse &'|Prefix}|{SkipSpaces T}
	    [] &\\|C|T then {GetCharToken T C|&\\|Prefix}
	    [] H|T then {GetCharToken T H|Prefix}
	    end
	 end
	 fun {GetIdentifier S Prefix}
	    case S
	    of nil then [{Reverse Prefix}]
	    [] H|T then
	       if {IsAlpha H} then {GetIdentifier T H|Prefix}
	       else {Reverse Prefix}|{SkipSpaces S} end
	    end
	 end
	 fun {GetToken S Prefix}
	    case S
	    of nil then [{Reverse Prefix}]
	    [] H|T then
	       if {Char.isSpace H} orelse {IsPunct H} orelse {IsAlpha H}
	       then {Reverse Prefix}|{SkipSpaces S}
	       else {GetToken T H|Prefix} end
	    end
	 end
      in
	 fun {Tokenize S} {SkipSpaces S} end
      end

      %% this function rewrites the stream of tokens, replacing certain
      %% tokens or sequences of tokens
      
      fun lazy {Sanitize Tokens}
	 case Tokens
	 of nil then nil
	 [] H|T then
	    case H
	    of "__extension__" then {Sanitize T}
	    [] "__signed"      then {Sanitize T}
	    [] "__volatile"    then "volatile"|{Sanitize T}
	    [] "__const"       then "const"|{Sanitize T}
	    [] "__restrict"    then {Sanitize T}
	    [] "__attribute__" then {Sanitize {SkipAttribute T}}
	    [] "dllimport"     then {Sanitize T}
	       %% if the definition of __ssize_t is somehow missing on
	       %% some platform then it should be added to gtkraw
	       %% rather than replaced here.  This replacement does
	       %% not work (1) precisely when typedefing __ssize_t,
	       %% and (2) on platforms where it happens to be longer
	       %% (e.g. MacOSX).
	       %% [] "__ssize_t"     then "unsigned"|"int"|{Sanitize T}
	    else H|{Sanitize T} end
	 end
      end

      %% this function skips over the balanced ((...)) of an __attribute__
      
      fun {SkipAttribute T}
	 fun {Loop L N}
	    case L
	    of "("|T then {Loop T N+1}
	    [] ")"|T then
	       if N==1 then T else {Loop T N-1} end
	    [] _|T then {Loop T N}
	    end
	 end
      in
	 case T of "("|"("|T then {Loop T 2} end
      end

      %% this function removes lines beginning with #
      %% i.e. line directives

      fun {RemoveDirectiveLines IN}
	 %% lazily read each line and discard those that
	 %% correspond to directives
	 fun lazy {Loop}
	    Line = {IN getS($)}
	 in
	    case Line
	    of false then nil
	    [] &#|_  then {Loop}
	    else {Append Line &\n|{Loop}} end
	 end
      in
	 {Loop}
      end

      EOL_Tokens = [";" "{" ","]
   in
      proc {Prepare InFile OutFile}
	 IF = {New TextFile init(name:InFile flags:[read])}
	 OF = {New TextFile init(name:OutFile flags:[write create truncate])}
      in
	 try
	    %% note that the list of tokens is computed lazily
	    for Token in {Sanitize {Tokenize {RemoveDirectiveLines IF}}} do
	       {OF write(vs:Token#
			 %% keep the minus sign contiguous with what follows:
			 %% presumably an integer.  output newlines after
			 %% certain tokens (specified by EOL_Tokens).
			 %% otherwise separate tokens by a space
			 if Token=="-" then ''
			 elseif {Member Token EOL_Tokens} then '\n'
			 else ' ' end)}
	    end
	 finally
	    {IF close}
	    {OF close}
	 end
      end
   end
end
