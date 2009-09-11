%%%
%%% Authors:
%%%   Gert Smolka <smolka@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Gert Smolka, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

declare
proc {Problem Solution}
   Vars
   [Sex Claim Truth        % Kibi
    Sex1 Truth1            % Parent 1
    Sex2 Truth2a Truth2b]  % Parent 2
   = Vars
   B1 B2 B3 B4
in
   %Vars:::0#1
   {FD.dom 0#1 Vars}
   
   %(Claim=:Sex)=:Truth
   {FD.reified.sum [Claim] '=:' Sex B1}
   {FD.sum [B1] '=:' Truth}
   
   %Sex+Truth>:0
   {FD.sum [Sex Truth] '>:' 0}
   
   %(Claim=:0)=:Truth1
   {FD.reified.sum [Claim] '=:' 0 B2}
   {FD.sum [B2] '=:' Truth1}
   
   %Sex1+Truth1>:0
   {FD.sum [Sex1 Truth1] '>:' 0}
   
   %(Sex=:1)=:Truth2a
   {FD.reified.sum [Sex] '=:' 1 B3}
   {FD.sum [B3] '=:' Truth2a}
   
   %(Truth=:0)=:Truth2b
   {FD.reified.sum [Truth] '=:' 0 B4}
   {FD.sum [B4] '=:' Truth2b}
   
   %Sex2+Truth2a+Truth2b=:2
   {FD.sum [Sex2 Truth2a Truth2b] '=:' 2}
   
   %Sex1\=:Sex2
   {FD.sumC [1 ~1] [Sex1 Sex2] '\\=:' 0}
   
   Solution=Sex#Sex1#Sex2
   {FD.distribute ff Vars}
end

{Show {SearchOne Problem}}
