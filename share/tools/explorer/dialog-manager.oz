%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   fun {UpdateOption SetOpts SetOptName MapOpts MapOptName Map}
      case {HasSubtreeAt SetOpts SetOptName} then
	 {AdjoinAt MapOpts MapOptName {Map SetOpts.SetOptName}}
      else MapOpts
      end
   end

   fun {Id X} X end
   

   class AboutDialog 
      from TkTools.dialog Object.closedFeature

      meth init(master:Master)
	 <<TkTools.dialog tkInit(master:  Master
				 title:   TitleName#': About'
				 buttons: ['Okay'#close]
				 focus:   1
				 default: 1)>>
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       TitleName
				      foreground: ChooseTermColor)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('by Christian Schulte\n' #
					      '(schulte@dfki.uni-sb.de)\n'))}
      in
	 {Tk.send pack(Title Author
		       side:top expand:1 padx:BigPad pady:BigPad)}
      end

   end

   class ErrorTool from TkTools.error Object.closedFeature end
   
   local
      RadioWidth = 12
      LabelWidth = 12      
   in

      class PostscriptDialog
	 from TkTools.dialog

	 meth init(master:Master previous:Prev options:?Options)
	    proc {Continue}
	       SizeString={SizeEntry tkReturn(get $)}
	    in
	       case {Misc.check SizeString}
	       of !False then true
	       elseof Size then
		  {self close}
		  Options = {Adjoin Size
			     o(size:  SizeString
			       color: {ColorVar tkReturn($)}
			       orient: {OrientVar tkReturn($)})}
	       end
	    end

	    <<TkTools.dialog tkInit(master:  Master
				    title:   TitleName#': Postscript Options'
				    buttons: ['Okay'#Continue
					      'Cancel'#close(proc {$}
								Options=Prev
							     end)]
				    default:  1)>>
	    Color  = {New Tk.frame tkInit(parent: self
					  relief: ridge
					  bd:     Border)}

	    ColorVar = {New Tk.variable tkInit(Prev.color)}
	    ColorLabel  = {New Tk.label tkInit(parent: Color
					       text:   'Color mode: '
					       width:   LabelWidth)}
	    ColorButton = {New Tk.radiobutton tkInit(parent:   Color
						     value:    color
						     variable: ColorVar
						     width:    RadioWidth
						     text:     'Full color')}
	    GrayButton  = {New Tk.radiobutton tkInit(parent:   Color
						     value:    grey
						     variable: ColorVar
						     width:    RadioWidth
						     text:     'Grayscale')}
	    MonoButton  = {New Tk.radiobutton tkInit(parent:   Color
						     value:    mono
						     variable: ColorVar
						     width:    RadioWidth
						     text:     'Black & white')}

	    Orient = {New Tk.frame tkInit(parent: self
					  relief: ridge
					  bd:     Border)}
	    OrientLabel  = {New Tk.label tkInit(parent: Orient
						text:   'Orientation: '
						width:  LabelWidth)}
	    OrientVar = {New Tk.variable tkInit(Prev.orient)}
	    Portrait  = {New Tk.radiobutton tkInit(parent:   Orient
						   value:    0
						   variable: OrientVar
						   width:    RadioWidth
						   text:     'Portrait  ')}
	    Landscape = {New Tk.radiobutton tkInit(parent:   Orient
						   value:    1
						   variable: OrientVar
						   width:    RadioWidth
						   text:     'Landscape')}
      
	    Size   = {New Tk.frame tkInit(parent: self
					  relief: ridge
					  bd:     Border)}
	    SizeLabel = {New Tk.label tkInit(parent: Size
					     text:   'Maximal size:'
					     width: LabelWidth)}      
	    SizeEntry = {New Tk.entry [tkInit(parent: Size
					      back:   EntryColor
					      width:  LargeEntryWidth)
				       tk(insert 0 Prev.size)]}
	 in
	    {Tk.batch [pack(ColorLabel ColorButton GrayButton MonoButton
			    side:left pady:Pad)
		       pack(OrientLabel Portrait Landscape
			    side:left fill:x pady:Pad)
		       pack(SizeLabel SizeEntry
			    side:left fill:x pady:Pad)
		       pack(Color Orient Size
			    side:top fill:x padx:BigPad pady:BigPad)
		       focus(SizeEntry)]}
	 end

      end
   
   end

   local
      TextWidth = 20
   in
      class LayoutDialog
	 from TkTools.dialog

	 meth init(master:   Master
		   previous: Previous
		   options:  ?Options)
	    <<TkTools.dialog
	      tkInit(master:  Master
		   title:   TitleName#': Drawing Options'
		   default: 1
		   buttons: ['Okay'#
			     close(proc {$}
				      Options =
				      options(hide:  {IsHide tkReturnInt($)}==1
					      update:
						 {Tk.string.toInt
						  case {Filter
							{RedrawEntry
							 tkReturn(get $)}
							Char.isDigit}
						  of nil then
						     DefLayoutOptions.update
						  elseof S then S
						  end})

				   end)
			     'Cancel'#
			     close(proc {$}
				      Options = Previous
				   end)])>>
	    IsHide     = {New Tk.variable
			  tkInit(case Previous.hide then 1 else 0 end)}
	    HideButton = {New Tk.checkbutton
			  tkInit(parent:   self
				 width:    TextWidth
				 anchor:   w
				 variable: IsHide
				 text:     'Hide failed subtrees')}
	    RedrawFrame = {New Tk.frame tkInit(parent:self)}
	    RedrawFirst = {New Tk.label tkInit(parent:RedrawFrame
					       anchor:w
					       text: 'Update every ')}
	    RedrawEntry = {New Tk.entry [tkInit(parent:RedrawFrame
						background: EntryColor
						width: SmallEntryWidth)
					 tk(insert 0 Previous.update)]}
	    RedrawSnd   = {New Tk.label tkInit(parent:RedrawFrame
					       anchor:w
					       text: ' solutions')}
	 in
	    {Tk.batch [pack(RedrawFirst RedrawEntry RedrawSnd
			    side:left fill:x)
		       pack(HideButton RedrawFrame
			    side:top fill:x)
		       focus(RedrawEntry)]}		
	 end
      
      end
   end

   
   local

      class DistanceWidget
	 from Tk.frame
	 feat mode entry
	 meth init(parent:   Parent
		   text:     Text
		   distance: Dist
		   custom:   CustomDist)
      	    <<Tk.frame tkInit(parent: Parent
			      relief: groove
			      bd:     Border)>>
	    Head    = {New Tk.label tkInit(parent: self
					   relief: ridge
					   bd:     Border
					   text:   Text)}
	    Buttons = {New Tk.frame tkInit(parent: self)}

	    Mode    = {New Tk.variable tkInit(case Dist
					      of  1 then none
					      [] ~1 then full
					      else       custom
					      end)}
	    
	    None    = {New Tk.radiobutton tkInit(parent:   Buttons
						 value:    none
						 variable: Mode
						 anchor:   w
						 text:     'None')}
	    Full    = {New Tk.radiobutton tkInit(parent:   Buttons
						 value:    full
						 variable: Mode
						 anchor:   w
						 text:     'Full')}
	    Custom  = {New Tk.frame tkInit(parent:             Buttons
					   highlightthickness: 0)}
	    CustomButton = {New Tk.radiobutton 
			    tkInit(parent:   Custom
				   value:    custom
				   variable: Mode
				   anchor:   w
				   text:     'Choose distance')}

	    CustomEntry  = {New Tk.entry
			    [tkInit(parent:   Custom
				    back:     EntryColor
				    width:    SmallEntryWidth)
			     tk(insert 0 CustomDist)]}
	 in
	    {Tk.batch [pack(CustomButton CustomEntry
			    side:left)
		       pack(None Full Custom
			    side:top padx:Pad pady:Pad fill:both expand:1)
		       pack(Head
			    side:top padx:BigPad pady:BigPad
			    ipadx:BigPad ipady:BigPad fill:x)
		       pack(Buttons side:left fill:y expand:1)]}
	    self.mode  = Mode
	    self.entry = CustomEntry
	 end

	 meth getDistance(?Dist ?CustomDist)
	    CustomDist = {self.entry tkReturnInt(get $)}
	    Dist       = case {self.mode tkReturn($)}
			 of "none" then 1
			 [] "full" then ~1
			 else CustomDist
			 end
	 end
      end

   in

      class SearchDialog
	 from TkTools.dialog

	 meth init(master:Master previous:Prev options:?Options)
	    <<TkTools.dialog
	    tkInit(master:  Master
		 title:   TitleName#': Search Options'
		 default: 1
		 buttons: ['Okay'#
			   proc {$}
			      GetDist GetCustomDist
			   in
			      {Dist getDistance(GetDist GetCustomDist)}
			      case
				 {IsInt GetDist} andthen
				 {IsInt GetCustomDist}
			      then
				 Options = o(dist:        GetDist
					     customDist:  GetCustomDist)
				 {self close}
			      else true
			      end
			   end
			   'Cancel'#
			   close(proc {$}
				    Options = Prev
				 end)])>>
	    Dist = {New DistanceWidget init(parent:   self
					    text:     'Search Recomputation'
					    distance: Prev.dist
					    custom:   Prev.customDist)}
	 in
	    {Tk.send pack(Dist
			  side:top fill:x padx:BigPad pady:BigPad)}
	 end

      end

      class InfoDialog
	 from TkTools.dialog
	 
	 meth init(master:Master previous:?Prev options:?Options)
	    <<TkTools.dialog
	      tkInit(master:  Master
		   title:   TitleName#': Information Options'
		   default: 1
		   buttons: ['Okay'#
			     proc {$}
				GetDist GetCustomDist
			     in
				{Dist getDistance(GetDist GetCustomDist)}
				case
				   {IsInt GetDist} andthen
				   {IsInt GetCustomDist}
				then
				   Options = o(dist:       GetDist
					       customDist: GetCustomDist)
				   {self close}
				else true
				end
			     end
			     'Cancel'#
			     close(proc {$}
				      Options = Prev
				   end)])>>
	    Dist = {New DistanceWidget init(parent: self
					    text:   'Information Recomputation'
					    distance: Prev.dist
					    custom:   Prev.customDist)}
	 in
	    {Tk.send pack(Dist
			  side:top fill:x padx:BigPad pady:BigPad)}
	 end

      end

   end

   class PostscriptManager
      attr
	 Options: DefPostscriptOptions
      feat
	 FS
      meth init
	 self.FS = {New TkTools.file init(master: self.toplevel
					  title:  TitleName#': Select Postscript File')}
      end
      
      meth postscriptOptions
	 Options <- {New PostscriptDialog init(master:   self.toplevel
					       previous: @Options
					       options:  $) _}
      end

      meth setPostscriptOptions(O)
	 Options <- {UpdateOption O mode
		     {UpdateOption O orientation
		      {UpdateOption O size
		       {UpdateOption O size
			{UpdateOption O size @Options
			 width fun {$ V}
				  {Misc.check {VirtualString.toString V}}.width
			       end}
			height fun {$ V}
				  {Misc.check {VirtualString.toString V}}.height
			       end}
		       size VirtualString.toString}
		      orient fun {$ O}
				case O of portrait then 0 else 1 end
			     end}
		     color fun {$ O}
			      case O
			      of bw then mono
			      [] color then color
			      [] grayscale then grey
			      end
			   end}
      end
			 
      meth postscript
	 Filename = {self.FS select(file:$)}
	 O        = @Options
      in
	 case Filename==False then true else
	    {self.canvas postscript(colormode:O.color
				    rotate:   O.orient
				    file:     Filename
				    height:   O.height
				    width:    O.width)}
	 end
      end

      meth close
	 {self.FS close}
      end
      
   end

   class SearchOptionsManager
      attr Options: DefSearchOptions

      meth searchOptions
	 NewOptions = {New SearchDialog init(master:   self.toplevel
					     previous: @Options
					     options:  $) _}
      in
	 case {Det NewOptions} then Options <- NewOptions end
      end

      meth setSearchOptions(O)
	 Options <- {UpdateOption O recomputation @Options dist Id}
      end
      
      meth getSearchDist($)
	 @Options.dist
      end
   end

   class InfoOptionsManager
      attr Options: DefInfoOptions

      meth infoOptions
	 NewOptions = {New InfoDialog init(master:   self.toplevel
					   previous: @Options
					   options:  $) _}
      in
	 case {Det NewOptions} then Options <- NewOptions end
      end

      meth setInfoOptions(O)
	 Options <- {UpdateOption O recomputation @Options dist Id}
      end

      meth getInfoDist($)
	 @Options.dist
      end

   end

   class LayoutOptionsManager
      attr Options: DefLayoutOptions

      meth layoutOptions
	 NewOptions = {New LayoutDialog init(master:   self.toplevel
					     previous: @Options
					     options:  $) _}
      in
	 case {Det NewOptions} then Options <- NewOptions end
      end

      meth setLayoutOptions(O)
	 Options <- {UpdateOption O hide
		     {UpdateOption O update @Options
		      update Id}
		     hide Id}
      end

      meth getAutoHide($)
	 @Options.hide
      end

      meth getUpdateSol($)
	 @Options.update
      end

   end

in

   class DialogManager
      from
	 PostscriptManager
	 SearchOptionsManager
	 InfoOptionsManager
	 LayoutOptionsManager

      meth init
	 <<PostscriptManager init>>
      end

      meth Lock(O)
	 case O.closed then <<UrObject nil>> end
      end

      meth about
	 <<DialogManager Lock({New AboutDialog init(master:self.toplevel)})>> 
      end

      meth error(M)
	 <<DialogManager Lock({New ErrorTool
			       tkInit(master:  self.toplevel
				      text:    M
				      title:   TitleName#': Error Message')})>>
      end

   end
end







