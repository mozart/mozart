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

functor
import
   Property(get)
   Open(file text)
export
   'class': AuthorDBClass
define
   AuthorDBError = 'author database error'

   DBPath = {ByNeedFuture
	     fun {$}
		{String.tokens
		 {VirtualString.toString
		  {Property.get 'ozdoc.author.path'}} &:}
	     end}

   local
      class TextFile from Open.file Open.text
	 prop final
      end

      fun {FindFile DBPath FileName}
	 case DBPath of P|Pr then
	    try
	       {New TextFile init(name: P#'/'#FileName)}
	    catch _ then
	       {FindFile Pr FileName}
	    end
	 [] nil then unit
	 end
      end

      fun {ReadAuthors File Ps}
	 case {File getS($)} of false then [{List.toRecord author Ps}]
	 [] S=_|_ then Name Rest Value in
	    Name = {List.takeDropWhile S fun {$ C} C \= &: end $ &:|?Rest}
	    Value = {List.dropWhile Rest Char.isSpace}
	    {ReadAuthors File {String.toAtom {Map Name Char.toLower}}#Value|Ps}
	 [] nil then
	    {List.toRecord author Ps}|{ReadAuthors File nil}
	 end
      end
   in
      proc {ReadDB FileName Reporter ?DB} File in
	 File = {FindFile DBPath FileName}
	 case File of unit then DB = unit
	 else
	    DB = {NewDictionary}
	    {ForAll {ReadAuthors File nil}
	     proc {$ Author}
		case Author of author() then skip
		elsecase {CondSelect Author key unit} of unit then
		   {Reporter warn(kind: AuthorDBError
				  msg: 'missing key in author database entry'
				  items: [hint(l: 'Entry' m: oz(Author))])}
		else
		   {Dictionary.put DB {String.toAtom Author.key} Author}
		end
	     end}
	    {File close()}
	 end
      end
   end

   class AuthorDBClass
      attr DBs: unit Reporter: unit
      meth init(Rep)
	 DBs <- {NewDictionary}
	 Reporter <- Rep
      end
      meth get(To Key ?Author) DB in
	 AuthorDBClass, GetDB(To ?DB)
	 case DB of unit then
	    {@Reporter error(kind: AuthorDBError
			     msg: 'author database not found'
			     items: [hint(l: 'File name' m: To)])}
	    Author = author(firstname: Key)
	 elsecase {Dictionary.condGet DB {String.toAtom Key} unit} of unit then
	    {@Reporter error(kind: AuthorDBError
			     msg: 'author not found in database'
			     items: [hint(l: 'File name' m: To)
				     hint(l: 'Key' m: Key)])}
	    Author = author(firstname: Key)
	 elseof Entry then
	    Author = Entry
	 end
      end
      meth GetDB(To ?DB) DBID in
	 DBID = {String.toAtom To}
	 if {Dictionary.member @DBs DBID} then
	    DB = {Dictionary.get @DBs DBID}
	 else
	    DB = {ReadDB To @Reporter}
	    {Dictionary.put @DBs DBID DB}
	 end
      end
   end
end
