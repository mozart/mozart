%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

Ozcar =
{New
 class
    from
       Frontend
       ThreadManager
       StackManager
       SourceManager
    
    meth init
       Frontend,init
       ThreadManager,init
       SourceManager,init
    end
    
    meth exit
       {Debug.off}
       ThreadManager,close
       SourceManager,close
       {self.toplevel tkClose}
    end
 end init}
