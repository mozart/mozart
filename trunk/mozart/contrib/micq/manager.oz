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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
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
	 OS(getEnv:GetEnv stat:Stat uName:UName
	    time:Time gmTime:GT localTime:LT
	    rand:Rand srand:Srand randLimits:RL)
      export
	 getEnv:GetEnv
	 stat:Stat
	 uName:UName
	 time:Time
	 gmTime:GT
	 localTime:LT
	 rand:Rand
	 srand:Srand
	 randLimits:RL
      define
	 skip
      end
      
      functor
	 SecureProperty
      import
	 Property(get:Get)
      export
	 get:Get
      define skip end

      functor SecureOpen
      import
	 Open(file:File)
	 TkTools Tk
      export
	 file:SecureFile
      define
	 class SecureFile from File
	    meth init(name:F flags:Flags ...)=M
	       IsOk
	       T={New TkTools.dialog tkInit(title:"Application Security Manager"
					    buttons:['Okay'#proc {$} IsOk=true end 
						      'Cancel' # proc{$} IsOk=false end]
					    default:1)}
	       L=if {Member write Flags} then
		    {New Tk.label tkInit(parent:T text:"Application requires to save file: "#F)}
		 else
		    {New Tk.label tkInit(parent:T text:"Application requires to load file: "#F)}
		 end
	    in
	       {Tk.send pack(L)}
	       {Wait IsOk}
	       {T tkClose}
	       if IsOk==true then
		  File, M
	       else
		  raise operationCanceled() end
	       end
	    end
	 end
      end
      
      functor
	 NotAccessible
      define skip end
   in
      %% Insert all "secure" modules into the untrusted manager
      {ForAll ['System'#SecureSystem 'OS'#SecureOS 'Property'#SecureProperty
	       'Open'#SecureOpen 'Module'#NotAccessible]
       proc{$ F} Module={TM apply(url:'' F.2 $)} in
	  {SM enter(name:F.1 Module)}  
       end}
      SM
   end
end




