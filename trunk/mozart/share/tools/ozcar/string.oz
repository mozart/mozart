%%%
%%% Authors:
%%%   Author's name (Author's email address)
%%%
%%% Contributors:
%%%   optional, Contributor's name (Contributor's email address)
%%%
%%% Copyright:
%%%   Organization or Person (Year(s))
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% The "Str" module offers some of the well known
%%% string functions of the C library.
%%%

local

   %% strcat: {Str.cat "Hallo" " Benni"} --> "Hallo Benni"
   fun {StrCat Dst Src}
      {Append Dst Src}
   end

   %% strchr: {Str.chr "Hallo Benni" & } --> " Benni"
   fun {StrChr S C}
      case S of C0|R then
	 case C == C0 then S
	 else {StrChr R C}
	 end
      [] nil then nil
      end
   end

   %% strrchr: {Str.rchr "/var/spool/mail/benni" &/} --> "/benni"
   local
      fun {DoStrRChr S A C}
	 case S == nil then nil
	 elsecase S.1 == C then S.1|A
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
