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
%% The Main Application
%%
%% Parse the command line, initialize the connection,
%% wait for the graphical front-end window to be closed
%% and exit.
%%

functor
import
   Application(getCmdArgs exit)
   System(printError)
   Property(get)
   TkDictionary('class')
prepare
   ArgSpec = record(help(rightmost char: [&? &h] default: false)
		    server(single char: &s type: string default: 'dict.org')
		    host(alias: server)
		    port(single char: &p type: int default: 2628))

   UsageString =
   '--help, -?, -h  Display this message.\n'#
   '--server=HOST, --host=HOST, -s HOST\n'#
   '--port=PORT, -p PORT\n'#
   '                Initially try to connect to HOST on PORT.\n'
define
   proc {Usage VS N}
      {System.printError
       VS#'Usage: '#{Property.get 'application.url'}#' <option> ...\n'#
       UsageString}
      {Application.exit N}
   end

   try Args in
      Args = {Application.getCmdArgs ArgSpec}
      if Args.help then
	 {Usage "" 0}
      end
      {Wait {New TkDictionary.'class' init(Args.server Args.port)}.closed}
      {Application.exit 0}
   catch error(ap(usage VS) ...) then
      {Usage 'Usage error: '#VS#'\n' 2}
   end
end
