%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 2001
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
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
   Application(exit)
   OS(localTime)
   Open(file)
   Property(get)
   GBuilder(create: Create) at 'x-oz://system/gtk/GBuilder.ozf'
   System(show)
define
   local
      fun {NColumns Rows}
	 {FoldR Rows fun {$ Row In} {Max {Length Row} In} end 0}
      end

      fun {MakeRow Row I J Rest}
	 case Row of unit|Cells then
	    {MakeRow Cells I J + 1 Rest}
	 [] Cell|Cells then
	    attach(left: I top: J
		   xpadding: 5 ypadding: 5 Cell)|
	    {MakeRow Cells I J + 1 Rest}
	 [] nil then Rest
	 end
      end

      fun {MakeRows Rows I Rest}
	 case Rows of Row|Rows then
	    {MakeRow Row I 0 {MakeRows Rows I + 1 Rest}}
	 [] nil then Rest
	 end
      end
   in
      fun {MakeTable Rows}
	 table(nRows: {Length Rows} nColumns: {NColumns Rows}
	       borderWidth: 5
	       homogeneous: true
	       children: {MakeRows Rows 0 nil})
      end
   end

   fun {MakeArrows ArrowType}
      label(label: ArrowType)|
      {Map ['in' out etchedIn etchedOut]
       fun {$ ShadowType}
	  arrow(arrowType: ArrowType
		shadowType: ShadowType)
       end}
   end

   ArrowDesc = {MakeTable (unit|{Map ['in' out etchedIn etchedOut]
				 fun {$ ArrowType}
				    label(label: ArrowType
					  xalign: 1.0)
				 end})|{Map [up down left right] MakeArrows}}

   local
      Label Group

      fun {MakeButton Relief}
	 button(label: Relief
		relief: Relief
		clicked:
		   proc {$ _}
		      {Label conf(label: 'button `'#Relief#'\' clicked')}
		   end)
      end

      fun {MakeToggleButton Relief}
	 toggleButton(
	    label: Relief
	    relief: Relief
	    toggled:
	       proc {$ _}
		  {Label conf(label: 'toggle button `'#Relief#'\' toggled')}
	       end)
      end

      fun {MakeCheckButton Relief}
	 checkButton(
	    label: Relief
	    relief: Relief
	    toggled:
	       proc {$ _}
		  {Label conf(label: 'check button `'#Relief#'\' toggled')}
	       end)
      end

      fun {MakeRadioButton Relief Group}
	 {Adjoin
	  radioButton(
	     label: Relief
	     relief: Relief
	     toggled:
		proc {$ _}
		   {Label conf(label: 'radio button `'#Relief#'\' toggled')}
		end)
	  case Group of handle(Handle) then radioButton(handle: Handle)
	  [] group(Group) then radioButton(group: Group)
	  end}
      end
   in
      ButtonDesc =
      table(nRows: 5 nColumns: 3
	    columnSpacing: 5 rowSpacing: 5
	    homogeneous: true
	    attach(left: 0 right: 3 top: 0
		   label(handle: Label))
	    %% buttons
	    attach(left: 0 top: 1
		   xoptions: nil yoptions: nil
		   {MakeButton normal})
	    attach(left: 1 top: 1
		   xoptions: nil yoptions: nil
		   {MakeButton half})
	    attach(left: 2 top: 1
		   xoptions: nil yoptions: nil
		   {MakeButton none})
	    %% toggle buttons
	    attach(left: 0 top: 2
		   xoptions: nil yoptions: nil
		   {MakeToggleButton normal})
	    attach(left: 1 top: 2
		   xoptions: nil yoptions: nil
		   {MakeToggleButton half})
	    attach(left: 2 top: 2
		   xoptions: nil yoptions: nil
		   {MakeToggleButton none})
	    %% check buttons
	    attach(left: 0 top: 3
		   xoptions: nil yoptions: nil
		   {MakeCheckButton normal})
	    attach(left: 1 top: 3
		   xoptions: nil yoptions: nil
		   {MakeCheckButton half})
	    attach(left: 2 top: 3
		   xoptions: nil yoptions: nil
		   {MakeCheckButton none})
	    %% radio buttons
	    attach(left: 0 top: 4
		   xoptions: nil yoptions: nil
		   {MakeRadioButton normal handle(Group)})
	    attach(left: 1 top: 4
		   xoptions: nil yoptions: nil
		   {MakeRadioButton half group(Group)})
	    attach(left: 2 top: 4
		   xoptions: nil yoptions: nil
		   {MakeRadioButton none group(Group)}))
   end

   CalendarDesc =
   calendar(displayOptions: [showHeading showDayNames showWeekNumbers]
	    day: {OS.localTime}.mDay)

   local
      fun {MakeFrame L T Shadow XAlign YAlign}
	 attach(left: L top: T
		frame(label: Shadow
		      shadow: Shadow
		      labelXalign: XAlign
		      labelYalign: YAlign
		      add(label(label: 'foo'))))
      end
   in
      FrameDesc =
      table(nColumns: 4 nRows: 3
	    columnSpacing: 5 rowSpacing: 5
	    homogeneous: true
	    borderWidth: 5
	    {MakeFrame 0 0 'in' 0.0 0.5}
	    {MakeFrame 1 0 'out' 0.0 0.5}
	    {MakeFrame 2 0 'etchedIn' 0.0 0.5}
	    {MakeFrame 3 0 'etchedOut' 0.0 0.5}
	    {MakeFrame 0 1 'in' 1.0 0.0}
	    {MakeFrame 1 1 'out' 1.0 0.0}
	    {MakeFrame 2 1 'etchedIn' 1.0 0.0}
	    {MakeFrame 3 1 'etchedOut' 1.0 0.0}
	    {MakeFrame 0 2 'in' 0.5 1.0}
	    {MakeFrame 1 2 'out' 0.5 1.0}
	    {MakeFrame 2 2 'etchedIn' 0.5 1.0}
	    {MakeFrame 3 2 'etchedOut' 0.5 1.0})
   end

   proc {LoadFile Filename ?Contents}
      try F in
	 F = {New Open.file init(name: Filename flags: [read])}
	 {F read(size: all list: ?Contents)}
	 {F close()}
      catch _ then
	 Contents = 'Could not read file '#Filename
      end
   end

   Text

   local
      proc {LoadFileAction _}
	 Dialog = {Create fileSelection(title: 'Open File')}
      in
	 {{Dialog fileSelectionGetFieldOkButton($)}
	  signalConnect(clicked
			proc {$ _} FileName in
			   FileName = {Dialog getFilename($)}
			   {Dialog destroy()}
			   {Text deleteText(0 ~1)}
			   {Text insert(unit unit unit {LoadFile FileName} ~1)}
			end _)}
	 {{Dialog fileSelectionGetFieldCancelButton($)}
	  signalConnect(clicked proc {$ _} {Dialog destroy()} end _)}
	 {Dialog showAll()}
      end

      Username Password
   in
      EditableDesc =
      table(nRows: 4 nColumns: 3
	    attach(left: 0 top: 0 yoptions: nil
		   label(label: 'Username: '
			 xalign: 0.0))
	    attach(left: 1 top: 0 yoptions: nil
		   entry(visibility: true
			 handle: Username))
	    attach(left: 0 top: 1 yoptions: nil
		   label(label: 'Password: '
			 xalign: 0.0))
	    attach(left: 2 top: 0 bottom: 2 yoptions: nil
		   button(label: 'Test'
			  clicked:
			     proc {$ _}
				{System.show
				 test(user: {Username return(text: $)}
				      pass: {Password return(text: $)})}
			     end))
	    attach(left: 1 top: 1 yoptions: nil
		   entry(visibility: false
			 handle: Password))
	    attach(left: 0 right: 3 top: 2
		   scrolledWindow(add(text(editable: true
					   handle: Text))))
	    attach(left: 0 right: 3 top: 3 yoptions: nil
		   button(label: 'Open file'
			  clicked: LoadFileAction)))
   end

   {{Create
     window(type: toplevel
	    title: 'GBuilder Examples'
	    destroy: proc {$ _} {Application.exit 0} end
	    add(notebook(tabPos: left
			 appendPage(tabLabel: label(label: 'Arrows')
				    ArrowDesc)
			 appendPage(tabLabel: label(label: 'Buttons')
				    ButtonDesc)
			 appendPage(tabLabel: label(label: 'Calendar')
				    CalendarDesc)
			 appendPage(tabLabel: label(label: 'Editables')
				    EditableDesc)
			 appendPage(tabLabel: label(label: 'Frames')
				    FrameDesc))))}
    showAll()}

   {Text insert(unit unit unit
		{LoadFile {Property.get 'oz.home'}#'/LICENSE'} ~1)}
end
