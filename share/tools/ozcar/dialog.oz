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
			       buttons: ['Ok'#tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 T = {New Tk.label tkInit(parent: self
				  fg:     SelectedBackground
				  font:   HelpTitleFont
				  text:   'Oz Debugger')}

	 V = {New Tk.label tkInit(parent: self
				  text:   ('Last updated on ' # Version #
					   '\n' #
					   'Current platform is ' # Platform))}

	 A = {New Tk.label tkInit(parent: self
				  text:   NameOfBenni # '\n' # EmailOfBenni)}
      in
	 {Tk.send pack(T V A side:top expand:true)}
	 AboutDialog,tkPack
      end
   end

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

      meth init(master:Master)
	 fun {EvalInit}
	    C      = {New Compiler.interface.quiet init}
	    AuxEnv = {Ozcar PrivateSend(getEnv(unit $))}
	 in
	    CurComp <- C
	    CurEnv  <- {Record.adjoinList env {Append AuxEnv.'G' AuxEnv.'Y'}}

	    {C putEnv(\insert Base.env
		      )}
	    {C mergeEnv(\insert Standard.env
		       )}
	    {C mergeEnv(\insert Browser.env
		       )}
	    {C mergeEnv(@CurEnv)}
	    {C reset}

	    {self.Result tk(conf fg:DefaultForeground)}
	    {self.Result tk(delete 0 'end')}
	    C
	 end

	 proc {Dots W X}
	    case {IsFree X} then
	       {Delay 500}
	       {W tk(insert 'end' '.')}
	       {Dots W X}
	    else skip end
	 end

	 proc {Doit V}
	    case @EvalThread == unit then
	       EvalThread <- {Thread.this}
	       C           = {EvalInit}
	       Self        = {CondSelect @CurEnv 'self' unit}
	    in
	       case Self of unit then
		  {C feedVirtualString('declare fun {`result` _}\n' # V #
				       '\nend')}
	       else
		  {C feedVirtualString('\\switch +selfallowedanywhere\n' #
				       'declare fun {`result` Self}\n' #
				       '{`ooSetSelf` Self}' # V # '\nend')}
	       end
	       case {C hasErrors($)} then
		  {self.Result tk(conf fg:BlockedThreadColor)}
		  {self.Result tk(insert 0 'Compile Error')}
		  {System.printInfo {C getVS($)}}
	       else R in
		  thread try
			    R = {{C getEnv($)}.'`result`' Self}
			 finally
			    case {IsFree R} then R = unit else skip end
			 end
		  end
		  {Thread.preempt {Thread.this}}
		  {Dots self.Result R}
		  {self.Result tk(insert 0 {V2VS R})}
	       end
	       EvalThread <- unit
	    else
	       skip
	    end
	 end

	 proc {Eval}
	    {Doit {self.Expr tkReturn(get $)}}
	 end

	 proc {DoBrowse}
	    {Doit '{Browse ' # {self.Expr tkReturn(get $)} # '} unit'}
	 end

	 proc {Kill}
	    case @EvalThread == unit then skip else
	       {Thread.terminate @EvalThread}
	       EvalThread <- unit
	    end
	    {self.Result tk(conf fg:DefaultForeground)}
	    {self.Result tk(delete 0 'end')}
	    {self.Expr   tk(delete 0 'end')}
	 end

	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'Evaluate Expression'
			       buttons: ['Eval'   # Eval
					 'Browse' # DoBrowse
					 'Reset'  # Kill
					 'Done'   # tkClose]
			       pack:    false
			       default: 1)
	 F1 = {New Tk.frame tkInit(parent: self)}
	 F2 = {New Tk.frame tkInit(parent: self)}

	 ExprLabel = {New Tk.label tkInit(parent: F1
					  width:  6
					  anchor: w
					  text:   'Expr: ')}
	 ExprEntry = {New Tk.entry tkInit(parent:     F1
					  font:       DefaultFont
					  background: DefaultBackground
					  width:      40)}
	 ResultLabel = {New Tk.label tkInit(parent: F2
					    width:  6
					    anchor: w
					    text:   'Result: ')}
	 ResultEntry = {New Tk.entry tkInit(parent:     F2
					    font:       DefaultFont
					    background: DefaultBackground
					    width:      40)}

      in
	 self.Expr = ExprEntry
	 self.Result = ResultEntry
	 {Tk.batch [pack(F1 F2 side:top pady:2 expand:true)
		    pack(ExprLabel ExprEntry side:left expand:true)
		    pack(ResultLabel ResultEntry side:left expand:true)
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
