%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$


local

   class AboutDialog 
      from TkTools.dialog

      meth init(master:Master done:Done)
	 <<TkTools.dialog tkInit(master:  Master
				 title:   TitleName#': About'
				 buttons: ['Okay'#close(proc {$}
							   Done = Unit
							end)]
				 focus:   1
				 default: 1)>>
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       TitleName
				      foreground: AboutColor)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('Christian Schulte\n' #
					      '(schulte@dfki.uni-sb.de)\n'))}
      in
	 {Tk.send pack(Title Author
		       side:top expand:1 padx:Pad pady:Pad)}
      end

   end

   class ShutdownDialog 
      from TkTools.dialog

      meth init(master:Master done:Done)
	 <<TkTools.dialog tkInit(master:  Master
				 title:   TitleName#': Shutdown'
				 buttons: ['Okay'#close(proc {$}
							   Done = True
							end)
					   'Cancel'#close(proc {$}
							   Done = False
							end)]
				 focus:   1
				 default: 1)>>
	 Bitmap  = {New Tk.label   tkInit(parent: self
					  bitmap: question)}
	 Message = {New Tk.message tkInit(parent: self
					  aspect: 250
					  text:   'Do you really want to shutdown?')}   
      in
	 {Tk.send pack(Bitmap Message
		       side:left expand:1 padx:Pad pady:Pad)}
      end

   end

   local

      ScaleWidth  = 120
      FrameWidth  = ScaleWidth + 120
   
      fun {FindPos TLs FT N}
	 !TLs = T#L|TLr
      in
	 case T==FT then N else {FindPos TLr FT N+1} end
      end

   in
      
      class UpdateDialog
	 from TkTools.dialog
	 attr NextMouse: True
	 meth init(master: Master
		   prev:   Prev
		   next:   Next)
	    <<TkTools.dialog tkInit(master:  Master
				    title:   TitleName#': Update'
				    buttons: ['Okay'   #
					      close(proc {$}
						       Next =
						       {AdjoinAt
							{AdjoinAt Prev
							 time
							 {TimeScale get($)}}
							mouse
							{self GetMouse($)}}
						    end)
					      'Cancel' # close(proc {$}
								  Next = Prev
							       end)]
				    focus:   1
				    default: 1)>>
	    TimeOuter = {New Labelframe tkInit(parent: self
					       text:   'Update Time'
					       width:  FrameWidth
					       height: 40)}
	    TimeInner = {New Tk.frame tkInit(parent:             TimeOuter
					     highlightthickness: 0)}
	    TimeLabel = {New Tk.label tkInit(parent: TimeInner
					     text:   'Update every: ')}
	    TimeScale = {New DiscreteScale init(parent: TimeInner
						width:  ScaleWidth
						values: SampleTimes
						initpos: {FindPos SampleTimes
							  Prev.time 1})}
	    MouseOuter = {New Labelframe tkInit(parent: self
						text:   'Update Requirement'
						width:  FrameWidth
						height: 30)}
	    MouseInner = {New Tk.frame tkInit(parent:             MouseOuter
					      highlightthickness: 0)}
	    MouseVar   = {New Tk.variable tkInit(Prev.mouse)}
	    MouseButton = {New Tk.checkbutton
			   tkInit(parent:   MouseInner
				  variable: MouseVar
				  text:     'Require mouse over panel'
				  action:   self # ToggleMouse)}
	 in
	    NextMouse <- Prev.mouse
	    {TimeOuter  add(TimeInner)}
	    {MouseOuter add(MouseInner)}
	    {Tk.batch [pack(TimeLabel TimeScale side:left)
		       pack(MouseButton side:left)
		       pack(TimeOuter MouseOuter pady:Pad)]}
	 end

	 meth ToggleMouse
	    NextMouse <- {Not @NextMouse}
	 end

	 meth GetMouse($)
	    @NextMouse
	 end
	 
      end

   end

in

   fun {DoAbout Master}
      Done
   in
      _ = {New AboutDialog init(master:Master done:?Done)}
      Done
   end
      
   fun {DoShutdown Master}
      Done
   in
      _ = {New ShutdownDialog init(master:Master done:?Done)}
      Done
   end
      
   fun {DoOptionUpdate Master Prev}
      Next
   in
      _ = {New UpdateDialog init(master:Master prev:Prev next:?Next)}
      Next
   end

end

