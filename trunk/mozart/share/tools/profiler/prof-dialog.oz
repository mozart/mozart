%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class AboutDialog from TkTools.dialog
      meth init(master:Master)
         TkTools.dialog,tkInit(master:  Master
			       root:    pointer
                               title:   'About'
                               buttons: ['Ok' # tkClose]
                               focus:   1
                               pack:    false
                               default: 1)
	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  font:   HelpTitleFont
				  text:   'Oz ' # IconName)}

	 V = {New Tk.label tkInit(parent: self
				  text:   'Last updated on ' # Version)}

	 A = {New Tk.label
	      tkInit(parent: self
		     text:  NameOfRalf #' & '# NameOfBenni #'\n'# EmailOfBoth)}
      in
         {Tk.send pack(T V A side:top expand:1)}
         AboutDialog,tkPack
      end

   end
   
   local

      fun {FindPos TLs FT N}
	 T#L | TLr = TLs
      in
	 case T == FT then N else {FindPos TLr FT N+1} end
      end
      
      ScaleWidth = 100
      Prefix     = 'Update interval set to '
      Suffix     = ' seconds'
      Off        = 'Automatic update turned off'

   in
      
      class UpdateDialog
	 from TkTools.dialog
	 meth init(master:Master)
	    TkTools.dialog,
	    tkInit(master: Master
		   root:   pointer
		   title:  'Update'
		   buttons:
		      ['Ok' #
		       tkClose(proc {$}
				  NewValue = {TimeScale get($)}
			       in
				  {Cset update NewValue}
				  case NewValue == 0 then
				     {ForAll [stop
					      doStatus(Off)
					     ] Profiler}
				  else
				     S = NewValue div 1000
				  in
				     {ForAll [setRepDelay(NewValue)
					      doStatus(Prefix # S # Suffix)
					     ] Profiler}
				     thread
					{Profiler go}
				     end
				  end
			       end)
		       'Cancel' # tkClose]
		   pack:    false
		   focus:   1
		   default: 1)
	    
	    TimeScale = {New DiscreteScale
			 init(parent:  self
			      width:   ScaleWidth
			      values:  UpdateTimes
			      initpos: {FindPos UpdateTimes {Cget update} 1})}
	 in
	    {Tk.send pack(TimeScale side:top expand:true)}
	    UpdateDialog,tkPack
	 end
	 
      end
   end

in
   
   class Dialog

      meth init
	 skip
      end
   
      meth about
         {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
      end

      meth configureUpdate
	 {Wait {New UpdateDialog init(master:self.toplevel)}.tkClosed}
      end
   end
   
end
