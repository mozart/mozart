%%%
%%% Authors:
%%%   Andreas Sundstroem <andreas@sics.se>
%%%
%%% Copyright:
%%%   Andreas Sundstroem, 1998
%%%
%%% Last change:
%%%   $Date$Author: 
%%%   $Revision: 
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

\switch +gump +compilerpasses

functor
import
   GumpScanner('class')
   GumpParser('class')
   Property(get)
   Open(file)
   OS(getEnv)
   System(showInfo)
export
   parseFile: ParseFile
   parseVirtualString: ParseVirtualString
define
   \insert OzScanner.ozg
   \insert OzParser.ozg

   fun {ParseFile FileName Reporter GetSwitch Macros}
      \gumpscannerprefix ozfront
      MyScanner = {New OzScanner init(gump:{GetSwitch gump}
				      showInsert:{GetSwitch showinsert}
				      reporter:Reporter macros:Macros)}
      MyParser = {New OzParser init(allowDeprecated:{GetSwitch allowdeprecated}
				    'scanner':MyScanner
				    reporter:Reporter)}
      Status ParseTree
   in
      {MyScanner scanFile(FileName)}
      {MyParser parse(file(?ParseTree) ?Status)}
      {MyScanner close()}
      if Status then
	 ParseTree
      else
	 parseError
      end
   end

   fun {ParseVirtualString VS Reporter GetSwitch Macros}
      \gumpscannerprefix ozfront
      MyScanner = {New OzScanner init(gump:{GetSwitch gump}
				      showInsert:{GetSwitch showinsert}
				      reporter:Reporter macros:Macros)}
      MyParser = {New OzParser init(allowDeprecated:{GetSwitch allowdeprecated}
				    'scanner':MyScanner
				    reporter:Reporter)}
      Status ParseTree
   in
      {MyScanner scanVirtualString(VS)}
      {MyParser parse(file(ParseTree) ?Status)}
      {MyScanner close()}
      if Status then
	 ParseTree
      else
	 parseError
      end
   end
end
