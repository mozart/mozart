%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   MessageWidth = 380
   HelpTitle    = 'Ozcar Help'
   OkButtonText = 'Aha'
   NoTopic      = 'No Help Available'
   NoHelp       = ('Feel free to ask the author.\n' #
		   'Send a mail to ' # EmailOfBenni)

   HelpDict     = {Dictionary.new}

   {ForAll
    [
     nil #
     ('The Ozcar Help System' #
      ('For most of the widgets you can ' #
       'get some help.\nJust click ' #
       'with the right mouse button on the widget.'))

     StatusHelp #
     ('The Status Line' #
      ('Important events like reaching a breakpoint or raising ' #
       'an exception are reported here.'))

     IgnoreQueriesBitmap #
     ('Ignore Emacs Queries' #
      ('Activate this checkbutton if you want to feed some code ' #
       'from Emacs without Ozcar taking over control.'))

     IgnoreSubThreadsBitmap #
     ('Ignore Subthreads' #
      ('Activate this checkbutton if you don\'t want Ozcar ' #
       'to debug any subthreads of your initial query thread.'))

     BreakpointStaticHelp #
     ('Static Breakpoints' #
      ('You can set a Static Breakpoint by inserting ' #
       '{Debug.breakpoint} into ' #
       'your code, just before the line where you want ' #
       'the thread to stop.'))

     BreakpointDynamicHelp #
     ('Dynamic Breakpoints' #
      ('You can set / delete a Dynamic Breakpoint at the current line of ' #
       'an Emacs buffer by entering C-c C-d C-b / C-c C-d C-d.\n' #
       'Alternatively, click with the left / right mouse button ' #
       'on the line where you wish to set / delete a breakpoint, while ' #
       'holding down the Shift and Meta key.'))

     TreeTitle #
     ('The Thread Tree' #
      ('Threads are added to the tree when they run into a breakpoint ' #
       'or when they are created by Emacs queries and the ' #
       '\'Ignore Emacs Queries\'' # ' checkbutton is not activated. ' #
       '\n\nYou can select a thread by clicking on it or by walking ' #
       'to it with the `Left\' and `Right\' (cursor) keys.\n' #
       '\nThe different colors correspond to the ' #
       'following thread states:\n' #
       'green: runnable, red: blocked, grey: terminated\n' #
       '\nA thread can be removed from the tree either by ' #
       'killing it (action `terminate\') or by forgetting it ' #
       '(action `forget\').'))

     StackTitle #
     ('The Stack' #
      ('You can navigate through the stack either by clicking on a ' #
       'specific line or by using the `Up\' and `Down\' (cursor) keys.\n' #
       '\nYou can browse a variable by clicking ' #
       'on its type information.'))

     LocalEnvTitle #
     ('The Local Environment' #
      ('You can browse a local variable by clicking ' #
       'on its type information.'))

     GlobalEnvTitle #
     ('The Global Environment' #
      ('You can browse a global variable by clicking ' #
       'on its type information.'))

     StepButtonBitmap #
     ('Step into' #
      ('Let the current thread continue to run until ' #
       'it reaches the next steppoint.'))

     NextButtonBitmap #
     ('Step Over' #
      ('Let the current thread continue to run until ' #
       'it reaches the next steppoint in the current stack frame.'))

     UnleashButtonBitmap #
     ('Unleash' #
      ('Let the current thread continue to run until ' #
       'the selected stack frame is discarded or the thread blocks, ' #
       'reaches a breakpoint or raises an unhandled exception.'))

     StopButtonBitmap #
     ('Stop' #
      ('Stop the current thread as soon as it reaches the next ' #
       'steppoint. Note that blocked threads need to be stopped ' #
       'in order to get updated stack and environment windows.'))

     ForgetButtonBitmap #
     ('Forget' #
      ('Do not trace the current thread anymore, let it ' #
       'continue to run, and remove it ' #
       'from the thread tree. It will come back when it reaches ' #
       'a breakpoint or raises an unhandled exception.'))

     TermButtonBitmap #
     ('Terminate' #
      ('Terminate current thread and remove it from the thread tree.'))

    ]
    proc {$ S}
       {Dictionary.put HelpDict S.1 S.2}
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
	 {Dictionary.condGet HelpDict Topic NoTopic # NoHelp}
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
