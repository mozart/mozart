%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   class AboutDialog from TkTools.dialog
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
      feat
	 Expr
	 Result

      meth init(master:Master)
	 fun {EvalInit}
	    CC     = {New Compiler.interface.quiet init}
	    AuxEnv = {Ozcar PrivateSend(getEnv(unit $))}
	    CurEnv = {Record.adjoinList env {Append AuxEnv.'G' AuxEnv.'Y'}}
	 in
	    {CC putEnv(\insert Base.env
		      )}
	    {CC mergeEnv(\insert Standard.env
			)}
	    {CC mergeEnv(CurEnv)}

	    {CC reset}

	    {self.Result tk(conf fg:DefaultForeground)}
	    {self.Result tk(delete 0 'end')}
	    CC
	 end

	 proc {Eval}
	    CC  = {EvalInit}
	    V   = {self.Expr tkReturn(get $)}
	 in
	    {CC feedVirtualString('declare fun {`result`}\n' # V # '\nend')}
	    case {CC hasErrors($)} then
	       {self.Result tk(conf fg:BlockedThreadColor)}
	       {self.Result tk(insert 0 'Compiler Error')}
	       {System.printInfo {CC getVS($)}}
	    else
	       {self.Result tk(insert 0 {V2VS {{CC getEnv($)}.'`result`'}})}
	    end
	 end

	 proc {DoBrowse}
	    CC = {EvalInit}
	    V  = {self.Expr tkReturn(get $)}
	 in
	    {CC mergeEnv(\insert Browser.env
			)}
	    {CC feedVirtualString('{Browse ' # V # '}')}
	    case {CC hasErrors($)} then
	       {System.printInfo {CC getVS($)}}
	    else skip end
	 end

	 TkTools.dialog,tkInit(master:  Master
			       root:    pointer
			       title:   'Evaluate Expression'
			       buttons: ['Eval'   # Eval
					 'Browse' # DoBrowse
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
