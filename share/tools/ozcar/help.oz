%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   MessageWidth = 400
   OkButtonText = 'Aaah...'

   class HelpDialog from TkTools.dialog
      feat
	 topic help
      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   self.topic
			       buttons: [OkButtonText # tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 Title = {New Tk.label tkInit(parent: self
				      bg:     DefaultBackground
				      text:   self.topic
				      font:   HelpTitleFont)}
	 Help = {New Tk.message
		 tkInit(parent: self
			font:   HelpFont
			bg:     DefaultBackground
			width:  MessageWidth
			text:   self.help)}
      in
	 {self tk(conf background:DefaultBackground)}
	 {Tk.send pack(Title Help side:top expand:1 pady:7 fill:x)}
	 HelpDialog,tkPack
      end
   end
   
   class BreakpointStaticHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'Static Breakpoints'
	 self.help  =
	 'You can set a Static Breakpoint by inserting ' #
	 'the application {Debug.breakpoint} into ' #
	 'your code, just before the line where you want ' #
	 'the thread to stop.' /* end */
	 HelpDialog,init(master:Master)
      end
   end
   
   class BreakpointDynamicHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'Dynamic Breakpoints'
	 self.help  =
	 'You can set a Dynamic Breakpoint at the current line of ' #
	 'an Emacs buffer by entering C-c C-d C-b. If you want to delete ' #
	 'the breakpoint, enter C-c C-d C-d.\n' #
	 'Alternatively, click with the left/right mouse button ' #
	 'on the line where you wish to set/delete a breakpoint, while ' #
	 'holding down the Shift and Meta key.'
	 HelpDialog,init(master:Master)
      end
   end      

in
   
   class Help

      meth init
	 skip
      end
   
      meth helpBreakpointStatic
         {Wait {New BreakpointStaticHelp init(master:self.toplevel)}.tkClosed}
      end

      meth helpBreakpointDynamic
	 {Wait {New BreakpointDynamicHelp init(master:self.toplevel)}.tkClosed}
      end
   end
   
end
