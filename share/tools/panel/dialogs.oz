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
	 TkTools.dialog,tkInit(master:  Master
			       title:   TitleName#': About'
			       buttons: ['Okay'#close(proc {$}
							 Done = unit
						      end)]
			       focus:   1
			       default: 1)
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
	 TkTools.dialog,tkInit(master:  Master
			       title:   TitleName#': Shutdown'
			       buttons: ['Okay'#close(proc {$}
							 Done = True
						      end)
					 'Cancel'#close(proc {$}
							   Done = False
							end)]
			       focus:   1
			       default: 1)
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

      fun {FindPos TLs FT N}
	 T#L|TLr = !TLs
      in
	 case T==FT then N else {FindPos TLr FT N+1} end
      end
      
      ScaleWidth  = 100

   in
      
      class UpdateDialog
	 from TkTools.dialog
	 attr NextMouse: True
	 meth init(master: Master
		   prev:   Prev
		   next:   Next)
	    TkTools.dialog,tkInit(master:  Master
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
				  default: 1)
	    TimeOuter = {New TkTools.textframe tkInit(parent: self
						      text:   'Update Time')}
	    TimeLabel = {New Tk.label tkInit(parent: TimeOuter.inner
					     text:   'Update every: ')}
	    TimeScale = {New DiscreteScale init(parent: TimeOuter.inner
						width:  ScaleWidth
						values: UpdateTimes
						initpos: {FindPos UpdateTimes
							  Prev.time 1})}
	    MouseOuter = {New TkTools.textframe tkInit(parent: self
						       text:   'Update Requirement')}
	    MouseVar   = {New Tk.variable tkInit(Prev.mouse)}
	    MouseButton = {New Tk.checkbutton
			   tkInit(parent:   MouseOuter.inner
				  variable: MouseVar
				  text:     'Require mouse over panel'
				  action:   self # ToggleMouse)}
	 in
	    NextMouse <- Prev.mouse
	    {Tk.batch [pack(TimeLabel TimeScale side:left fill:x)
		       pack(MouseButton side:left fill:x)
		       pack(TimeOuter MouseOuter fill:x)]}
	 end

	 meth ToggleMouse
	    NextMouse <- {Not @NextMouse}
	 end

	 meth GetMouse($)
	    @NextMouse
	 end
	 
      end

      
      class HistoryDialog
	 from TkTools.dialog

	 meth init(master: Master
		   prev:   Prev
		   next:   Next)
	    TkTools.dialog,tkInit(master:  Master
				  title:   TitleName#': History'
				  buttons: ['Okay'   #
					    close(proc {$}
						     Next = 
						     {RangeScale get($)}
						  end)
					    'Cancel' # close(proc {$}
								Next = Prev
							     end)]
				  focus:   1
				  default: 1)
	    RangeOuter = {New TkTools.textframe tkInit(parent: self
						       text:   'History Range')}
	    RangeLabel = {New Tk.label tkInit(parent: RangeOuter.inner
					     text:   'Range covers: ')}
	    RangeScale = {New DiscreteScale init(parent:  RangeOuter.inner
						 width:   ScaleWidth
						 values:  HistoryRanges
						 initpos: {FindPos
							   HistoryRanges
							   Prev 1})}
	 in
	    {Tk.batch [pack(RangeLabel RangeScale side:left fill:x)
		       pack(RangeOuter)]}
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

   fun {DoOptionHistory Master Prev}
      Next
   in
      _ = {New HistoryDialog init(master:Master prev:Prev next:?Next)}
      Next
   end

end

