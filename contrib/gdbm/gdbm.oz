%%% Copyright (c) by Denys Duchier, Jun 1997, Universität des Saarlandes
%%% ------------------------------------------------------------------
%%%
%%%
declare
proc {NewGdbm Standard GDBM}
\insert 'Standard.env'
   = Standard
in
   %% open a new `local' to protect redefinitions of
   %% variables already in Base
   local
      cgdbm(open:Open fetch:Fetch store:Store
            firstkey:FirstKey nextkey:NextKey close:Close
            error:Error delete:Delete reorganize:Reorganize
            bitor:BitOr)
      %% Foreign.require will resolve this name in a platform
      %% dependent way
      = {Foreign.require 'tools/gdbm/gdbm.dl'
         cgdbm(open:5 fetch:3 store:5
               firstkey:2 nextkey:3 close:1
               error:2 delete:3 reorganize:2
               bitor:3)}
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
         {List.foldL Rights
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

/*

{Save NewGdbm '/home/ps-home2/duchier/Oz/tools/gdbm/gdbm.ozc'
 x(url:'file:/home/ps-home2/duchier/Oz/tools/gdbm/gdbm.ozc'
   include:['file:/home/ps-home2/duchier/Oz/tools/gdbm/gdbm.ozc']
   components:['file:/project/ps/soft/oz-devel/oz/lib/Base.ozc']
   resources:nil)}

{SmartSave NewGdbm
 '/tmp/gdbm.ozc'
 'file:/tmp/gdbm.ozc'
 ['file:/tmp/gdbm.ozc']
 nil nil}}

*/
