%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class ThreadDebugger from Emacs
   feat
      Thr

   meth init(thr:T)
      self.Thr = T
      {Dbg.stepmode T on}
   end
   
   meth step
      {Thread.resume self.Thr}
   end

   meth stack
      {Browse {Dbg.taskstack self.Thr}}
   end
end
