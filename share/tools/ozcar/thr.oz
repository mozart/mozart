%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class ThreadDebugger from Emacs
   feat
      Thr

   meth init(thr:T)
      self.Thr = T
   end
   
   meth step
      {Thread.resume self.Thr}
   end

   meth cont
      {Dbg.stepmode self.Thr off}
      {Thread.resume self.Thr}
   end
   
   meth stack
      {Ozcar stack(self.Thr)}
   end
end
