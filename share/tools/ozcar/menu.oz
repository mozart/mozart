%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

class Menu
   meth init
      self.menuBar = 
      {TkTools.menubar self.toplevel self.toplevel
       [menubutton(text: 'Ozcar'
		   menu:
		      [command(label:   'About'
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
		      [separator]
		   feature: options)]
       [menubutton(text: 'Help'
		   menu:
		      [separator]
		   feature: help)
       ]}
   end
end
