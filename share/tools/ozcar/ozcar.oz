%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

create Ozcar from Frontend with init

   meth init
      Frontend,init
   end

   meth exit
      %% turn the emulator into non-debugging mode
      Frontend,printStatus("resetting emulator...")
      {Debug.off}
      {Delay 2000}
      %% close emulator->ozcar interface
      Frontend,printStatus("closing emulator -> ozcar interface...")
      {Manager close}
      {Delay 2000}
      %% reset emacs and compiler flags, close oz->emacs interface
      Frontend,printStatus("closing oz -> emacs interface...")
      {ForAll [eval('(oz-debug-cont)' _)
	       eval('(oz-insert-file "tools/ozcar/reset_compiler")' _)
	       /* disconnect */] ThisEmacs}
      {Delay 2000}
      %% close frontend
      Frontend,printStatus("All done. Exiting...")
      {Delay 1000}
      {self.toplevel tkClose}
   end
end
