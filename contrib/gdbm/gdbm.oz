%%% -*-oz-*-
%%% Copyright (c) by Denys Duchier, Jun 1997, Universitšt des Saarlandes
%%% ------------------------------------------------------------------
%%%
%%%

functor prop once
import
   MOD @ 'gdbm.so{native}'
   Finalize
   Error ErrorRegistry
export
   'class': GDBM
define
   Register = Finalize.register
   %% open a new `local' to protect redefinitions of
   %% variables already in Base
   local
      Open       = MOD.open
      Fetch      = MOD.fetch
      Store      = MOD.store
      FirstKey   = MOD.firstkey
      NextKey    = MOD.nextkey
      Close      = MOD.close
      ERROR      = MOD.error
      Delete     = MOD.delete
      Reorganize = MOD.reorganize
      BitOr      = MOD.bitor
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
      proc {FREE DB}
         try {Close DB} catch _ then skip end
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
            {Register @DB FREE}
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
      %%
      %% Error Handlers
      %%
      local
         fun {GdbmFormatter Exc}
            E = {Error.dispatch Exc}
            T = 'GDBM Error'
         in
            case E
            of gdbm(Proc Errno Arg) then
               case Errno
               of alreadyClosed then
                  {Error.format T
                   'Database already closed'
                   [hint(l:'Operation' m:Proc)]
                   Exc}
               elsecase {IsInt Errno} then
                  Msg = {ERROR Errno}
               in
                  {Error.format T Msg
                   [hint(l:'Operation' m:Proc)
                    hint(l:'Using'     m:Arg)]
                   Exc}
               else
                  {Error.formatGeneric T Exc}
               end
            else
               {Error.formatGeneric T Exc}
            end
         end
      in
         {ErrorRegistry.put gdbm GdbmFormatter}
      end
   end
end
