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
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor prop once
import
   Open(file text)
export
   'class': AuthorDBClass
define
   DBPath = ['.' '/home/ps-home/kornstae/mozart/doc/']

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
      proc {ReadDB FileName ?DB} File in
	 File = {FindFile DBPath FileName}
	 case File of unit then DB = unit
	 else
	    DB = {NewDictionary}
	    {ForAll {ReadAuthors File nil}
	     proc {$ Author}
		case Author of author() then skip
		elsecase {CondSelect Author key unit} of unit then
		   {Exception.raiseError
		    ozDoc(authorDB authorWithoutKey FileName)}
		else
		   {Dictionary.put DB {String.toAtom Author.key} Author}
		end
	     end}
	    {File close()}
	 end
      end
   end

   class AuthorDBClass
      attr DBs: unit
      meth init()
	 DBs <- {NewDictionary}
      end
      meth get(To Key ?Author) DB in
	 AuthorDBClass, GetDB(To ?DB)
	 case DB of unit then
	    {Exception.raiseError ozDoc(authorDB nonExistentDB To)}
	 elsecase {Dictionary.condGet DB {String.toAtom Key} unit} of unit then
	    {Exception.raiseError ozDoc(authorDB nonExistentKey To Key)}
	 elseof Entry then
	    Author = Entry
	 end
      end
      meth GetDB(To ?DB) DBID in
	 DBID = {String.toAtom To}
	 if {Dictionary.member @DBs DBID} then
	    DB = {Dictionary.get @DBs DBID}
	 else
	    DB = {ReadDB To}
	    {Dictionary.put @DBs DBID DB}
	 end
      end
   end
end
