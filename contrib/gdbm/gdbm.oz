%%% Copyright (c) by Denys Duchier, Jun 1997, Universität des Saarlandes
%%% ------------------------------------------------------------------
%%%
%%%

proc {$ Foreign GDBM}
   FLoad = Foreign.load
in
   %% open a new `local' to protect redefinitions of
   %% variables already in Base
   local
      foreign(cgdbm_open        :Open
              cgdbm_fetch       :Fetch
              cgdbm_store       :Store
              cgdbm_firstkey    :FirstKey
              cgdbm_nextkey     :NextKey
              cgdbm_close       :Close
              cgdbm_error       :Error
              cgdbm_delete      :Delete
              cgdbm_reorganize  :Reorganize
              cgdbm_bitor       :BitOr)
      %% Foreign.require will resolve this name in a platform
      %% dependent way
      = {FLoad 'tools/gdbm/gdbm.so'}
      fun {FlagsToVS L}
         case L of [F] then {FlagToVS F}
         elseof nil then nil
         elseof H|T then {FlagToVS H}#{FlagsToVS T} end
      end
      fun {FlagToVS F}
         case F of
            read     then 'r'
         [] write    then 'w'
         [] create   then 'c'
         [] truncate then 'n'
         [] new      then 'n'
         [] fast     then 'f'
         end
      end
      Modes=mode(owner:  access(read   :0400
                                write  :0200
                                execute:0100)
                 group:  access(read   :0040
                                write  :0020
                                execute:0010)
                 others: access(read   :0004
                                write  :0002
                                execute:0001)
                 all:    access(read   :0444
                                write  :0222
                                execute:0111))
      fun {ModesToInt Modes}
         {Record.foldLInd Modes ModeToInt 0}
      end
      fun {ModeToInt Who N Rights}
         MAP=Modes.Who
      in
         {FoldL Rights
          fun {$ N Right} {BitOr N MAP.Right} end N}
      end

   in
      class GDBM from BaseObject
         attr DB
         meth init(name     :Name
                   %% read write create truncate new fast
                   flags    :Flags <= [create]
                   mode     :Mode  <= mode(owner:[write] all:[read])
                   blockSize:Block <= unit)
            ZFlags = {FlagsToVS Flags}
            ZMode  = {ModesToInt Mode}
         in
            DB<-{Open Name ZFlags ZMode Block}
         end
         meth put(Key Val replace:Rep <= true)
            case {Store @DB Key Val Rep} of
               ~1 then raise putFailed end
            []  1  then raise keyExists end
            else skip end
         end
         meth get(Key Val)
            {Fetch @DB Key Val}
         end
         meth del(Key)
            case {Delete @DB Key} of
               ~1 then raise delFailed end
            else skip end
         end
         meth close
            {Close @DB}
         end
         meth firstkey($)
            {FirstKey @DB}
         end
         meth nextkey(Key $)
            {NextKey @DB Key}
         end
         meth reorganize
            case {Reorganize @DB} of
               0 then skip
            else raise reorganizeFailed end
            end
         end
      end
   end
end
