%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Ozcar =
{New
 class
    from
       Frontend
       ThreadManager
       SourceManager
       StackManager
    
    meth init
       Frontend,init
       ThreadManager,init
       SourceManager,init
    end
    
    meth exit
       %% turn the emulator into non-debugging mode
       Frontend,printStatus("resetting emulator...")
       {Debug.off}
       {Delay 900}
       %% close emulator->ozcar interface
       Frontend,printStatus("closing emulator -> ozcar interface...")
       {Manager close}
       {Delay 900}
       Frontend,printStatus("All done. Exiting...")
       {Delay 900}
       {self.toplevel tkClose}
    end
 end init}
