%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class AboutDialog from TkTools.dialog
      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'About'
			       buttons: ['Ok'#tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  font:   HelpTitleFont
				  text:   'Oz Debugger')}

	 V = {New Tk.label tkInit(parent: self
				  text:   ('Last updated on ' # Version #
					   '\n' #
					   'Current platform is ' # Platform))}

	 A = {New Tk.label tkInit(parent: self
				  text:   NameOfBenni # '\n' # EmailOfBenni)}
      in
	 {Tk.send pack(T V A side:top expand:1)}
	 AboutDialog,tkPack
      end

   end

in

   class Dialog

      meth about
	 {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
      end
   end

end
