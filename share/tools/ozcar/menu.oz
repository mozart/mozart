%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local
   
   TkVerbose = {New Tk.variable tkInit(ConfigVerbose)}
   
in
   
   class Menu
      meth init
	 self.menuBar = 
	 {TkTools.menubar self.toplevel self.toplevel
	  [menubutton(text: 'Ozcar'
		      menu:
			 [command(label:   'About...'
				  action:  self # about
				  feature: about)]
		      feature: ozcar)
	   menubutton(text: 'File'
		      menu:
			 [command(label:   'Quit'
				  action:  self # exit
				  key:     ctrl(c)
				  feature: quit)]
		      feature: file)
	   menubutton(text: 'Thread'
		      menu:
			 [separator]
		      feature: 'thread')
	   menubutton(text: 'Options'
		      menu:
			 [checkbutton(label:    'Messages in Emulator buffer'
				      variable: TkVerbose
				      action:   Config # toggleVerbose
				      feature:  verbose)]
		      feature: options)]
	  [menubutton(text: 'Help'
		      menu:
			 [separator]
		      feature: help)
	  ]}
      end
   end
end
