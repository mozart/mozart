%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
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

local

   class AboutDialog from TkTools.dialog
      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'About'
			       bg:      DefaultBackground
			       buttons: ['Ok' # tkClose]
			       default: 1
			       pack:    false)
	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  bg:     DefaultBackground
				  font:   HelpTitleFont
				  text:   'Oz ' # IconName)}

	 A = {New Tk.label
	      tkInit(parent: self
		     bg:     DefaultBackground
		     text:   (NameOfRalf #' & '#
			      NameOfBenni #'\n'# EmailOfBoth))}
      in
	 {Tk.send pack(T A side:top expand:1)}
	 AboutDialog,tkPack
      end

   end

   local

      fun {FindPos TLs FT N}
	 T#_ | TLr = TLs
      in
	 if T == FT then N else {FindPos TLr FT N+1} end
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
				  if NewValue == 0 then
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

	    TimeScale = {New TkTools.scale
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
