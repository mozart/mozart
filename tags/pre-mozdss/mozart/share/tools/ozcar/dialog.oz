%%%
%%% Author:
%%%   Benjamin Lorenz <lorenz@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Benjamin Lorenz, 1997
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.

local

   class AboutDialog from TkTools.dialog
      prop
	 final
      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'About'
			       bg:      DefaultBackground
			       buttons: ['Ok'#tkClose]
			       pack:    false
			       default: 1)
	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  bg:     DefaultBackground
				  font:   HelpTitleFont
				  text:   'Oz Debugger')}

	 V = {New Tk.label tkInit(parent: self
				  bg:     DefaultBackground
				  text:
				     ('Platform: ' # Platform # ', ' #
				      {Tk.return winfo(server '.')}))}

	 A = {New Tk.label tkInit(parent: self
				  bg:     DefaultBackground
				  text:   NameOfBenni # '\n' # EmailOfBenni)}
      in
	 {Tk.send pack(T V A side:top expand:true)}
	 AboutDialog,tkPack
      end
   end

   class ServerDialog from TkTools.dialog
      prop
	 final
      meth init(master:Master)
	 Ticket = {StartServer}

	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'Oz Debugger Server'
			       bg:      DefaultBackground
			       buttons: ['Ok'#tkClose]
			       pack:    false
			       default: 1)

	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  bg:     DefaultBackground
				  font:   HelpTitleFont
				  text:   'Server Started')}

	 V = {New Tk.label tkInit(parent: self
				  bg:     DefaultBackground
				  text:   ('Ready to accept remote ' #
					   'debug sessions with the ' #
					   'following ticket:\n'))}

	 E = {New Tk.entry tkInit(parent: self
				  font:   DefaultFont
				  width:  {VirtualString.length Ticket})}
      in
	 {E tk(insert 'end' Ticket)}
	 %{E tk(conf state:disabled)}
	 {Tk.send pack(T V E side:top expand:true)}
	 AboutDialog,tkPack
      end
   end

   class EvalDialog from BaseEvalDialog.dialog
      prop final
      meth init(master: Master)
	 proc {AcquireEnv ?Env ?Self}
	    AuxEnv = {Ozcar PrivateSend(getEnv(unit $))}
	 in
	    Env = {Record.adjoinList
		   case {Cget emacsInterface} of false then OPIEnv.full
		   elseof I then
		      {{I getNarrator($)} enqueue(getEnv($))}
		   end
		   {Filter {Append AuxEnv.'G' AuxEnv.'Y'}
		    fun {$ V#W}
		       case V of 'self' then Self = W false
		       else true
		       end
		    end}}
	    if {IsFree Self} then
	       Self = unit
	    end
	 end
      in
	 BaseEvalDialog.dialog, tkInit(master:   Master
				       root:     pointer
				       acquireEnvProc: AcquireEnv)
	 {BaseEvalDialog.dialog, getCompiler($)
	  enqueue([setSwitch(controlflowinfo true)
		   setSwitch(staticvarnames true)])}

      end
   end

   class SettingsDialog from TkTools.dialog
      prop
	 final
      attr
	 ReturnSync : _
      meth init(master:Master)
	 TkVerbose            = {New Tk.variable tkInit({Cget verbose})}
	 TkStepDotBuiltin     = {New Tk.variable tkInit({Cget stepDotBuiltin})}
	 TkStepNewNameBuiltin = {New Tk.variable
				 tkInit({Cget stepNewNameBuiltin})}
	 TkEnvSystemVariables = {New Tk.variable
				  tkInit({Cget envSystemVariables})}

	 proc {Apply}
	    {DoFocusToTop}

	    local
	       Verbose = {TkVerbose tkReturnInt($)} > 0
	    in
	       {Config set(verbose Verbose)}
	    end

	    {Config set(stepDotBuiltin {TkStepDotBuiltin tkReturnInt($)} > 0)}
	    {Config set(stepNewNameBuiltin
			{TkStepNewNameBuiltin tkReturnInt($)} > 0)}
	    {Config set(envSystemVariables
			{TkEnvSystemVariables tkReturnInt($)} > 0)}

	    {Config set(printWidth         {WidthEntry tkGet($)})}
	    {Config set(printDepth         {DepthEntry tkGet($)})}
	    {Config set(timeoutToSwitch    {TSwiEntry  tkGet($)})}
	    {Config set(timeoutToUpdateEnv {TEnvEntry  tkGet($)})}

	    {Ozcar PrivateSend(rebuildCurrentStack)}
	 end

	 proc {ApplyAndExit}
	    {Apply}
	    {self tkClose}
	 end

	 proc {CheckApplyAndExit}
	    Top = {Tk.getTclName self.toplevel}
	    Cur = {Tk.return focus(displayof:self.toplevel)}
	 in
	    if Cur == Top then
	       {ApplyAndExit}
	    else
	       @ReturnSync = unit
	    end
	 end

	 proc {DoFocusToTop}
	    {Tk.send focus(self.toplevel)}
	 end

	 proc {FocusToTop}
	    {Wait @ReturnSync}
	    {DoFocusToTop}
	 end

	 proc {FocusFromTop}
	    ReturnSync <- _
	 end

	 TkTools.dialog,tkInit(master:  Master
			       root:
				  %({Tk.returnInt winfo(pointerx '.')}-169 #
				  % {Tk.returnInt winfo(pointery '.')}-127)
			       master # 143 # 77
			       title:   'Preferences'
			       buttons: ['Ok'    # ApplyAndExit
					 'Apply' # Apply
					 'Cancel' # tkClose]
			       pack:    false)
	 Title = {New Tk.label tkInit(parent: self
				      fg:     SelectedBackground
				      font:   HelpTitleFont
				      text:   'Miscellaneous Settings')}
%---------------
	 TimeoutFrame = {New TkTools.textframe
			    tkInit(parent:  self
				   'class': 'NumberEntry'
				   text:    'Idle Time To Perform Action')}
	 DummyFrame2 = {New Tk.frame tkInit(parent: TimeoutFrame.inner)}
	 TSwiLabel   = {New Tk.label tkInit(parent: DummyFrame2
					    text:   'Thread Switch:')}
	 TSwiEntry   = {New TkTools.numberentry
			tkInit(parent:       DummyFrame2
			       returnaction: FocusToTop
			       min:          0
			       max:          2000
			       val:          {Cget timeoutToSwitch}
			       width:        5)}
	 TSwiLabel2  = {New Tk.label tkInit(parent: DummyFrame2
					    text:   'ms')}
	 TEnvLabel   = {New Tk.label tkInit(parent: DummyFrame2
					    text:   'Env Update:')}
	 TEnvEntry   = {New TkTools.numberentry
			tkInit(parent:       DummyFrame2
			       returnaction: FocusToTop
			       min:          0
			       max:          5000
			       val:          {Cget timeoutToUpdateEnv}
			       width:        5)}
	 TEnvLabel2  = {New Tk.label tkInit(parent: DummyFrame2
					    text:   'ms')}
%---------------
	 WidthDepthFrame = {New TkTools.textframe
			    tkInit(parent:  self
				   'class': 'NumberEntry'
				   text:    'Value Printing')}
	 DummyFrame = {New Tk.frame tkInit(parent: WidthDepthFrame.inner)}
	 WidthLabel = {New Tk.label tkInit(parent: DummyFrame
					   text:   'Width:')}
	 WidthEntry = {New TkTools.numberentry
		       tkInit(parent:       DummyFrame
			      returnaction: FocusToTop
			      min:          1
			      max:          20
			      val:          {Cget printWidth}
			      width:        3)}
	 DepthLabel = {New Tk.label tkInit(parent: DummyFrame
					   text:   'Depth:')}
	 DepthEntry = {New TkTools.numberentry
		       tkInit(parent:       DummyFrame
			      returnaction: FocusToTop
			      min:          0
			      max:          5
			      val:          {Cget printDepth}
			      width:        3)}
%---------------
	 StepFrame = {New TkTools.textframe
			tkInit(parent:  self
			       text:    'Step on Builtin')}
	 StepDot   = {New Tk.checkbutton
		      tkInit(parent:   StepFrame.inner
			     text:     '`.\' ' %% make it somewhat larger
					       %% for easier clicking... ;)
			     variable: TkStepDotBuiltin)}
	 StepNewName = {New Tk.checkbutton
			tkInit(parent:   StepFrame.inner
			       text:     '`NewName\''
			       variable: TkStepNewNameBuiltin)}
	 FilterFrame = {New TkTools.textframe
			tkInit(parent:  self
			       text:    'Filtering')}
	 SystemVButton = {New Tk.checkbutton
			  tkInit(parent:   FilterFrame.inner
				 text:     'Show System Variables'
				 variable: TkEnvSystemVariables)}
	 OtherFrame = {New TkTools.textframe
		       tkInit(parent:  self
			      text:    'Esoteric Options')}
	 DDButton = {New Tk.checkbutton
		     tkInit(parent:   OtherFrame.inner
			    text:     'Debug Debugger'
			    variable: TkVerbose)}

      in

	 {self.toplevel tkBind(event:'<Tab>'       action:FocusFromTop)}
	 {self.toplevel tkBind(event:'<Shift-Tab>' action:FocusFromTop)}
	 {self.toplevel tkBind(event:'<Return>'    action:CheckApplyAndExit)}
	 {self.toplevel tkBind(event:'<Escape>'    action:self#tkClose)}

	 {StepDot       tkBind(event:'<Return>'    action:FocusToTop)}
	 {StepNewName   tkBind(event:'<Return>'    action:FocusToTop)}
	 {SystemVButton tkBind(event:'<Return>'    action:FocusToTop)}
	 {DDButton      tkBind(event:'<Return>'    action:FocusToTop)}

	 {Tk.batch [grid(Title row:0 column:1 columnspan:2 sticky:we pady:2)
		    grid(TimeoutFrame    row:1 column:1 columnspan:2
			 sticky:nswe padx:2 pady:2)
		    grid(WidthDepthFrame row:2 column:1
			 sticky:nswe padx:2 pady:2)
		    grid(StepFrame       row:2 column:2
			 sticky:nswe padx:2 pady:2)
		    grid(FilterFrame     row:3 column:1
			 sticky:nswe padx:2 pady:2)
		    grid(OtherFrame      row:3 column:2
			 sticky:nswe padx:2 pady:2)
		    pack(DummyFrame2 side:left anchor:w padx:1 pady:2)
		    pack(DummyFrame  side:left anchor:w padx:1 pady:2)
		    grid(TSwiLabel       row:1 column:1 sticky:w)
		    grid(TSwiEntry       row:1 column:2 sticky:w)
		    grid(TSwiLabel2      row:1 column:3 sticky:w)
		    grid(TEnvLabel       row:2 column:1 sticky:w)
		    grid(TEnvEntry       row:2 column:2 sticky:w)
		    grid(TEnvLabel2      row:2 column:3 sticky:w)
		    grid(WidthLabel      row:1 column:1 sticky:w)
		    grid(WidthEntry      row:1 column:2 sticky:w)
		    grid(DepthLabel      row:2 column:1 sticky:w)
		    grid(DepthEntry      row:2 column:2 sticky:w)
		    pack(StepDot StepNewName side:top anchor:w pady:1)
		    pack(SystemVButton anchor:w)
		    pack(DDButton
			 {New Tk.label tkInit(parent:OtherFrame.inner
					      text:'      ')}
			 anchor:w side:left)
		   ]}
	 SettingsDialog,tkPack
      end
   end

in

   class Dialog

      meth about
	 {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
      end

      meth startServer
	 {Wait {New ServerDialog init(master:self.toplevel)}.tkClosed}
      end

      meth eval
	 {Wait {New EvalDialog init(master:self.toplevel)}.tkClosed}
      end

      meth settings
	 {Wait {New SettingsDialog init(master:self.toplevel)}.tkClosed}
      end

   end

end
