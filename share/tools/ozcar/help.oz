%%% oz-mode is stupid, so we use -*-text-mode-*-
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   MessageWidth = 380
   OkButtonText = 'Aha'
   NoTopic      = 'No Help Available'
   NoHelp       = 'Feel free to ask the author.\n' #
                  'Send a mail to ' # EmailOfBenni
   
   HelpDict     = {Dictionary.new}
   
   {ForAll
    [
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

      ThreadTreeHelp # ('The Thread Tree' #
	('Threads can be added to the tree by feeding code from Emacs. ' #
	'When added this way, new threads get selected automatically.\n' # 
	'\nYou can select another thread by clicking on it.\n' #
	'\nThe different colors correspond to the ' #
	'following thread states:\n' #
	'green -> runnable, red -> blocked, black -> terminated\n' #
	'\nA thread can be removed from the tree by ' #
	'pressing f or clicking on the forget button.'))

      StackHelp # ('The Stack' #
	('You can navigate through the stack either by clicking on a ' #
	'specific line or by using the Up and Down (cursor) keys.\n' #
	'\nYou can browse an argument by clicking ' #
	'on its type information.'))

      EnvHelp # ('The Environment' #
	('You can browse a variable by clicking ' #
	'on its type information.'))

      StepButtonText   # (NoTopic # NoHelp)
      NextButtonText   # (NoTopic # NoHelp)
      ContButtonText   # (NoTopic # NoHelp)
      ForgetButtonText # (NoTopic # NoHelp)
      TermButtonText   # (NoTopic # NoHelp)
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
			       title:   self.topic
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
