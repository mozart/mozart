functor

import
   System(show:Show)
   Application
   QTkFrame
   QTkMenu
   QTkSpace
   QTkLabel
   QTkButton
   QTkCheckbutton
   QTkRadiobutton
   QTkScale
   QTkScrollbar
   QTkEntry
   QTkCanvas
   QTkListbox
   QTkText
   QTkPlaceholder
   QTkGrid
   QTkRubberframe
   QTkScrollframe
   QTkPanel
   QTkToolbar
   QTkDropdownlistbox
   QTkNumberentry
   
define

   Start
   C={NewCell Start}
   proc{Gather T}
      N
      E={Access C}
      E=T|N
   in
      {Assign C N}
   end
   
   {ForAll [frame#QTkFrame
	    menu#QTkMenu
	    space#QTkSpace
	    label#QTkLabel
	    button#QTkButton
	    checkbutton#QTkCheckbutton
	    radiobutton#QTkRadiobutton
	    scale#QTkScale
	    scrollbar#QTkScrollbar
	    entry#QTkEntry
	    canvas#QTkCanvas
	    listbox#QTkListbox
	    text#QTkText
	    placeholder#QTkPlaceholder
	    grid#QTkGrid
	    rubberframe#QTkRubberframe
	    scrollframe#QTkScrollframe
	    panel#QTkPanel
	    toolbar#QTkToolbar
	    dropdrownlistbox#QTkDropdownlistbox
	    numberentry#QTkNumberentry]
    proc{$ N#V}
       {ForAll V.register proc{$ W}
			     {Gather N#W.widgetType}
			  end}
    end}

   {Access C}=nil
   
   CorrectList=[frame#td
		frame#lr
		menu#menubutton
		space#tdspace
		space#lrspace
		space#tdline
		space#lrline
		label#label
		label#message
		button#button
		checkbutton#checkbutton
		radiobutton#radiobutton
		scale#tdscale
		scale#lrscale
		scrollbar#tdscrollbar
		scrollbar#lrscrollbar
		entry#entry
		canvas#canvas
		listbox#listbox
		text#text
		placeholder#placeholder
		grid#grid
		rubberframe#tdrubberframe
		rubberframe#lrrubberframe
		scrollframe#scrollframe
		panel#panel
		toolbar#tbbutton
		toolbar#tbradiobutton
		toolbar#tbcheckbutton
		dropdrownlistbox#dropdownlistbox
		numberentry#numberentry]			    

   {ForAll Start
    proc{$ E}
       if {Not {List.member E CorrectList}} then
	  {Show E}
       end
    end}
   {Application.exit 0}
   
end

   
