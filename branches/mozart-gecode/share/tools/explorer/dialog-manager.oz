%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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
%%%

local

   class AboutDialog
      from TkTools.dialog
      prop final

      meth init(master:Master title:T)
	 TkTools.dialog,tkInit(master:  Master
			       title:   T#': About'
			       buttons: ['Okay'#tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       T
				      foreground: blue)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('Christian Schulte\n' #
					      '<schulte@ps.uni-sb.de>\n'))}
      in
	 {Tk.send pack(Title Author side:top expand:1 padx:BigPad pady:BigPad)}
	 AboutDialog,tkPack
      end

   end


   class PostscriptDialog
      from TkTools.dialog
      prop final

      meth init(master:M options:O title:TitleName)

	 proc {Okay}
	    SizeString={SizeEntry tkReturn(get $)}
	 in
	    case {CheckSize SizeString} of false then skip
	    elseof Size then
	       {Dictionary.put O size        SizeString}
	       {Dictionary.put O width       Size.width}
	       {Dictionary.put O height      Size.height}
	       {Dictionary.put O color       {ColorVar  tkReturnAtom($)}}
	       {Dictionary.put O orientation {OrientVar tkReturnAtom($)}}
	       {self tkClose}
	    end
	 end

	 TkTools.dialog,tkInit(master:  M
			       title:   TitleName#': Postscript'
			       buttons: ['Okay'#Okay 'Cancel'#tkClose]
			       pack:    false
			       default: 1)
	 Color     = {New TkTools.textframe tkInit(parent: self
						   text:   'Color Mode')}
	 ColorVar  = {New Tk.variable tkInit({Dictionary.get O color})}

	 Orient    = {New TkTools.textframe tkInit(parent: self
						   text:   'Orientation')}
	 OrientVar = {New Tk.variable tkInit({Dictionary.get O orientation})}

	 Size      = {New TkTools.textframe tkInit(parent: self
						   'class':'NumberEntry'
						   text:   'Size')}
	 SizeEntry = {New Tk.entry tkInit(parent: Size.inner
					  width:  LargeEntryWidth)}
      in
	 {SizeEntry tk(insert 0 {Dictionary.get O size})}
	 {Tk.batch [pack({New Tk.radiobutton tkInit(parent:   Color.inner
						    value:    color
						    variable: ColorVar
						    text:     'Full Color')}
			 {New Tk.radiobutton tkInit(parent:   Color.inner
						    value:    gray
						    variable: ColorVar
						    text:     'Grayscale')}
			 {New Tk.radiobutton tkInit(parent:   Color.inner
						    value:    mono
						    variable: ColorVar
						    text:     'Black & White')}
			 side:left pady:Pad)
		    pack({New Tk.radiobutton tkInit(parent:   Orient.inner
						    value:    false
						    variable: OrientVar
						    text:     'Portrait')}
			 {New Tk.radiobutton tkInit(parent:   Orient.inner
						    value:    true
						    variable: OrientVar
						    text:     'Landscape')}
			 side:left pady:Pad)
		    pack({New Tk.label tkInit(parent: Size.inner
					      text:   'Maximal Size:')}
			 SizeEntry side:left pady:Pad)
		    pack(Color Orient Size side:top fill:x)
		    focus(SizeEntry)]}
	 PostscriptDialog,tkPack
      end

   end


   class DrawingDialog
      from TkTools.dialog
      prop final

      meth init(master:M options:O title:TitleName)

	 proc {Okay}
	    {Dictionary.put O hide   {IsHide tkReturnInt($)}==1}
	    {Dictionary.put O scale  {IsScale tkReturnInt($)}==1}
	    {Dictionary.put O update {Update tkGet($)}}
	 end

	 TkTools.dialog,tkInit(master:  M
			       title:   TitleName#': Drawing'
			       default: 1
			       pack:    false
			       buttons: ['Okay'#tkClose(Okay)
					 'Cancel'#tkClose])

	 Drawing  = {New TkTools.textframe tkInit(parent:self
						  text:  'Drawing')}
	 IsHide   = {New Tk.variable tkInit({Dictionary.get O hide})}
	 IsScale  = {New Tk.variable tkInit({Dictionary.get O scale})}

	 Update   = {New TkTools.numberentry
		     tkInit(parent: Drawing.inner
			    min:1 val:{Dictionary.get O update})}
      in
	 {Tk.batch [grid({New Tk.checkbutton
			  tkInit(parent:   Drawing.inner
				 anchor:   w
				 variable: IsHide
				 text:     'Hide Failed Subtrees')}
			 row:0 column:0 columnspan:3 sticky:ew)
		    grid({New Tk.checkbutton
			  tkInit(parent:   Drawing.inner
				 anchor:   w
				 variable: IsScale
				 text:     'Scale to Fit')}
			 row:1 column:0 columnspan:3 sticky:ew)
		    grid({New Tk.label tkInit(parent: Drawing.inner
					      anchor: w
					      text:  'Update Every ')}
			 row:3 column:0)
		    grid(Update row:3 column:1)
		    grid({New Tk.label tkInit(parent: Drawing.inner
					      anchor: w
					      text:  ' Solutions')}
			 row:3 column:2)
		    pack(Drawing)]}
	 DrawingDialog,tkPack
      end

   end


   local

      fun {DistS2I S}
	 FS={Map {Filter S Char.isGraph} Char.toLower}
      in
	 if FS=="none" then 1
	 elseif FS=="full" then 0
	 else {Tk.string.toInt FS}
	 end
      end

      fun {DistI2VS I}
	 if I<1 then full
	 elseif I==1 then none
	 else I
	 end
      end

   in

      class SearchDialog
	 from TkTools.dialog
	 prop final

	 meth init(master:M options:O title:TitleName)

	    proc {Okay}
	       SD={DistS2I {Search tkReturn(get $)}}
	       ID={DistS2I {Info   tkReturn(get $)}}
	    in
	       if {IsInt SD} andthen {IsInt ID} then
		  {Dictionary.put O search      SD}
		  {Dictionary.put O information ID}
		  {Dictionary.put O failed      {FailedVar tkReturnInt($)}==1}
		  {self tkClose}
	       end
	    end

	    TkTools.dialog,tkInit(master:  M
				  title:   TitleName#': Search Options'
				  default: 1
				  pack:    false
				  buttons: ['Okay'#Okay 'Cancel'#tkClose])
	    Recomp = {New TkTools.textframe tkInit(parent: self
						   text:   'Recomputation')}
	    Left   = {New Tk.frame tkInit(parent:Recomp.inner
					 'class':'NumberEntry')}
	    Search = {New Tk.entry tkInit(parent:Left
					  width: SmallEntryWidth)}
	    Info   = {New Tk.entry tkInit(parent:Left
					  width: SmallEntryWidth)}
	    FailedVar = {New Tk.variable tkInit}
	    Right  = {New Tk.frame tkInit(parent:Recomp.inner)}

	    proc {Enter S#I#F}
	       {Search tk(delete 0 'end')} {Search tk(insert 0 S)}
	       {Info   tk(delete 0 'end')} {Info   tk(insert 0 I)}
	       {FailedVar tkSet(F)}
	    end

	 in
	    {Enter ({DistI2VS {Dictionary.get O search}} #
		    {DistI2VS {Dictionary.get O information}} #
		    {Dictionary.get O failed})}

	    {Tk.batch [grid({New Tk.label tkInit(parent:Left
						 text:  'Search:'
						 anchor:w)}
			    row:0 column:0 sticky:w)
		       grid(Search row:0 column:1 sticky:w)
		       grid({New Tk.label tkInit(parent:Left
						 text:  'Information:'
						 anchor:w)}
			    row:1 column:0 sticky:w)
		       grid(Info row:1 column:1 sticky:w)
		       grid({New Tk.checkbutton
			     tkInit(parent:Left
				    text:
				       'Full Recomputation in Failed Subtrees'
				    anchor:w
				    var:FailedVar)}
			    row:2 column:0 columnspan:2 sticky:we)

		       pack({New Tk.button tkInit(parent: Right
						  text:   'Normal'
						  action: Enter #
							  (none # 5 # true))}
			    {New Tk.button tkInit(parent: Right
						  text:   'Large'
						  action: Enter #
							  (5 # 25 # true))}
			    {New Tk.button tkInit(parent: Right
						  text:   'Huge'
						  action: Enter #
							  (25 # full # true))}
			    fill:x)

		       pack(Left side:left anchor:n)
		       pack({New Tk.frame tkInit(parent:Recomp.inner)}
			    side:left ipadx:1#c)
		       pack(Right side:right anchor:n)

		       pack(Recomp fill:x)]}
	    SearchDialog,tkPack
	 end

      end

   end

in

   class DialogManager

      meth guiOptions(What)
	 {Wait {New case What
		    of postscript then PostscriptDialog
		    [] search     then SearchDialog
		    [] drawing    then DrawingDialog
		    end
		    init(master:  self.toplevel
			 options: self.options.What
			 title:   {Dictionary.get self.options.visual
				   title})}.tkClosed}
      end

      meth postscript
	 case {Tk.return
	       tk_getSaveFile(filetypes:  q(q('Postscript Files' q('.ps'))
					    q('All Files'        '*'))
			      parent:     self.toplevel
			      title:      {Dictionary.get self.options.visual
					   title}#': Export Postscript')}
	 of nil then skip
	 elseof S then O = self.options.postscript in
	    {self.canvas postscript(colormode: {Dictionary.get O color}
				    rotate:    {Dictionary.get O orientation}
				    file:      S
				    height:    {Dictionary.get O height}
				    width:     {Dictionary.get O width})}

	 end
      end

      meth about
	 {Wait {New AboutDialog
		init(master:self.toplevel
		     title: {Dictionary.get self.options.visual title}
		    )}.tkClosed}
      end

      meth error(M)
	 {Wait {New TkTools.error
		tkInit(master:  self.toplevel
		       text:    M
		       title:   ({Dictionary.get self.options.visual title}#
				 ': Error Message'))}.tkClosed}
      end

   end
end







