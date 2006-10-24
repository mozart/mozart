%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1999
%%%
%%% Last change:
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

functor
import
   Application(getArgs exit)
   Property(get)
   System(printError printInfo)
   Error(printException)
   Pickle(save)
   TextPickle(load) at 'x-oz://contrib/compat/TextPickle.ozf'
prepare
   Spec = record('in'(single char: &i type: string optional: false)
		 'out'(single char: &o type: string optional: false)
		 'help'(rightmost char: [&h &?] default: false))
define
   Args HasError
   try
      Args = {Application.getArgs Spec}
      HasError = Args.1 \= nil
   catch error(ap(usage VS) ...) then
      {System.printError VS#'\n'}
      HasError = true
   end
   if HasError orelse Args.help then
      {System.printInfo
       'Usage: '#{Property.get 'application.url'}#' <option> ...\n'#
       '-h, -?, --help        Display this text.\n'#
       '-i FILE, --in=FILE    Read text pickle from FILE.\n'#
       '-o FILE, --out=FILE   Write binary pickle to FILE.\n'}
      {Application.exit if HasError then 2 else 0 end}
   else V Inconvertible in
      try
	 {TextPickle.load Args.'in' ?V ?Inconvertible}
	 case Inconvertible of nil then
	    {Pickle.save V Args.out}
	    {Application.exit 0}
	 else
	    {System.printError 'conversion failed: procedures found\n'}
	    {Application.exit 1}
	 end
      catch E then
	 {Error.printException E}
	 {System.printError 'conversion failed\n'}
	 {Application.exit 1}
      end
   end
end
