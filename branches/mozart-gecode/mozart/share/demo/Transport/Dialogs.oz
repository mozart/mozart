%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
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

functor

import
   Tk TkTools

   Widgets(entryChooser)
   Country(isCity cities)
   Configure(title colors fonts)
   
export
   About
   AddCompany
   RemCompany
   AddDriver
   RemDriver

prepare
   Pad           = 2
   BigPad        = 4
   BigTextWidth  = 17

define

   TextBg    = Configure.colors.textBg
   AboutFont = Configure.fonts.about
   
   class About
      from TkTools.dialog

      meth init(master:Master)
	 TkTools.dialog,tkInit(master:  Master
			       title:   Configure.title#': About'
			       buttons: ['Okay'#tkClose]
			       focus:   1
			       pack:    false
			       default: 1)
	 Title = {New Tk.label tkInit(parent:     self
				      font:       AboutFont
				      text:       Configure.title
				      foreground: blue)}

	 Author = {New Tk.label tkInit(parent: self
				       text: ('Christian Schulte\n' #
					      '<schulte@ps.uni-sb.de>\n'))}
      in
	 {Tk.send pack(Title Author side:top expand:1 padx:BigPad pady:BigPad)}
	 About,tkPack
      end

   end


   class OkayDialog
      from TkTools.dialog
	 
      meth init(master:M title:T okay:O)
	 TkTools.dialog,tkInit(master:  M
			       title:   Configure.title#': '#T
			       buttons: ['Okay'#O 'Cancel'#tkClose]
			       pack:    false
			       default: 1)
      end
   end

   proc {Error M T}
      {Wait {New TkTools.error tkInit(master:M text:T)}.tkClosed}
   end

   class AddCompany
      from OkayDialog
      prop final
      meth init(master:M agents:AS company:C)

	 proc {Okay}
	    AddC={Entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS AddC} then
	       {Error self 'Company '#AddC#' already exists.'}
	    else C=AddC {self tkClose}
	    end
	 end

	 OkayDialog,init(master:M title:'Add Company' okay:Okay)
	 Frame = {New TkTools.textframe tkInit(parent:self text:'Add Company')}
	 Name  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 Entry = {New Tk.entry tkInit(parent:Frame.inner bg:TextBg
				      width:BigTextWidth)}
      in
	 {Tk.batch [pack(Name Entry side:left padx:Pad pady:Pad)
		    pack(Frame) focus(Entry)]}
	 AddCompany,tkPack
      end

   end


   class RemCompany
      from OkayDialog
      prop final

      meth init(master:M agents:AS company:C)
	 proc {Okay}
	    RemC={Entry.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS RemC} then C=RemC {self tkClose}
	    else {Error self 'There is no company with name: '#RemC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Remove Company' okay:Okay)
	 Frame = {New TkTools.textframe tkInit(parent:self
					       text:'Remove Company')}
	 Name  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 Entry = {New Widgets.entryChooser
		  tkInit(parent:Frame.inner
			 toplevel:self.toplevel
			 entries:{Dictionary.keys AS})}
      in
	 {Tk.batch [pack(Name Entry side:left padx:Pad pady:Pad)
		    pack(Frame) focus(Entry.entry)]}
	 RemCompany,tkPack
      end

   end


   class AddDriver
      from OkayDialog
      prop final

      meth init(master:M agents:AS company:C driver:D city:Y)
	 proc {Okay}
	    AddC={EntryC.entry tkReturnAtom(get $)}
	    AddD={EntryD       tkReturnAtom(get $)}
	    AddY={EntryY.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS AddC} then
	       if {Member AddD {Dictionary.get AS AddC}} then
		  {Error self 'Driver '#AddD#' already exists for company '#
		              AddC#'.'}
	       elseif {Country.isCity AddY} then
		  C=AddC D=AddD Y=AddY  {self tkClose}
	       else
		  {Error self 'There is no city '#AddY#'.'}
	       end
	    else {Error self 'There is no company '#AddC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Add Driver' okay:Okay)
	 Frame  = {New TkTools.textframe tkInit(parent:self text:'Add Driver')}
	 NameC  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 EntryC = {New Widgets.entryChooser
		   tkInit(parent:   Frame.inner
			  toplevel: self.toplevel
			  entries:  {Dictionary.keys AS})}
	 NameD  = {New Tk.label tkInit(parent:Frame.inner text:'Driver:')}
	 EntryD = {New Tk.entry tkInit(parent:Frame.inner bg:TextBg)}
	 NameY  = {New Tk.label tkInit(parent:Frame.inner text:'City:')}
	 EntryY = {New Widgets.entryChooser
		   tkInit(parent:   Frame.inner
			  toplevel: self.toplevel
			  entries:  Country.cities)}
      in
	 {Tk.batch [grid(NameC  row:0 column:0 sticky:w)
		    grid(NameD  row:1 column:0 sticky:w)
		    grid(NameY  row:2 column:0 sticky:w)
		    grid(EntryC row:0 column:1 sticky:we)
		    grid(EntryD row:1 column:1 sticky:we)
		    grid(EntryY row:2 column:1 sticky:we)
		    pack(Frame)]}
	 AddDriver,tkPack
      end

   end


   class RemDriver
      from OkayDialog
      prop final

      meth init(master:M agents:AS company:C driver:D)
	 Companies = {Filter {Dictionary.keys AS}
		      fun {$ C}
			 {Dictionary.get AS C}\=nil
		      end}
	 
	 proc {Okay}
	    RemC={EntryC.entry tkReturnAtom(get $)}
	    RemD={EntryD.entry tkReturnAtom(get $)}
	 in
	    if {Dictionary.member AS RemC} then
	       if {Member RemD {Dictionary.get AS RemC}} then
		  C=RemC D=RemD {self tkClose}
	       else
		  {Error self 'No driver '#RemD#' for company '#RemC#'.'}
	       end
	    else {Error self 'There is no company '#RemC#'.'}
	    end
	 end

	 OkayDialog, init(master:M title:'Remove Driver' okay:Okay)
	 Frame  = {New TkTools.textframe tkInit(parent:self text:'Remove Driver')}
	 NameC  = {New Tk.label tkInit(parent:Frame.inner text:'Company:')}
	 EntryC = {New Widgets.entryChooser
		   tkInit(parent:   Frame.inner
			  toplevel: self.toplevel
			  entries:  Companies
			  action:   proc {$ A}
				       {EntryD entries({Dictionary.get AS A})}
				    end)}
	 NameD  = {New Tk.label tkInit(parent:Frame.inner text:'Driver:')}
	 EntryD = {New Widgets.entryChooser
		   tkInit(parent:   Frame.inner
			  toplevel: self.toplevel
			  entries:  {Dictionary.get AS Companies.1})}
      in
	 {Tk.batch [grid(NameC  row:0 column:0 sticky:w)
		    grid(NameD  row:1 column:0 sticky:w)
		    grid(EntryC row:0 column:1 sticky:we)
		    grid(EntryD row:1 column:1 sticky:we)
		    pack(Frame)]}
	 RemDriver,tkPack
      end

   end
   
end







