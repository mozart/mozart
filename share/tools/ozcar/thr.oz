%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class ThreadManager from UrObject

   meth init
      skip
   end

   meth step(T)
      {Thread.resume T}
   end

   meth cont(T)
      {Dbg.stepmode T off}
      {Thread.resume T}
   end
   
   meth stack(T)
      {Ozcar stackThread(T)}
   end
end
