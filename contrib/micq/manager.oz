%%%
%%% Authors:
%%%   Nils Franzén (nilsf@sics.se)
%%%   Simon Lindblom (simon@sics.se)
%%%
%%% Copyright:
%%%   Nils Franzén, 1998
%%%   Simon Lindblom, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Module
export
   newSecureManager:SecureManager
define
   fun{SecureManager}
      %% We need to use two module managers to achieve this!
      SM={New Module.manager init()} % Secure Manager
      TM={New Module.manager init()} % Temporary Manager

      %% Make some restrictions on the imported
      %% modules in the untrustedmanager
      %% System, OS, Open, Property
      functor
         SecureSystem
      import
         System(show:Show showError:ShowError showInfo:ShowInfo)
      export
         show:Show
         showError:ShowError
         showInfo:ShowInfo
      define skip end

      functor
         SecureOS
      import
         OS(getEnv:GetEnv time:Time rand:Rand file:File)
         TkTools Tk
      export
         getEnv:GetEnv
         time:Time
         rand:Rand
         file:SecureFile
      define
         class SecureFile from File
            meth init(name:File flags:Flags ...)=M
               IsOk
               T={New TkTools.dialog
                  tkInit(title:"Start Application"
                         buttons: ['Okay'#proc {$} IsOk=true end
                                   'Cancel' # proc{$} IsOk=false end]
                         default: 1)}
               L=if {Member Flags create} then
                    {New Tk.label tkInit(parent:T text:"Application requires to save file: "#File)}
                 else
                    {New Tk.label tkInit(parent:T text:"Application requires to load file: "#File)}
                 end
            in
               if IsOk==true then
                  File, M
               else
                  raise operationCanceled() end
               end
            end
         end
      end

      functor
         SecureProperty
      import
         Property(get:Get)
      export
         get:Get
      define skip end

      functor
         NotAccessible
      define skip end
   in
      %% Insert all "secure" modules into the untrusted manager
      {ForAll ['System'#SecureSystem 'OS'#SecureOS 'Property'#SecureProperty
               'Open'#NotAccessible 'Module'#NotAccessible]
       proc{$ F} Module={TM apply(url:'' F.2 $)} in
          {SM enter(name:F.1 Module)}
       end}
      SM
   end
end
