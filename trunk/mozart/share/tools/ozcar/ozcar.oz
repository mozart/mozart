%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class OzcarClass
      from
	 ThreadManager
	 SourceManager
	 Gui

      prop
	 final

      meth init
	 ThreadManager,init
	 Gui,init
      end

      meth on
	 {Tk.batch [update(idletasks)
		    wm(deiconify self.toplevel)]}
	 case {NewCompiler} then
	    case {CgetTk emacsThreads} then
	       {Compile '\\switch +debuginfo'}
	    else
	       {Compile '\\switch +debuginfovarnames +debuginfocontrol'}
	    end
	 else
	    {Compile '\\switch +debuginfo'}
	 end
	 {Dbg.on}
	 case @currentThread == unit then
	    Gui,status(TitleName # ' initialized')
	 else skip end
      end

      meth off
	 {Dbg.off}
	 {Tk.send wm(withdraw self.toplevel)}
	 {Compile '\\switch -debuginfo'}
	 {Emacs removeBar}
      end
   end

in

   PrivateSend = {NewName}

   Ozcar =
   {New class

	   prop
	      final

	   attr
	      MyOzcar : unit

	   meth reInit
	      case @MyOzcar == unit then skip else
		 {@MyOzcar destroy}
	      end
	      MyOzcar <- {New OzcarClass init}
	   end

	   meth on
	      {@MyOzcar on}
	   end

	   meth off
	      {@MyOzcar off}
	   end

	   meth bpAt(_ _ _)=M
	      {@MyOzcar M}
	   end

	   meth lastClickedValue(?V)
	      {@MyOzcar lastClickedValue(V)}
	   end

	   meth !PrivateSend(M)
	      {@MyOzcar M}
	   end

	end reInit}
end
