%%% oz-mode is stupid, so we use -*-text-mode-*-
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   MessageWidth = 400
   OkButtonText = 'Aha...'

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
	 '{Debug.breakpoint} into ' #
	 'your code, just before the line where you want ' #
	 'the thread to stop.'
	 HelpDialog,init(master:Master)
      end
   end
   
   class BreakpointDynamicHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'Dynamic Breakpoints'
	 self.help  =
	 'You can set / delete a Dynamic Breakpoint at the current line of ' #
	 'an Emacs buffer by entering C-c C-d C-b / C-c C-d C-d.\n' #
	 'Alternatively, click with the left / right mouse button ' #
	 'on the line where you wish to set / delete a breakpoint, while ' #
	 'holding down the Shift and Meta key.'
	 HelpDialog,init(master:Master)
      end
   end
   
   class ThreadTreeHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'The Thread Tree'
	 self.help  =
	 'Threads can be added to the tree by feeding code from Emacs. ' #
	 'When added this way, new threads get selected automatically.\n' # 
	 '\nYou can select another threads by clicking on it.\n' #
	 '\nThe different colors correspond to the ' #
	 'following thread states:\n' #
	 'green <-> runnable, red <-> blocked, black <-> terminated\n' #
	 '\nA thread can be removed from the tree by ' #
	 'pressing f or clicking on the forget button.' 
	 HelpDialog,init(master:Master)
      end
   end      

   class StackHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'The Stack'
	 self.help  =
	 'You can navigate through the stack either by clicking on a ' #
	 'specific line or by using the Up and Down (cursor) keys.\n' #
	 '\nYou can browse an argument by clicking ' #
	 'on its type information.'
	 HelpDialog,init(master:Master)
      end
   end      

   class EnvHelp from HelpDialog
      meth init(master:Master)
	 self.topic = 'The Environment'
	 self.help  =
	 'You can browse a variable by clicking ' #
	 'on its type information.'
	 HelpDialog,init(master:Master)
      end
   end      

in
   
   class Help
      
      meth init
	 skip
      end

      meth helpThreadTree
	 {Wait {New ThreadTreeHelp init(master:self.toplevel)}.tkClosed}
      end
      
      meth helpStack
	 {Wait {New StackHelp init(master:self.toplevel)}.tkClosed}
      end

      meth helpEnv
	 {Wait {New EnvHelp init(master:self.toplevel)}.tkClosed}
      end

      meth helpBreakpointStatic
         {Wait {New BreakpointStaticHelp init(master:self.toplevel)}.tkClosed}
      end

      meth helpBreakpointDynamic
	 {Wait {New BreakpointDynamicHelp init(master:self.toplevel)}.tkClosed}
      end
   end
   
end
