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
export
   'flatten' : Flatten
define
   local
      fun {IsBinTuple T}
	 {IsTuple T} andthen {Width T} == 2
      end
      fun {FetchLeft L T Xs}
	 if {IsBinTuple T} andthen {Label T} == L
	 then {FetchLeft L T.1 T.2|Xs}
	 else T|Xs
	 end
      end
      fun {FetchRight L T Xs}
	 if {IsBinTuple T} andthen {Label T} == L
	 then {FetchRight L T.2 T.1|Xs}
	 else T|Xs
	 end
      end
   in
      fun {Flatten T}
	 if {IsBinTuple T}
	 then
	    L  = {Label T}
	    Ls = case L
		 of 'enumerator_list' then {Reverse {FetchRight L T.2 [T.1]}}
		 [] 'specifier_list'  then {Reverse {FetchRight L T.2 [T.1]}}
		 [] L                 then {FetchLeft L T.1 [T.2]}
		 end
	 in
	    {Record.map {List.toTuple L Ls} Flatten}
	 elseif {IsTuple T}
	 then {Record.map T Flatten}
	 else T
	 end
      end
   end
end
