%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

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
			       %focus:   1
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
      feat
	 Expr
	 Result
      attr
	 CurComp    : unit
	 CurEnv     : unit
	 EvalThread : unit

	 SlashList  : SL

      meth init(master:Master)
	 fun {EvalInit}
	    C      = {New Compiler.interface.quiet init}
	    AuxEnv = {Ozcar PrivateSend(getEnv(unit $))}
	 in
	    CurComp <- C
	    CurEnv  <- {Record.adjoinList env {Append AuxEnv.'G' AuxEnv.'Y'}}

	    {C putEnv({{Compiler.getOPICompiler} getEnv($)})}
	    {C mergeEnv(@CurEnv)}
	    {C reset}

	    {self.Result tk(conf
			    fg:DefaultForeground
			    text:'')}
	    C
	 end

	 proc {Spinner W X}
	    case {IsFree X} then
	       S|Sr = @SlashList
	    in
	       {Delay 80}
	       {W tk(conf text:S)}
	       SlashList <- Sr
	       {Spinner W X}
	    else skip end
	 end

	 proc {Doit V}
	    case @EvalThread == unit then
	       EvalThread <- {Thread.this}
	       C           = {EvalInit}
	       Self        = {CondSelect @CurEnv 'self' unit}
	    in
	       {OzcarMessage 'Doit: ' # V}
	       case Self of unit then
		  {C feedVirtualString('declare fun {`result` _}\n' # V #
				       '\nend')}
	       else
		  {C feedVirtualString('\\switch +selfallowedanywhere\n' #
				       'declare fun {`result` Self}\n' #
				       '{`ooSetSelf` Self}' # V # '\nend')}
	       end
	       case {C hasErrors($)} then
		  {self.Result tk(conf
				  fg:BlockedThreadColor
				  text:'Compile Error')}
		  {System.printInfo {C getVS($)}}
	       else R Sync in
		  thread
		     EvalThread <- {Thread.this}
		     try
			R = {{C getEnv($)}.'`result`' Self}
		     finally
			Sync = unit
		     end
		  end
		  {Thread.preempt {Thread.this}}
		  {Spinner self.Result Sync}
		  {self.Result tk(conf text:{V2VS R})}
	       end
	       EvalThread <- unit
	    else
	       skip
	    end
	 end

	 proc {Eval}
	    {Doit {self.Expr tkReturn(get $)}}
	 end

	 proc {Exec}
	    {Doit {self.Expr tkReturn(get $)} # ' unit'}
	 end

	 proc {Kill}
	    case @EvalThread == unit then skip else
	       {Thread.terminate @EvalThread}
	       EvalThread <- unit
	    end
	    {Delay 120}
	    {self.Result tk(conf fg:DefaultForeground)}
	    {self.Result tk(conf text:'')}
	    %{self.Expr   tk(delete 0 'end')}
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
					 'Reset' # Kill
					 'Done'  # Close]
			       pack:    false
			       default: 1 /* <Return> calls 'Eval' */)

	 Frame = {New TkTools.textframe tkInit(parent: self
					       text: 'Eval Expression' #
						     ' / Exec Statement')}

	 ExprLabel = {New Tk.label tkInit(parent: Frame.inner
					  anchor: w
					  text:   'Query:')}
	 ExprEntry = {New TkExtEntry tkInit(parent:     Frame.inner
					    bd:         1
					    font:       DefaultFont
					    background: DefaultBackground
					    width:      40)}
	 ResultLabel = {New Tk.label tkInit(parent: Frame.inner
					    anchor: w
					    text:   'Result:')}
	 ResultEntry = {New Tk.label tkInit(parent:     Frame.inner
					    relief:     sunken
					    bd:         1
					    anchor:     w
					    font:       DefaultFont
					    background: DefaultBackground
					    width:      40)}

      in
	 self.Expr = ExprEntry
	 self.Result = ResultEntry

	 %% how to close the dialog
	 {ExprEntry tkBind(event: '<Escape>'
			   action: Close)}
	 {ExprEntry tkBind(event: '<Control-x>'
			   action: Close)}
	 %% resetting (kill eval/exec thread)
	 {ExprEntry tkBind(event: '<Control-r>'
			   action: Kill)}
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

in

   class Dialog

      meth about
	 {Wait {New AboutDialog init(master:self.toplevel)}.tkClosed}
      end

      meth eval
	 {Wait {New EvalDialog init(master:self.toplevel)}.tkClosed}
      end

   end

end
