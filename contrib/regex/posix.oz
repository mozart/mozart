%%% Copyright © by Denys Duchier, Dec 1997, Universität des Saarlandes

fun {$ IMPORT}
   Require  = IMPORT.'SP'.'Foreign'.require
   Register = IMPORT.'SP'.'Finalize'.register
   regex(compile:COMPILE execute:EXECUTE free:FREE flags:FLAGS)
   = {Require 'tools/regex/posix.dl'
      regex(compile:3 execute:5 free:1 flags:1)}
   flag(EXTENDED ICASE NEWLINE NOSUB NOTBOL NOTEOL)={FLAGS}
   Flags = flag(extended:EXTENDED
                icase   :ICASE
                newline :NEWLINE
                nosub   :NOSUB
                notbol  :NOTBOL
                noteol  :NOTEOL)
   IsRE = {NewName}
   DefaultInit = EXTENDED+NEWLINE
   DefaultExec = 0
   class Regex
      feat !IsRE:!IsRE          %private feature identifies Regex
      prop final
      attr re
      meth init(TXT flags:FLAGS<=DefaultInit)
         re<-{COMPILE TXT FLAGS}
         {Register @re FREE}
      end
      meth exec(TXT index:IDX<=0 flags:FLAGS<=DefaultExec $)
         {EXECUTE @re TXT IDX FLAGS $}
      end
   end
   fun {Regex_Is X} {IsObject X} andthen {HasFeature X IsRE} end
   fun {Regex_Compile Pattern} {New Regex init(Pattern)} end
   fun {Regex_Execute RE TXT} {RE exec(TXT $)} end
   fun {Regex_Match  PAT TXT}
      {Regex_Execute
       case {Regex_Is PAT} then PAT else {Regex_Compile PAT} end TXT}
   end
in
   regex('class'        : Regex
         'is'           : Regex_Is
         'compile'      : Regex_Compile
         'execute'      : Regex_Execute
         'match'        : Regex_Match
         'flag'         : Flags
        )
end
