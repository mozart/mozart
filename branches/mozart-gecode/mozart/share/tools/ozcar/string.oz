%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

local

   %% strcat: {Str.cat "Hallo" " Benni"} --> "Hallo Benni"
   fun {StrCat Dst Src}
      {Append Dst Src}
   end

   %% strchr: {Str.chr "Hallo Benni" & } --> " Benni"
   fun {StrChr S C}
      case S of C0|R then
	 if C == C0 then S
	 else {StrChr R C}
	 end
      [] nil then nil
      end
   end

   %% strrchr: {Str.rchr "/var/spool/mail/benni" &/} --> "/benni"
   local
      fun {DoStrRChr S A C}
	 if S == nil then nil
	 elseif S.1 == C then S.1|A
	 else {DoStrRChr S.2 S.1|A C}
	 end
      end
   in
      fun {StrRChr S C}
	 {DoStrRChr {Reverse S} nil C}
      end
   end

in

   Str = str(cat : StrCat
	     chr : StrChr
	     rchr: StrRChr)

end
