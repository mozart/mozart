functor
export 'class' : InteractiveManager
import
   Application
   QTk at 'http://www.info.ucl.ac.be/people/ned/qtk/QTk.ozf'
   System(show:Show)
   
define

   ArchiveManager
   OzpmInfo

   class ListDataView
      feat
	 infoPane
	 setTitle
	 archiveManager
	 mogulDB
	 handle
      meth init(INFO ST AM OZPM Desc)
	 self.infoPane=INFO
	 self.setTitle=ST
	 self.archiveManager=AM
	 self.mogulDB=OZPM
	 Desc=listbox(glue:nswe
		      bg:white
%		      tdscrollbar:true
		      handle:self.handle)
      end
   end

   class InfoView
      feat
	 setTitle
	 handle
      meth init(ST Desc)
	 self.setTitle=ST
	 Desc=listbox(glue:nswe
		      bg:white
%		      tdscrollbar:true
		      handle:self.handle)
      end
      meth display(Info)
	 skip
      end
   end
   
   class InteractiveManager

      feat dataPlace dataLabel infoPlace infoLabel

      attr data info
	 
      meth init(OI AM)
	 Look={QTk.newLook}
	 TitleLook={QTk.newLook}
	 InfoMain
	 DataMain
	 {TitleLook.set label(text:"Title" glue:nwes bg:darkblue fg:white relief:sunken borderwidth:2)}
	 %%
	 info<-{New InfoView init(proc{$ Title} {self.infoLabel set(text:Title)} end
				  InfoMain)}
	 data<-{New ListDataView init(@info
				      proc{$ Title} {self.dataLabel set(text:Title)} end
				      AM
				      OI
				      DataMain)}
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
	 tdrubberframe(glue:nswe
		       td(glue:nwe
			  lr(glue:we
			     label(look:TitleLook
				   handle:self.dataLabel)
			     tbbutton(text:"Detach"
				      action:proc{$} skip end
				      glue:e))
			  placeholder(handle:self.dataPlace glue:nswe
				      DataMain
				     ))
		       td(glue:nwe
			  lr(glue:we
			     label(look:TitleLook
				   handle:self.infoLabel)
			     tbbutton(text:"Detach"
				      action:proc{$} skip end
				      glue:e))
			  placeholder(handle:self.infoPlace glue:nswe
				      InfoMain
				     )
			 ))
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
%		 ToolbarDesc
		 MainWindowDesc
		 StatusBarDesc)
      in
	 OI=OzpmInfo
	 {Wait OzpmInfo}
	 AM=ArchiveManager
	 {{QTk.build Desc} show(wait:true)}
	 {Application.exit 0}
      end
   end
end
