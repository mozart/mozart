%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1997-1998
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

%%
%% This file contains the test programs from the Gump Manual.
%%

\switch +gump

\gumpscannerprefix lambda
\insert gump/examples/LambdaScanner.ozg

\switch +gumpparseroutputsimplified +gumpparserverbose
\gumpparserexpect 0
\insert gump/examples/LambdaParser.ozg

declare LambdaIn = 'Lambda.in'

%-----------------------------------------------------------------------
% Testing the Scanner:

local
   MyScanner = {New LambdaScanner init()}
   proc {GetTokens} T V in
      {MyScanner getToken(?T ?V)}
      case T of 'EOF' then
	 {System.showInfo 'End of file reached.'}
      else
	 {Show T#V}
	 {GetTokens}
      end
   end
in
   {MyScanner scanFile(LambdaIn)}
   {GetTokens}
   {MyScanner close()}
end

%-----------------------------------------------------------------------
% Testing the Parser:

local
   MyScanner = {New LambdaScanner init()}
   MyParser = {New LambdaParser init(MyScanner)}
   Definitions Terms Status
in
   {MyScanner scanFile(LambdaIn)}
   {MyParser parse(program(?Definitions ?Terms) ?Status)}
   {MyScanner close()}
   if Status then
      {Inspect Definitions}
      {Inspect Terms}
      {System.showInfo 'accepted'}
   else
      {System.showInfo 'rejected'}
   end
end
