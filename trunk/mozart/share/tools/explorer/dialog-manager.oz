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
   
   class DialogClass
      feat toplevel
      from Tk.frame
      meth init(master:  Master
		title:   Title
		buttons: Buttons
		focus:   Focus  <= 0
		grab:    Grab   <= True
		return:  Return <= 0)
	 MasterX = {Tk.string.toInt {Tk.return winfo(rootx Master)}}
	 MasterY = {Tk.string.toInt {Tk.return winfo(rooty Master)}}
	 self.toplevel =
	 Toplevel = {New Tk.toplevel [tkInit
				      tkWM(withdraw)
				      tkWM(transient Master)
				      tkWM(title Title)]}

	 Top    = {New Tk.frame tkInit(parent:             Toplevel
				       bd:                 Border
				       relief:             raised
				       highlightthickness: 0)}

	 <<Tk.frame tkInit(parent:             Top
			   bd:                 BigBorder
			   highlightthickness: 0)>>

	 Bottom = {New Tk.frame tkInit(parent:             Toplevel
				       bd:                 Border
				       relief:             raised
				       highlightthickness: 0)}
	 ReturnButton
	 ReturnAction
	 FocusButton

	 TkButtons = {List.mapInd Buttons
		      fun {$ Ind Text#Action}
			 ButtonAction = case Action
					of close(Do) then
					   proc {$}
					      {Do} {Toplevel close}
					   end
					[] close then
					   proc {$}
					      {Toplevel close}
					   end
					[] release(Do) then
					   proc {$}
					      {Tk.send grab(release Toplevel)}
					      {Do}
					   end
					else Action
					end
			 Parent = case Ind==Return then
				     ReturnAction = ButtonAction
				     ReturnButton = Button
				     {New Tk.frame
				      tkInit(parent: Bottom
					     relief: sunken
					     bd:     Border)}
				  else Bottom
				  end
			 Button = {New Tk.button tkInit(parent: Parent
							relief: raised
							bd:     Border
							text:   Text
							action: ButtonAction)}
		      in
			 case Ind==Focus then FocusButton=Button
			 else true
			 end
			 case Ind==Return then Parent else Button end 
		      end}
      in
	 {Tk.batch [wm(resizable Toplevel 0 0)
		    pack(self o(side:top fill:both))
		    pack(Top o(side:top fill:both))]}
	 case Return>0 then
	    {Tk.send pack(ReturnButton)}
	 else true end
	 {Tk.batch [pack(b(TkButtons) 
			 o(padx:BigPad pady:BigPad side:left expand:1))
		    wm(geometry Toplevel '+'#MasterX#'+'#MasterY + 30)
		    wm(deiconify Toplevel)
		    pack(Bottom o(side:bottom fill:both))]}
	 case Return>0 then
	    {Toplevel tkBind(event:  '<Return>'
			     action: ReturnAction)}
	 else true end
	 case Focus>0 then {Tk.send focus(FocusButton)} else true end
	 case Grab then {Tk.send grab(set Toplevel)} else true end 
      end

      meth withdraw
	 {Tk.send wm(withdraw self.toplevel)}
      end

      meth release
	 {Tk.send grab(release self.toplevel)}
      end

      meth deiconify
	 {Tk.send wm(deiconify self.toplevel)}
      end

      meth close
	 {self.toplevel close}
      end
   end


   class AboutDialog from DialogClass

      meth init(master:Master)
	 <<DialogClass init(master:  Master
			    title:   TitleName#': About'
			    buttons: ['Okay'#close]
			    focus:   1
			    return:  1)>>
	 Title = {New Tk.label tkInit(parent:self
				      font:   AboutFont
				      text:  TitleName
				      foreground: ChoiceTermColor)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('by Christian Schulte\n' #
					      '(schulte@dfki.uni-sb.de)\n'))}
      in
	 {Tk.send pack(Title Author
		       o(side:top expand:1 padx:BigPad pady:BigPad))}
      end

   end


   class ErrorDialog from DialogClass

      meth init(master:Master message:M)
	 <<DialogClass init(master:  Master
			    title:   TitleName#': Error Message'
			    buttons: ['Okay'#close]
			    focus:   1
			    return:  1)>>
	 Bitmap = {New Tk.label tkInit(parent: self
				       bitmap: error)}
	 Message = {New Tk.message tkInit(parent: self
					  aspect: ErrorAspect
					  text:   M)}   
      in
	 {Tk.send pack(Bitmap Message
		       o(side:left expand:1 padx:BigPad pady:BigPad))}
      end

   end


   local

      Width         = 35

      local
	 fun {StringLess Is Js}
	    case Is of nil then True
	    [] I|Ir then
	       case Js of nil then False
	       [] J|Jr then I<J orelse I==J andthen {StringLess Ir Jr}
	       end
	    end
	 end
	 
	 proc {FilterAndSplit Fs P ?Ds ?Rs}
	    case Fs of nil then Ds=nil Rs=nil
	    [] F|Fr then
	       case F
	       of &.|_ then {FilterAndSplit Fr P ?Ds ?Rs}
	       else
		  case {Unix.stat P#'/'#F}
		  of error(_ _ _) then {FilterAndSplit Fr P ?Ds ?Rs}
		  elseof Stat then
		     case Stat.type
		     of dir then Ds = F#'/'|{FilterAndSplit Fr P $ ?Rs}
		     [] reg then Rs = F|{FilterAndSplit Fr P ?Ds $}
		     elseof error(_ V _) then {Show V} 
		     else {FilterAndSplit Fr P ?Ds ?Rs}
		     end
		  end
	       end
	    end
	 end
	 
      in
	 
	 fun {GetFiles P}
	    Ds Rs
	    UGD={Unix.getDir P}
	 in
	    case {Label UGD}==error then UGD
	    else
	       {FilterAndSplit {Sort {Unix.getDir P} StringLess} P ?Ds ?Rs}
	       "../" | {Append Ds Rs}
	    end
	 end
	 
      end
      
      
      fun {StripLast Is Js}
	 case Is of nil then Js
	 [] I|Ir then
	    case I==&/ then Is else {StripLast Ir Js} end
	 end
      end
      
      
      fun {MakePath P}
	 S={VirtualString.toString P}
      in
	 case {Reverse S}
	 of &/|_ then S
	 elseof RS then {Reverse &/|RS}
	 end
      end
      
      
      class Selector from DialogClass
	 feat
	    Files
	    Path
	    Name
	 attr
	    CurPath:  nil
	    Selected: False
   
	 meth init(title:Title master:Master)
	    <<DialogClass init(title:   Title
			       master:  Master
			       buttons: ['Okay'  #release(proc {$}
							     {self Okay}
							  end)
					 'Cancel'#release(proc {$}
							     {self Cancel}
							  end)]
			       return:  1)>>

	    Directory = {New Tk.label tkInit(parent: self
					     bd:     2
					     relief: ridge
					     width:  Width + 2
					     text:   '')}

	    Box = {New Tk.frame tkInit(parent:self)}

	    Listbox = {New Tk.listbox [tkInit(parent:          Box
					      exportselection: 0
					      selectmode:      single
					      width:           Width)
				       tkBind(event:  '<1>'
					      action: proc {$}
							 {self SingleClick}
						      end)
				       tkBind(event:  '<Double-1>'
					      action: proc {$}
							 {self [release DoubleClick]}
						      end)]}

	    Scroller = {New Tk.scrollbar tkInit(parent: Box
						width:  ScrollerWidth)}

	    {Tk.addYScrollbar Listbox Scroller}

	    FileName = {New Tk.entry tkInit(parent: self back:EntryColor)}
	 in
	    {Tk.batch [pack(Directory o(side:top fill:x expand:0))
		       pack(Scroller  o(side:right fill:y))
		       pack(Listbox)
		       pack(Box o(padx:BigPad))
		       pack(FileName
			    o(side:bottom fill:x pady:BigPad padx:BigPad))
		       focus(FileName)]}
	    self.Path  = Directory
	    self.Files = Listbox
	    self.Name  = FileName
	    <<DialogClass tkBind(event:  '<Return>'
				 action: proc {$}
					    {self [release Okay]}
					 end)>>
	 end

	 meth Display(P)
	    GF = {GetFiles P}
	 in
	    case GF of error(N M _) then
	       {New ErrorDialog init(master:self message:M#' ('#N#')') _}
	    else
	       CurPath <- P
	       {self.Path tk(configure o(text:P))}
	       {self.Files [tk(delete 0 'end') tk(insert 0 b(GF))]}
	       {self.Name tk(delete 0 'end')}
	    end
	 end

	 meth DoubleClick
	    F  = {self.Files tkReturn(get({self.Files
					   tkReturn(curselection $)}) $)}
	 in
	    case {List.last F}==&/ then
	       <<Selector Display(case F=="../" then
				     RCP = {Reverse @CurPath}.2
				  in
				     {Reverse {StripLast RCP RCP}}
				  else {Append @CurPath F}
				  end)>>
	    else
	       @Selected = @CurPath # F
	       Selected <- _
	    end
	 end
	 
	 meth SingleClick
	    F  = {self.Files tkReturn(get({self.Files
					   tkReturn(curselection $)}) $)}
	 in
	    case {List.last F}==&/ then true else
	       {self.Name [tk(delete 0 'end') tk(insert 0 F)]}
	    end	 
	 end
	 
	 meth Okay
	    @Selected = case {self.Name tkReturn(get $)}
			of "" then False
			elseof N then @CurPath#N
			end
	    Selected <- _
	 end

	 meth Cancel
	    @Selected = False
	    Selected <- _
	 end
      
	 meth select(P ?F)
	    Selected <- F
	    <<Selector Display(P)>>
	 end
	 
	 meth close
	    @Selected = close
	    <<DialogClass close>>
	 end
      
      end    

      NoPath = {NewName}
   
   in

      class Fileselector from UrObject
	 attr
	    Path:          nil
	    MySelector:    False
	 feat
	    Title Master
	 meth init(path:P <= NoPath title:T <= 'Select File'
		   master:M)
	    Path <- {MakePath case P==NoPath then {Unix.getCWD}
			      else P
			      end}
	    self.Title  = T
	    self.Master = M
	 end
	 
	 meth select(path:P<=NoPath file:?F)
	    TakePath = {MakePath case P==NoPath then @Path else P end}
	 in
	    Path <- TakePath
	    case @MySelector==False then
	       MySelector <- {New Selector init(title:  self.Title
						master: self.Master)}
	    else
	       {@MySelector deiconify}
	    end
	    F = case {@MySelector select(TakePath $)}   
		of !False then {@MySelector withdraw} False
		[] close then MySelector <- False False
		[] GP#FN then Path <- GP {@MySelector withdraw} GP#FN
		end
	 end

	 meth close
	    case @MySelector of !False then true elseof Sel then
	       {Sel close}
	    end
	    <<UrObject close>>
	 end

      end

   end


   local
      RadioWidth = 12
      LabelWidth = 12      
   in

      class PostscriptDialog from DialogClass

	 meth init(master:Master previous:Prev options:?Options)
	    proc {Continue}
	       SizeString={SizeEntry tkReturn(get $)}
	    in
	       case {Misc.check SizeString}
	       of !False then Options=Prev
	       elseof Size then
		  {self close}
		  Options = {Adjoin Size
			     o(size:  SizeString
			       color: {ColorVar tkReturn($)}
			       orient: {OrientVar tkReturn($)})}
	       end
	    end

	    <<DialogClass init(master:  Master
			       title:   TitleName#': Postscript Options'
			       buttons: ['Okay'#Continue
					 'Cancel'#close(proc {$}
							   Options=Prev
							end)]
			       return:  1)>>
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
					      width:  20)
				       tk(insert 0 Prev.size)]}
	 in
	    {Tk.batch [pack(ColorLabel ColorButton GrayButton MonoButton
			    o(side:left pady:Pad))
		       pack(OrientLabel Portrait Landscape
			    o(side:left fill:x pady:Pad))
		       pack(SizeLabel SizeEntry
			    o(side:left fill:x pady:Pad))
		       pack(Color Orient Size
			    o(side:top fill:x padx:BigPad pady:BigPad))
		       focus(SizeEntry)]}
	 end

      end
   
   end

   local
      TextWidth = 20
   in
      class LayoutDialog from DialogClass

	 meth init(master:   Master
		   previous: Previous
		   options:  ?Options)
	    <<DialogClass
	      init(master:  Master
		   title:   TitleName#': Drawing Options'
		   return:  1
		   buttons: ['Okay'#
			     close(proc {$}
				      Options =
				      options(hide:  {IsHide tkReturnInt($)}==1
					      wait:  {IsWait tkReturnInt($)}==1
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
				 onvalue:  1
				 offvalue: 0
				 text:     'Hide failed subtrees')}
	    IsWait     = {New Tk.variable
			  tkInit(case Previous.wait then 1 else 0 end)}
	    WaitButton = {New Tk.checkbutton
			  tkInit(parent:   self
				 width:    TextWidth
				 anchor:   w
				 onvalue:  1
				 offvalue: 0
				 variable: IsWait
				 text:     'Draw wait nodes')}
	    RedrawFrame = {New Tk.frame tkInit(parent:self)}
	    RedrawFirst = {New Tk.label tkInit(parent:RedrawFrame
					       anchor:w
					       text: 'Update every ')}
	    RedrawEntry = {New Tk.entry [tkInit(parent:RedrawFrame
						background: EntryColor
						width: 4)
					 tk(insert 0 Previous.update)]}
	    RedrawSnd   = {New Tk.label tkInit(parent:RedrawFrame
					       anchor:w
					       text: ' solutions')}
	 in
	    {Tk.batch [pack(RedrawFirst RedrawEntry RedrawSnd
			    o(side:left fill:x))
		       pack(HideButton WaitButton RedrawFrame
			    o(side:top fill:x))
		       focus(RedrawEntry)]}		
	 end
      
      end
   end

   
   local
      RadioWidth  = 16
      FrameHeight = 3.5#c
      FrameWidth  = 1.6#c

      class DistanceWidget
	 from Tk.frame
	 feat scale mode
	 meth init(parent:Parent text:Text distance:Dist)
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

	    Forget = {New Tk.action tkInit(parent: self
					   action: proc {$}
						      {Tk.send
						       pack(forget Scale)}
						   end)}
	    proc {PackScale}
	       {Tk.send pack(Scale o(fill:both))}
	    end
	    
	    Pack   = {New Tk.action tkInit(parent: self
					   action: proc {$} {PackScale} end)}

	    None    = {New Tk.radiobutton tkInit(parent:   Buttons
						 value:    none
						 variable: Mode
						 anchor:   w
						 width:    RadioWidth
						 command:  Forget
						 text:     'None')}
	    Full    = {New Tk.radiobutton tkInit(parent:   Buttons
						 value:    full
						 variable: Mode
						 anchor:   w
						 command:  Forget
						 width:    RadioWidth
						 text:     'Full')}
	    Custom  = {New Tk.radiobutton tkInit(parent:   Buttons
						 value:    custom
						 variable: Mode
						 anchor:   w
						 command:  Pack
						 width:    RadioWidth
						 text:     'Choose distance')}
	    
	    ScaleFrame = {New Tk.frame tkInit(parent:             self
					      highlightthickness: 0
					      height:             FrameHeight
					      width:              FrameWidth)}
	    Scale      = {New Tk.scale [tkInit(parent:    ScaleFrame
					       'from':    2
					       to:        DefRecomputeMax
					       showvalue: 1
					       width:     ScaleWidth)
					tk(set {Max Dist 2})]}
	 in
	    {Tk.batch [pack(None Full Custom
			    o(side:top padx:Pad pady:Pad fill:both expand:1))
		       pack(Head
			    o(side:top padx:BigPad pady:BigPad
			      ipadx:BigPad ipady:BigPad fill:x))
		       pack(Buttons o(side:left fill:y expand:1))
		       pack(ScaleFrame o(side:right expand:1))
		       pack(propagate ScaleFrame 0)]}
	    case Dist<2 then true else {PackScale} end
	    self.scale = Scale
	    self.mode  = Mode
	 end

	 meth getDistance(?Dist)
	    Dist = case {self.mode tkReturn($)}
		   of "none" then 1
		   [] "full" then ~1
		   else {self.scale tkReturnInt(get $)}
		   end
	 end
      end

   in

      class SearchDialog from DialogClass
	 meth init(master:Master previous:Prev options:?Options)
	    <<DialogClass
	    init(master:Master
		 title: TitleName#': Search Options'
		 focus:  1
		 return: 1
		 buttons: ['Okay'#
			   proc {$}
			      Options = o(dist:{Dist getDistance($)})
			      case {Det Options.dist} then
				 {self close}
			      end
			   end
			   'Cancel'#
			   close(proc {$}
				    Options = Prev
				 end)])>>
	    Dist = {New DistanceWidget init(parent:   self
					    text:     'Search Recomputation'
					    distance: Prev.dist)}
	 in
	    {Tk.send pack(Dist o(side:top fill:x padx:BigPad pady:BigPad))}
	 end

      end

      class InfoDialog from DialogClass
	 meth init(master:Master previous:?Prev options:?Options)
	    <<DialogClass
	      init(master:  Master
		   title:   TitleName#': Information Options'
		   focus:   1
		   return:  1
		   buttons: ['Okay'#
			     proc {$}
				Options = o(dist: {Dist getDistance($)}
					    keep: {IsKeep tkReturnInt($)}==1)
				case
				   {Det Options.dist} andthen
				   {Det Options.keep}
				then
				   {self close}
				end
			     end
			     'Cancel'#
			     close(proc {$}
				      Options = Prev
				   end)])>>
	    Dist = {New DistanceWidget init(parent: self
					    text:   'Information Recomputation'
					    distance: Prev.dist)}
	    IsKeep     = {New Tk.variable
			  tkInit(case Prev.keep then 1 else 0 end)}
	    KeepButton = {New Tk.checkbutton
			  tkInit(parent:   self
				 relief:   groove
				 border:   Border
				 anchor:   w
				 onvalue:  1
				 offvalue: 0
				 variable: IsKeep
				 text:     'Keep solution information')}
	 in
	    {Tk.batch [pack(Dist
			    o(side:top fill:x padx:BigPad pady:BigPad))
		       pack(KeepButton
			    o(side:bottom fill:x ipadx:BigPad ipady:BigPad
			      padx:BigPad pady:BigPad))]}
	 end

      end

   end

   class PostscriptManager
      attr
	 Options:  DefPostscriptOptions
      feat
	 FS
      meth init
	 self.FS = {New Fileselector init(master: self.toplevel
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
      attr
	 Options:      DefSearchOptions
	 ClearOptions: DefSearchOptions
      meth clear
	 ClearOptions <- @Options
      end
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
      
      meth getSearchDistance(?Dist)
	 Dist = @ClearOptions.dist
      end
   end

   class InfoOptionsManager
      attr
	 Options:      DefInfoOptions
	 ClearOptions: DefInfoOptions
      meth clear
	 ClearOptions <- @Options
      end
      meth infoOptions
	 NewOptions = {New InfoDialog init(master:   self.toplevel
					   previous: @Options
					   options:  $) _}
      in
	 case {Det NewOptions} then Options <- NewOptions end
      end
      meth setInfoOptions(O)
	 Options <- {UpdateOption O recomputation
		     {UpdateOption O solutions @Options
		      keep Id}
		     dist Id}
      end
      meth getInfoDistance(?Dist)
	 Dist = @ClearOptions.dist
      end
      meth getKeepSolutions(?Keep)
	 Keep = @ClearOptions.keep
      end
   end

   class LayoutOptionsManager
      attr
	 Options:      DefLayoutOptions
	 ClearOptions: DefLayoutOptions
      meth clear
	 ClearOptions <- @Options
      end
      meth layoutOptions
	 NewOptions = {New LayoutDialog init(master:   self.toplevel
					     previous: @Options
					     options:  $) _}
      in
	 case {Det NewOptions} then Options <- NewOptions end
      end
      meth setLayoutOptions(O)
	 Options <- {UpdateOption O hide
		     {UpdateOption O wait
		      {UpdateOption O update @Options
		       update Id}
		      wait Id}
		     hide Id}
      end
      meth getDisplayWaits(?Waits)
	 Waits = @ClearOptions.wait
      end
      meth getAutoHide(?Hide)
	 Hide  = @ClearOptions.hide
      end
      meth getUpdateSol(?Sols)
	 Sols  = @ClearOptions.update
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

      meth clear
	 <<SearchOptionsManager clear>>
	 <<InfoOptionsManager   clear>>
	 <<LayoutOptionsManager clear>>
      end
      
      meth Lock(O)
	 case {Object.closed O} then <<UrObject nil>> end
      end

      meth about
	 <<DialogManager Lock({New AboutDialog init(master:self.toplevel)})>> 
      end

      meth error(M)
	 <<DialogManager Lock({New ErrorDialog init(master:  self.toplevel
						    message: M)})>> 
      end

   end
end







