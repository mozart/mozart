%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%% The "Str" module offers some of the well known
%% string functions of the C library.


local

   %% strcat: {Str.cat "Hallo" " Benni"} --> "Hallo Benni"
   fun {StrCat Dst Src}
      {Append Dst Src}
   end

   %% strchr: {Str.chr "Hallo Benni" & } --> " Benni"
   fun {StrChr S C}
      case S == nil then nil
      elsecase S.1 == C then S.1|S.2
      else {StrChr S.2 C}
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

