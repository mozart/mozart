functor
export 'class' : InteractiveManager
import
   Application
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
define
   class InteractiveManager

      meth init(OzpmInfo)
	 {Wait OzpmInfo}
	 Look={QTk.newLook}
	 %%
	 MenuDesc=
	 lr(glue:nwe
	    menubutton(
	       text:'File'
	       glue:w
	       menu:menu(
		       command(text:'Install package...')
		       command(text:'Remove package...')
		       separator
		       command(text:'Exit' action:toplevel#close)))
	    menubutton(
	       text:'Help'
	       glue:e
	       menu:menu(command(text:'Help...')
			 separator
			 command(text:"About..."
				 action:proc{$}
					   {{QTk.build
					     td(title:'About this application...'
						label(text:'Mozart Package Installer\n'#
						      'By Denys Duchier and Donatien Grolaux\n'#
						      '(c) 2000\n'
						      glue:nw)
						button(text:'Close'
						       glue:s
						       action:toplevel#close))}
					    show(modal:true wait:true)}
					end))))
	 %%
	 ToolbarDesc=lr(glue:nwe relief:sunken borderwidth:1
			tbbutton(text:'Install' glue:w)
			tbbutton(text:'Remove' glue:w)
			tdline(glue:nsw)
			tbbutton(text:'Help' glue:w)
			tbbutton(text:'Quit' glue:w))
	 %%
	 MainWindowDesc=
	 lrrubberframe(glue:nswe
		       td(label(text:'Installed package' glue:nw)
			  listbox(glue:nswe tdscrollbar:true lrscrollbar:true))
		       td(label(text:'Remaining packages' glue:nw)
			  listbox(glue:nswe tdscrollbar:true lrscrollbar:true)))
	 %%
	 StatusBar
	 StatusBarDesc=
	 placeholder(glue:swe relief:sunken borderwidth:1
		     handle:StatusBar
		     label(glue:nswe text:'Mozart Package installer'))
	 %%
	 Desc=td(look:Look
		 title:'Mozart Package Installer'
		 action:toplevel#close
		 MenuDesc
		 ToolbarDesc
		 MainWindowDesc
		 StatusBarDesc)
      in
	 {{QTk.build Desc} show(wait:true)}
	 {Application.exit 0}
      end
   end
end
