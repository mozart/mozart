%%% oz-mode is stupid, so we use -*-text-mode-*-
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   MessageWidth = 380
   HelpTitle    = 'Ozcar Help'
   OkButtonText = 'Aha'
   NoTopic      = 'No Help Available'
   NoHelp       = 'Feel free to ask the author.\n' #
                  'Send a mail to ' # EmailOfBenni
   
   HelpDict     = {Dictionary.new}
   
   {ForAll
    [
      nil # ('The Ozcar Help System' #
	('For most of the widgets in the Ozcar GUI you can ' #
	'get some help. To display it, you have to click ' #
	'with the right mouse button on the widget.\n' #
	'\nFurthermore, there are some interesting help topics available ' #
	'in the Help menu.'))

      StatusHelp # ('The Status Line' #
	('Important events like reaching a breakpoint or raising ' #
	'an exception are reported here.'))

      IgnoreFeeds # ('Ignore Emacs Queries' #
	('When this checkbutton is activated then you can feed and ' #
	'execute some code from Emacs without Ozcar taking over control.'))

      BreakpointStaticHelp # ('Static Breakpoints' #
	('You can set a Static Breakpoint by inserting ' #
	'{Debug.breakpoint} into ' #
	'your code, just before the line where you want ' #
	'the thread to stop.'))

      BreakpointDynamicHelp # ('Dynamic Breakpoints' #
	('You can set / delete a Dynamic Breakpoint at the current line of ' #
	'an Emacs buffer by entering C-c C-d C-b / C-c C-d C-d.\n' #
	'Alternatively, click with the left / right mouse button ' #
	'on the line where you wish to set / delete a breakpoint, while ' #
	'holding down the Shift and Meta key.'))

      TreeTitle # ('The Thread Tree' #
	('Threads can be added to the tree by feeding code from Emacs. ' #
	'When added this way, new threads get selected automatically.\n' # 
	'\nYou can select another thread by clicking on it.\n' #
	'\nThe different colors correspond to the ' #
	'following thread states:\n' #
	'green -> runnable, red -> blocked, black -> terminated\n' #
	'\nA thread can be removed from the tree by ' #
	'pressing f or clicking on the forget button.'))

      StackTitle # ('The Stack' #
	('You can navigate through the stack either by clicking on a ' #
	'specific line or by using the Up and Down (cursor) keys.\n' #
	'\nYou can browse an argument by clicking ' #
	'on its type information.'))

      LocalEnvTitle # ('The Local Environment' #
	('You can browse a local variable by clicking ' #
	'on its type information.'))

      GlobalEnvTitle # ('The Global Environment' #
	('You can browse a global variable by clicking ' #
	'on its type information.'))

      StepButtonText   # ('Step' #
	('Let the current thread continue to run until ' #
	'it reaches the next procedure call.'))

      NextButtonText   # ('Next' #
	('Let the current thread continue to run until ' #
	'it reaches the next procedure call in the current stack frame.'))

      ContButtonText   # ('Continue' #
	('Let the current thread continue to run until ' #
	'it terminates, blocks, reaches a breakpoint ' #
	'or raises an unhandled exception.'))

      ForgetButtonText # ('Forget' #
	('Do not trace current thread anymore, let it ' #
	'continue to run, and remove it ' #
	'from the thread tree. It will come back when it reaches ' #
	'a breakpoint or raises an unhandled exception.'))

      TermButtonText   # ('Terminate' #
	('Terminate current thread and remove it from the thread tree.'))

    ]
    proc {$ S}
       {Dput HelpDict S.1 S.2}
    end}
   
   class HelpDialog from TkTools.dialog
      feat
	 topic help
      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   HelpTitle % self.topic
			       buttons: [OkButtonText # tkClose]
			       focus:   1
			       bg:      DefaultBackground
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
	 {Tk.send pack(Title Help side:top expand:1 pady:3 fill:x)}
	 HelpDialog,tkPack
      end
   end

   class OzcarHelp from HelpDialog
      meth init(master:Master topic:Topic)
	 self.topic # self.help =
	 try {Dget HelpDict Topic} catch system(...) then NoTopic # NoHelp end
	 HelpDialog,init(master:Master)
      end
   end

in
   
   class Help
      
      meth init
	 skip
      end

      meth help(Topic)
	 {Wait {New OzcarHelp init(master:self.toplevel topic:Topic)}.tkClosed}
      end
      
   end
end
