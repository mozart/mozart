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
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
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
				  text:   ('Last updated on ' # Version #
					   '\n' #
					   'Current platform is ' # Platform))}

	 A = {New Tk.label tkInit(parent: self
				  bg:     DefaultBackground
				  text:   NameOfBenni # '\n' # EmailOfBenni)}
      in
	 {Tk.send pack(T V A side:top expand:true)}
	 AboutDialog,tkPack
      end
   end

   SL = '/' | '-' | '\\' | '|' | SL

   class EvalDialog from TkTools.dialog
      prop
	 final
	 locking
      feat
	 Expr
	 Result
      attr
	 CurComp    : unit
	 CurCompUI  : unit
	 CurEnv     : unit
	 EvalThread : unit

	 SlashList  : SL

      meth init(master:Master)
	 proc {EvalInit}
	    C      = {New Compiler.compilerClass init}
	    CUI    = {New Compiler.quietInterface init(C)}
	    AuxEnv = {Ozcar PrivateSend(getEnv(unit $))}
	 in
	    CurComp   <- C
	    CurCompUI <- CUI
	    CurEnv    <- {Record.adjoinList env {Append AuxEnv.'G' AuxEnv.'Y'}}

	    {C enqueue(putEnv({{Compiler.getOPICompiler} enqueue(getEnv($))}))}
	    {C enqueue(mergeEnv(@CurEnv))}
	    {Wait {C enqueue(ping($))}}
	    {CUI reset}
	 end

	 SpinnerLock = {NewLock}

	 proc {Spinner W X}
	    case {IsFree X} then
	       S|Sr = @SlashList
	    in
	       {Delay 70}
	       {W tk(conf text:S)}
	       SlashList <- Sr
	       {Spinner W X}
	    else skip end
	 end

	 proc {Doit V}
	    case @EvalThread == unit then Sync in
	       try Self R in
		  EvalThread <- {Thread.this}
		  {OzcarMessage 'Doit: ' # V}
		  thread
		     lock SpinnerLock then
			{Delay 150} %% short calculations don't need a spinner
			case {IsFree Sync} then
			   S|Sr = @SlashList
			in
			   {self.Result tk(conf fg:DefaultForeground text:S)}
			   SlashList <- Sr
			   {Thread.setThisPriority high}
			   {Spinner self.Result Sync}
			else skip end
		     end
		  end
		  {EvalInit}
		  Self = {CondSelect @CurEnv 'self' unit}
		  {@CurComp enqueue(setSwitch(expression true))}
		  {@CurComp enqueue(setSwitch(threadedqueries false))}
		  {@CurComp enqueue(setSwitch(debuginfovarnames true))}
		  {@CurComp enqueue(setSwitch(debuginfocontrol true))}
		  case Self of unit then
		     {@CurComp
		      enqueue(feedVirtualString(V return(result: ?R)))}
		  else
		     %% declare `result` in
		     %% local
		     %%    class Class1
		     %%       meth eval($)
		     %%          <V>
		     %%       end
		     %%    end
		     %% in
		     %%    {`send` eval(`result`) Class1 `self`}
		     %% end
		     {@CurComp enqueue(mergeEnv(env('`self`': Self)))}
		     {@CurComp
		      enqueue(feedVirtualString('{`send` eval($) ' #
						'class meth eval($)\n' #
						V # '\nend end `self`}'
						return(result: ?R)))}
		  end
		  {Wait {@CurComp enqueue(ping($))}}
		  Sync = unit
		  lock SpinnerLock then skip end %% wait for spinner to finish
		  case {@CurCompUI hasErrors($)} then ResultText in
		     case RunningWithOPI then
			ResultText = 'Compile Error (see *Oz Compiler* buffer)'
			{System.printInfo [6 17]#{@CurCompUI getVS($)}#[5]}
		     else
			ResultText = 'Compile Error'
			{System.printInfo {@CurCompUI getVS($)}}
		     end
		     {self.Result tk(conf fg:BlockedThreadColor
				     text:ResultText)}
		  else
		     {self.Result tk(conf fg:DefaultForeground
				     text:{V2VS R})}
		  end
		  EvalThread <- unit
	       catch E=kernel(terminate) then
		  case @CurComp \= unit then
		     {@CurComp clearQueue()}
		     {@CurComp interrupt()}
		  else skip
		  end
		  raise E end
	       finally
		  Sync = unit
	       end
	    else
	       skip
	    end
	 end

	 proc {Eval}
	    {Kill}
	    case {self.Expr tkReturn(get $)} of "" then
	       {self.Result tk(conf text:'Did you ask something?')}
	    elseof V then
	       {Doit V}
	    end
	 end

	 proc {Exec}
	    {Kill}
	    case {self.Expr tkReturn(get $)} of "" then
	       {self.Result tk(conf text:'Did you say something?')}
	    elseof V then
	       {Doit V # '\nunit'}
	    end
	 end

	 proc {Reset}
	    {Kill}
	    {self.Result tk(conf text:'')}
	 end

	 proc {Kill}
	    lock
	       case @EvalThread == unit then skip else
		  {Thread.terminate @EvalThread}
		  lock SpinnerLock then skip end %% wait for spinner to finish
		  EvalThread <- unit
	       end
	    end
	 end

	 proc {Close}
	    {Kill}
	    {self tkClose}
	 end

	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'Query'
			       buttons: ['Eval'  # Eval
					 'Exec'  # Exec
					 'Reset' # Reset
					 'Done'  # Close]
			       pack:    false)
	 Frame = {New TkTools.textframe tkInit(parent: self
					       text: 'Eval Expression' #
						     ' / Exec Statement')}

	 ExprLabel = {New Tk.label tkInit(parent: Frame.inner
					  anchor: w
					  text:   'Query:')}
	 ExprEntry = {New Tk.entry tkInit(parent:     Frame.inner
					  font:       DefaultFont
					  background: DefaultBackground
					  width:      40)}
	 ResultLabel = {New Tk.label tkInit(parent: Frame.inner
					    anchor: w
					    text:   'Result:')}
	 ResultEntry = {New Tk.label tkInit(parent:     Frame.inner
					    relief:     sunken
					    anchor:     w
					    font:       DefaultFont
					    background: DefaultBackground
					    width:      40)}

      in
	 self.Expr = ExprEntry
	 self.Result = ResultEntry

	 %% how to close the dialog
	 {self.toplevel tkBind(event: '<Escape>'
			       action: Close)}
	 %% resetting (kill eval/exec thread)
	 {ExprEntry tkBind(event: '<Meta-t>'
			   action: Reset)}
	 %% eval expression
	 {ExprEntry tkBind(event: '<Return>'
			   action: Eval)}
	 %% exec statement
	 {ExprEntry tkBind(event: '<Meta-Return>'
			   action: Exec)}

	 {Tk.batch [grid(ExprLabel    row:0 column:0 padx:1 pady:1)
		    grid(ExprEntry    row:0 column:1 padx:3 pady:1)
		    grid(ResultLabel  row:1 column:0 padx:1 pady:1)
		    grid(ResultEntry  row:1 column:1 padx:3 pady:1)
		    grid(Frame        row:0 column:0 padx:1 pady:0)
		    focus(ExprEntry)]}
	 EvalDialog,tkPack
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
	       {Emacs  setVerbose(Verbose)}
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
	    case Cur == Top then
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
					 'Abort' # tkClose]
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

      meth eval
	 {Wait {New EvalDialog init(master:self.toplevel)}.tkClosed}
      end

      meth settings
	 {Wait {New SettingsDialog init(master:self.toplevel)}.tkClosed}
      end

   end

end
