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
export
   Widgets
prepare
   BoxAddOptions = [expand(type: boolean
			   default: true)
		    fill(type: boolean
			 default: true)
		    padding(type: int
			    default: 0)]

   PanedAddOptions = [resize(type: boolean
			     default: true)
		      shrink(type: boolean
			     default: false)]

   Widgets =
   widgets(

      %% GDK

      color:
	 o(api: gdk
	   args: [red(type: int)
		  green(type: int)
		  blue(type: int)]
	   new: [[red green blue]#new])

      font:
	 o(api: gdk
	   args: [name(type: string)
		  foundry(type: string
			  default: '*')
		  family(type: string
			 default: '*')
		  weight(type: string
			 default: '*')
		  slant(type: string
			default: '*')
		  width(type: string
			default: '*')
		  style(type: string
			default: '*')
		  pixelSize(type: int
			    default: '*')
		  pointSize(type: int
			    default: '*')
		  xResolution(type: int
			      default: '*')
		  yResolution(type: int
			      default: '*')
		  spacing(type: string
			  default: '*')
		  averageWidth(type: int
			       default: '*')
		  registry(type: string
			   default: '*')
		  encoding(type: string
			   default: '*')]
	   new: [[foundry family weight slant width style pixelSize pointSize
		  xResolution yResolution spacing averageWidth registry
		  encoding]#
		 fun {$ Class A B C D E F G H I J K L M N}
		    {New Class load('-'#A#'-'#B#'-'#C#'-'#D#'-'#E#'-'#F#'-'#G#
				    '-'#H#'-'#I#'-'#J#'-'#K#'-'#L#'-'#M#'-'#N)}
		 end
		 [string]#load])

      %% GTK

      accelLabel:
	 o(api: gtk
	   isa: label
	   args: [accelWidget(type: object(widget)
			      get: generic
			      set: setAccelWidget)]
	   new: [[label]#new])

      adjustment:
	 o(api: gtk
	   isa: data
	   args: [value(type: float
			set: setValue)
		  lower(type: float)
		  upper(type: float)
		  stepIncrement(type: float)
		  pageIncrement(type: float)
		  pageSize(type: float)]
	   signals: [changed valueChanged]
	   new: [[value lower upper stepIncrement pageIncrement pageSize]#new])

      alignment:
	 o(api: gtk
	   isa: bin
	   args: [xalign(type: float
			 get: generic
			 set: generic)
		  yalign(type: float
			 get: generic
			 set: generic)
		  xscale(type: float
			 get: generic
			 set: generic)
		  yscale(type: float
			 get: generic
			 set: generic)]
	   new: [[xalign yalign xscale yscale]#new]
	   set: [[xalign yalign xscale yscale]#set])

      arrow:
	 o(api: gtk
	   isa: misc
	   args: [arrowType(type: arrowType
			    get: generic)
		  shadowType(type: shadowType
			     default: none
			     get: generic)]
	   new: [[arrowType shadowType]#new]
	   set: [[arrowType shadowType]#set])

      aspectFrame:
	 o(api: gtk
	   isa: frame
	   args: [label(type: option(string)
			default: unit)
		  xalign(type: float
			 default: 0.5
			 get: aspectFrameGetFieldXalign)
		  yalign(type: float
			 default: 0.5
			 get: aspectFrameGetFieldYalign)
		  ratio(type: float
			default: 1.0
			get: aspectFrameGetFieldRatio)
		  obeyChild(type: boolean
			    default: false
			    get: aspectFrameGetFieldObeyChild)]
	   new: [[label xalign yalign ratio obeyChild]#new]
	   set: [[xalign yalign ratio obeyChild]#set])

      bin:
	 o(%--** assert single child
	   api: gtk
	   isa: container)

      box:
	 o(api: gtk
	   isa: container
	   args: [spacing(type: int
			  default: 0
			  get: generic
			  set: setSpacing)
		  homogeneous(type: boolean
			      default: false
			      get: generic
			      set: setHomogeneous)]
	   add: [pack#(atEnd(type: boolean
			     default: false)|BoxAddOptions)#
		 proc {$ Me Child AtEnd Expand Fill Padding}
		    case AtEnd of 0 then
		       {Me packStart(Child Expand Fill Padding)}
		    [] 1 then
		       {Me packEnd(Child Expand Fill Padding)}
		    end
		 end
		 packStart#BoxAddOptions#packStart
		 packEnd#BoxAddOptions#packEnd])

      button:
	 o(api: gtk
	   isa: bin
	   args: [label(type: string
			get: generic
			set: generic)
		  relief(type: reliefStyle
			 get: getRelief
			 set: setRelief)]
	   signals: [pressed released clicked enter leave]
	   new: [[label]#newWithLabel
		 nil#new])

      buttonBox:
	 o(api: gtk
	   isa: box
	   args: [spacing(type: int
			  get: getSpacing
			  set: setSpacing)
		  layout(type: buttonBoxStyle
			 get: getLayout
			 set: setLayout)
		  minWidth(type: int
			   get: buttonBoxGetFieldChildMinWidth)
		  minHeight(type: int
			    get: buttonBotGetFieldChildMinHeight)
		  ipadX(type: int
			get: buttonBoxGetFieldChildIpadX)
		  ipadY(type: int
			get: buttonGetGetFieldChildIpadY)]
	   set: [[minWidth minHeight]#setChildSize
		 [ipadX ipadY]#setChildIpadding])

      calendar:
	 o(api: gtk
	   isa: widget
	   args: [displayOptions(type: calendarDisplayOptions
				 set: displayOptions)
		  frozen(type: boolean
			 set: proc {$ O I}
				 if I == 1 then {O freeze()}
				 else {O thaw()}
				 end
			      end)
		  day(type: int
		      set: selectDay)
		  month(type: int
			get: calendarGetFieldMonth)
		  year(type: int
		       get: calendarGetFieldYear)]
	   signals: [monthChanged daySelected daySelectedDoubleClick
		     prevMonth nextMonth prevYear nextYear]
	   new: [nil#new]
	   set: [[month year]#
		 proc {$ O Month Year} {O selectMonth(Month Year _)} end])

      checkButton:
	 o(api: gtk
	   isa: toggleButton
	   new: [[label]#newWithLabel
		 nil#new])

      checkMenuItem:
	 o(api: gtk
	   isa: menuItem
	   args: [active(type: boolean
			 set: setActive)
		  showToggle(type: boolean
			     set: setShowToggle)]
	   signals: [toggled]
	   new: [[label]#newWithLabel
		 nil#new])

      cList:
	 o(%--** children: append
	   % columns:
	   %    {get,set}ColumnTitle, {get,set}ColumnWidget,
	   %    columnTitle{Active,Passive},
	   %    setColumnJustification, setColumnVisibility,
	   %    setColumnResizable, setColumnAutoResize,
	   %    setColumn{,Min,Max}Width
	   % rows:
	   %    setForeground, setBackground, rowIsVisible,
	   %    {get,set}RowStyle, {get,set}Selectable
	   % cells:
	   %    setText, setPixmap, setPixtext, getCellType,
	   %    {get,set}CellStyle, setShift
	   % buttons: setButtonActions
	   api: gtk
	   isa: container
	   args: [nColumns(type: int
			   get: generic)
		  titles(type: stringArray)
		  shadowType(type: shadowType
			     get: generic
			     set: setShadowType)
		  selectionMode(type: selectionMode
				get: generic
				set: setSelectionMode)
		  rowHeight(type: int
			    get: generic
			    set: setRowHeight)
		  reorderable(type: boolean
			      get: generic
			      set: setReorderable)
		  titlesActive(type: boolean
			       get: generic
			       set: generic)
		  useDragIcons(type: boolean
			       get: generic
			       set: setUseDragIcons)
		  hadjustment(type: option(object(adjustment))
			      get: getHadjustment
			      set: setHadjustment)
		  vadjustment(type: option(object(adjustment))
			      get: getVadjustment
			      set: setHadjustment)
		  frozen(type: boolean
			 set: proc {$ O I}
				 if I == 1 then {O freeze()}
				 else {O thaw()}
				 end
			      end)
		  showColumnTitles(type: boolean
				   set: proc {$ O I}
					   if I == 1 then
					      {O columnTitlesShow()}
					   else
					      {O columnTitlesHide()}
					   end
					end)
		  activeColumnTitles(type: boolean
				     set: proc {$ O I}
					     if I == 1 then
						{O columnTitlesActive()}
					     else
						{O columnTitlesPassive()}
					     end
					  end)]
	   signals: [selectRow unselectRow rowMove clickColumn resizeColumn
		     toggleFocusRow selectAll unselectAll undoSelection
		     startSelection endSelection toggleAddMode extendSelection
		     scrollVertical scrollHorizontal abortColumnResize]
	   new: [[titles]#
		 fun {$ Class Titles}
		    {New Class newWithTitles({Length Titles} Titles)}
		 end
		 [nColumns]#new])

      colorSelection:
	 o(api: gtk
	   isa: vBox
	   args: [updatePolicy(type: updateType
			       set: setUpdatePolicy)
		  opacity(type: bool
			  set: setOpacity)
		  color(type: float4   %--** unknown type
			get: getColor
			set: setColor)]
	   new: [nil#new])

      colorSelectionDialog:
	 o(%--** colorSelectionDialogGetFieldOkButton,
	   %--** colorSelectionDialogGetFieldCancelButton,
	   %--** colorSelectionDialogGetFieldColorsel,
	   %--** colorSelectionDialogGetFieldHelpButton
	   api: gtk
	   isa: window
	   new: [[title]#new])

      combo:
	 o(%--** children: setItemString
	   api: gtk
	   isa: hBox
	   args: [popdownStrings(type: list(string)
				 set: setPopdownStrings)
		  useArrows(type: boolean
			    set: setUseArrows)
		  useArrowsAlways(type: boolean
				  set: setUseArrowsAlways)
		  caseSensitive(type: boolean
				set: setCaseSensitive)
		  valueInList(type: boolean
			      get: comboGetFieldValueInList)
		  okIfEmpty(type: boolean
			    get: comboGetFieldOkIfEmpty)
		  disableActivate(type: boolean)]
	   new: [nil#new]
	   set: [[valueInList okIfEmpty]#setValueInList])

      container:
	 o(api: gtk
	   isa: widget
	   args: [borderWidth(type: int
			      get: generic
			      set: setBorderWidth)
		  resizeMode(type: resizeMode
			     get: generic
			     set: setResizeMode)]
	   signals: [add remove checkSize focus setFocusChild]
	   add: [add#nil#add])

      %--** cTree

      curve:
	 o(%--** setVector, getVector
	   api: gtk
	   isa: drawingArea
	   args: [curveType(type: curveType
			    set: setCurveType)
		  gamma(type: float
			set: setGamma)
		  minX(type: float)
		  maxX(type: float)
		  minY(type: float)
		  maxY(type: float)]
	   signals: [curveTypeChanged]
	   new: [nil#new]
	   set: [[minX maxX minY maxY]#setRange])

      data:
	 o(api: gtk
	   isa: object
	   signals: [disconnect])

      dialog:
	 o(%--** make all vBox/hBox add methods accessible
	   api: gtk
	   isa: window
	   new: [nil#new]
	   add: [vBox#(atEnd(type: boolean
			     default: false)|BoxAddOptions)#
		 proc {$ Me Child AtEnd Expand Fill Padding}
		    case AtEnd of 0 then
		       {{Me dialogGetFieldVbox($)}
			packStart(Child Expand Fill Padding)}
		    [] 1 then
		       {{Me dialogGetFieldVbox($)}
			packEnd(Child Expand Fill Padding)}
		    end
		 end
		 actionArea#(atEnd(type: boolean
				   default: false)|BoxAddOptions)#
		 proc {$ Me Child AtEnd Expand Fill Padding}
		    case AtEnd of 0 then
		       {{Me dialogGetFieldActionArea($)}
			packStart(Child Expand Fill Padding)}
		    [] 1 then
		       {{Me dialogGetFieldActionArea($)}
			packEnd(Child Expand Fill Padding)}
		    end
		 end])

      drawingArea:
	 o(api: gtk
	   isa: widget
	   args: [width(type: int)
		  height(type: int)]
	   new: [nil#new]
	   set: [[width height]#size])

      editable:
	 o(api: gtk
	   isa: widget
	   args: [textPosition(type: int
			       get: getPosition
			       set: setPosition)
		  editable(type: boolean
			   get: generic
			   set: setEditable)]
	   signals: [changed insertText deleteText activate setEditable
		     moveCursor moveWord movePage moveToRow moveToColumn
		     killChar killWord killLine cutClipboard copyClipboard
		     pasteClipboard])

      entry:
	 o(api: gtk
	   isa: editable
	   args: [maxLength(type: int
			    get: generic
			    set: setMaxLength)
		  visibility(type: boolean
			     get: generic
			     set: setVisibility)
		  text(type: option(string)
		       get: getText
		       set: setText)]
	   new: [[maxLength]#newWithMaxLength
		 nil#new])

      eventBox:
	 o(api: gtk
	   isa: bin
	   new: [nil#new])

      fileSelection:
	 o(%--** fileSelectionGetFieldOkButton,
	   %--** fileSelectionGetFieldCancelButton
	   api: gtk
	   isa: window
	   args: [title(type: string)
		  filename(type: string
			   get: getFilename
			   set: setFilename)
		  complete(type: option(string)
			   set: complete)
		  fileopButtons(type: boolean
				set: proc {$ O I}
					if I == 1
					then {O showFileopButtons()}
					else {O hideFileopButtons()}
					end
				     end)]
	   new: [[title]#new])

      fixed:
	 o(api: gtk
	   isa: container
	   new: [nil#new]
	   add: [put#[x(type: int) y(type: int)]#put])

      fontSelection:
	 o(api: gtk
	   isa: notebook
	   args: [fontName(type: option(string)
			   get: getFontName
			   set: proc {$ O S} {O setFontName(S _)} end)
		  previewText(type: option(string)
			      get: getPreviewText
			      set: setPreviewText)]
	   new: [nil#new])

      fontSelectionDialog:
	 o(api: gtk
	   isa: window
	   args: [title(type: string)
		  fontName(type: option(string)
			   get: getFontName
			   set: proc {$ O S} {O setFontName(S _)} end)
		  previewText(type: option(string)
			      get: getPreviewText
			      set: setPreviewText)]
	   new: [[title]#new])

      frame:
	 o(api: gtk
	   isa: bin
	   args: [label(type: option(string)
			default: unit
			get: generic
			set: setLabel)
		  labelXalign(type: float
			      get: generic
			      set: generic)
		  labelYalign(type: float
			      get: generic
			      set: generic)
		  shadow(type: shadowType
			 get: generic
			 set: setShadowType)]
	   new: [[label]#new]
	   set: [[labelXalign labelYalign]#setLabelAlign])

      gammaCurve:
	 o(api: gtk
	   isa: vBox
	   new: [nil#new])

      handleBox:
	 o(api: gtk
	   isa: bin
	   args: [shadow(type: shadowType
			 get: generic
			 set: setShadowType)
		  handlePosition(type: positionType
				 set: setHandlePosition)
		  snapEdge(type: positionType
			   set: setSnapEdge)]
	   signals: [childAttached childDetached]
	   new: [nil#new])

      hBox:
	 o(api: gtk
	   isa: box
	   new: [[homogeneous spacing]#new])

      hButtonBox:
	 o(api: gtk
	   isa: buttonBox
	   new: [nil#new])

      hPaned:
	 o(api: gtk
	   isa: paned
	   new: [nil#new])

      hRuler:
	 o(api: gtk
	   isa: ruler
	   new: [nil#new])

      hScale:
	 o(api: gtk
	   isa: scale
	   args: [adjustment(type: option(object(adjustment))
			     default: unit
			     get: generic
			     set: generic)]
	   new: [[adjustment]#new])

      hScrollbar:
	 o(api: gtk
	   isa: scrollbar
	   args: [adjustment(type: option(object(adjustment))
			     default: unit
			     get: generic
			     set: generic)]
	   new: [[adjustment]#new])

      hSeparator:
	 o(api: gtk
	   isa: separator
	   new: [nil#new])

      %--** image

      inputDialog:
	 o(api: gtk
	   isa: dialog
	   signals: [enableDevice disableDevice]
	   new: [nil#new])

      item:
	 o(api: gtk
	   isa: bin
	   signals: [select deselect toggle])

      label:
	 o(api: gtk
	   isa: misc
	   args: [label(type: option(string)
			default: unit
			get: generic   %--** looks problematic
			set: setText)
		  pattern(type: option(string)
			  get: getPattern
			  set: setPattern)
		  justify(type: justification
			  get: generic
			  set: setJustify)
		  lineWrap(type: boolean
			   set: setLineWrap)]
	   new: [[label]#new])

      layout:
	 o(api: gtk
	   isa: container
	   args: [hadjustment(type: option(object(adjustment))
			      get: getHadjustment
			      set: setHadjustment)
		  vadjustment(type: option(object(adjustment))
			      get: getHadjustment
			      set: setHadjustment)
		  width(type: int
			get: layoutGetFieldWidth)
		  height(type: int
			 get: layoutGetFieldHeight)
		  frozen(type: boolean
			 set: proc {$ O I}
				 if I == 1 then {O freeze()}
				 else {O thaw()}
				 end
			      end)]
	   new: [[hadjustment vadjustment]#new]
	   set: [[width height]#setSize]
	   add: [put#[x(type: int) y(type: int)]#put])

      list:
	 o(%--** children: appendItems
	   api: gtk
	   isa: container
	   args: [selectionMode(type: selectionMode
				set: setSelectionMode)
		  selection(type: list(object(listItem))
			    get: listGetFieldSelection)]
	   signals: [selectionChanged selectChild unselectChild]
	   new: [nil#new])

      listItem:
	 o(api: gtk
	   isa: item
	   args: [label(type: string)
		  selected(type: boolean
			   set: proc {$ O I}
				   if I == 1 then {O select()}
				   else {O deselect()}
				   end
				end)]
	   signals: [toggleFocusRow selectAll unselectAll undoSelection
		     startSelection endSelection toggleAddMode extendSelection
		     scrollVertical scrollHorizontal]
	   new: [[label]#newWithLabel
		 nil#new])

      menu:
	 o(api: gtk
	   isa: menuShell
	   args: [accelGroup(type: accelGroup
			     get: getAccelGroup
			     set: setAccelGroup)
		  title(type: string
			set: setTitle)
		  tearoffState(type: boolean
			       set: setTearoffState)]
	   new: [nil#new])

      menuBar:
	 o(api: gtk
	   isa: menuShell
	   args: [shadow(type: shadowType
			 get: generic
			 set: setShadowType)]
	   new: [nil#new])

      menuItem:
	 o(api: gtk
	   isa: item
	   args: [label(type: option(string))
		  placement(type: submenuPlacement
			    set: setPlacement)
		  submenuIndicator(type: boolean
				   set: proc {$ O I} {O configure(0 I)} end)
		  rightJustify(type: boolean   %--** cannot be reset
			       set: proc {$ O I}
				       if I == 1 then
					  {O rightJustify()}
				       end
				    end)
		  selected(type: boolean
			   set: proc {$ O I}
				   if I == 1 then {O select()}
				   else {O deselect()}
				   end
				end)]
	   signals: [activate activateItem]
	   new: [[label]#newWithLabel
		 nil#new]
	   add: [submenu#nil#setSubmenu])

      menuShell:
	 o(api: gtk
	   isa: container
	   signals: [deactivate selectionDone moveCurrent activateCurrent
		     cancel]
	   add: [append#nil#append])

      misc:
	 o(api: gtk
	   isa: widget
	   args: [xalign(type: float
			 get: generic
			 set: generic)
		  yalign(type: float
			 get: generic
			 set: generic)
		  xpad(type: int
		       get: generic
		       set: generic)
		  ypad(type: int
		       get: generic
		       set: generic)]
	   set: [[xalign yalign]#setAlignment
		 [xpad ypad]#setPadding])

      notebook:
	 o(api: gtk
	   isa: container
	   args: [page(type: int
		       get: getCurrentPage
		       set: setPage)
		  tabPos(type: positionType
			 get: generic
			 set: setTabPos)
		  tabBorder(type: int
			    get: generic
			    set: setTabBorder)
		  tabHborder(type: int
			     get: generic
			     set: setTabHborder)
		  tabVborder(type: int
			     get: generic
			     set: setTabVborder)
		  showTabs(type: boolean
			   get: generic
			   set: setShowTabs)
		  showBorder(type: boolean
			     get: generic
			     set: setShowBorder)
		  scrollable(type: boolean
			     get: generic
			     set: setScrollable)
		  enablePopup(type: boolean
			      get: generic
			      set: proc {$ O I}
				      if I == 1 then {O popupEnable()}
				      else {O popupDisable()}
				      end
				   end)
		  homogeneousTabs(type: boolean
				  set: setHomogeneousTabs)]
	   signals: [switchPage]
	   new: [nil#new]
	   add: [appendPage#[tabLabel(type: object(widget))
			     menuLabel(type: option(object(widget))
				       default: unit)]#
		 proc {$ Me Child TabLabel MenuLabel}
		    case MenuLabel of unit then
		       {Me appendPage(Child TabLabel)}
		    else
		       {Me appendPageMenu(Child TabLabel MenuLabel)}
		    end
		 end])

      object:
	 o(api: gtk
	   signals: [destroy])

      optionMenu:
	 o(api: gtk
	   isa: button
	   args: [menu(type: object(menu)
		       get: getMenu
		       set: setMenu)
		  history(type: int
			  set: setHistory)]
	   new: [nil#new])

      packer:
	 o(api: gtk
	   isa: container
	   args: [spacing(type: int
			  get: generic
			  set: setSpacing)
		  defaultBorderWidth(type: int
				     get: generic
				     set: setDefaultBorderWidth)
		  defaultPadX(type: int
			      get: generic
			      set: generic)
		  defaultPadY(type: int
			      get: generic
			      set: generic)
		  defaultIpadX(type: int
			       get: generic
			       set: generic)
		  defaultIpadY(type: int
			       get: generic
			       set: generic)]
	   new: [nil#new]
	   set: [[defaultPadX defaultPadY]#setDefaultPad
		 [defaultIpadX defaultIpadY]#setDefaultIpad]
	   add: [add#[side(type: sideType)
		      anchor(type: anchorType
			     default: center)
		      options(type: packerOptions
			      default: [expand fillX fillY])
		      borderWidth(type: int
				  default: 0)
		      padX(type: int
			   default: 0)
		      padY(type: int
			   default: 0)
		      iPadX(type: int
			    default: 0)
		      iPadY(type: int
			    default: 0)]#add])

      paned:
	 o(api: gtk
	   args: [gutterSize(type: int
			     set: setGutterSize)
		  handleSize(type: int
			     set: setHandleSize)
		  position(type: int
			   set: setPosition)]
	   isa: container
	   add: [add1#nil#add1
		 add2#nil#add2
		 pack1#PanedAddOptions#pack1
		 pack2#PanedAddOptions#pack2])

      %--** pixmap

      preview:
	 o(%--** children: put
	   api: gtk
	   isa: widget
	   args: [type(type: previewType)
		  expand(type: boolean
			 set: setExpand)
		  dither(type: rgbDither
			 set: setDither)
		  width(type: int)
		  height(type: int)]
	   new: [[type]#new]
	   set: [[width height]#size])

      progress:
	 o(api: gtk
	   isa: widget
	   args: [activityMode(type: boolean
			       get: generic
			       set: setActivityMode)
		  showText(type: boolean
			   get: generic
			   set: setShowText)
		  textXalign(type: int
			     get: generic
			     set: generic)
		  textYalign(type: int
			     get: generic
			     set: generic)
		  formatString(type: option(string)
			       set: setFormatString)
		  percentage(type: float
			     get: getCurrentPercentage
			     set: setPercentage)
		  value(type: float
			get: getValue
			set: setValue)
		  adjustment(type: option(object(adjustment))
			     set: setAdjustment)
		  text(type: option(string)
		       get: getCurrentText)]
	   set: [[textXalign textYalign]#setTextAlignment])

      progressBar:
	 o(api: gtk
	   isa: progress
	   args: [adjustment(type: option(object(adjustment))
			     get: generic
			     set: setAdjustment)
		  orientation(type: progressBarOrientation
			      get: generic
			      set: setOrientation)
		  barStyle(type: progressBarStyle
			   get: generic
			   set: setBarStyle)
		  activityStep(type: int
			       get: generic
			       set: setActivityStep)
		  activityBlocks(type: int
				 get: generic
				 set: generic)
		  discreteBlocks(type: int
				 get: generic
				 set: setDiscreteBlocks)]
	   new: [[adjustment]#newWithAdjustment
		 nil#new])

      radioButton:
	 o(api: gtk
	   isa: checkButton
	   args: [group(type: option(group(radioButton))
			default: unit
			set: generic)]
	   new: [[group label]#newWithLabel
		 [group]#new])

      radioMenuItem:
	 o(api: gtk
	   isa: checkMenuItem
	   args: [group(type: option(group(radioMenuItem))
			default: unit)]
	   new: [[group label]#newWithLabel
		 [group]#new])

      range:
	 o(api: gtk
	   isa: widget
	   args: [updatePolicy(type: updateType
			       get: generic
			       set: generic)
		  adjustment(type: option(object(adjustment))
			     set: setAdjustment)])

      ruler:
	 o(api: gtk
	   isa: widget
	   args: [metric(type: metricType
			 set: setMetric)
		  lower(type: float
			get: rulerGetFieldLower)
		  upper(type: float
			get: rulerGetFieldUpper)
		  position(type: float
			   get: rulerGetFieldPosition)
		  maxSize(type: float
			  get: rulerGetFieldMaxSize)]
	   set: [[lower upper position maxSize]#setRange])

      scale:
	 o(api: gtk
	   isa: range
	   args: [digits(type: int
			 get: generic
			 set: setDigits)
		  drawValue(type: boolean
			    get: generic
			    set: setDrawValue)
		  valuePos(type: positionType
			   get: generic
			   set: setValuePos)])

      scrollbar:
	 o(api: gtk
	   isa: range)

      scrolledWindow:
	 o(api: gtk
	   isa: bin
	   args: [hadjustment(type: option(object(adjustment))
			      default: unit
			      get: getHadjustment
			      set: setHadjustment)
		  vadjustment(type: option(object(adjustment))
			      default: unit
			      get: getVadjustment
			      set: setVadjustment)
		  hscrollbarPolicy(type: policyType
				   get: generic
				   set: generic)
		  vscrollbarPolicy(type: policyType
				   get: generic
				   set: generic)
		  windowPlacement(type: cornerType
				  get: generic
				  set: setPlacement)]
	   new: [[hadjustment vadjustment]#new]
	   set: [[hscrollbarPolicy vscrollbarPolicy]#setPolicy]
	   add: [addWithViewport#nil#addWithViewport])

      separator:
	 o(api: gtk
	   isa: widget)

      spinButton:
	 o(api: gtk
	   isa: entry
	   args: [adjustment(type: option(object(adjustment))
			     default: unit
			     get: getAdjustment
			     set: setAdjustment)
		  climbRate(type: float
			    default: 1.0
			    get: generic
			    set: generic)
		  digits(type: int
			 get: generic
			 set: setDigits)
		  snapToTicks(type: boolean
			      get: generic
			      set: setSnapToTicks)
		  numeric(type: boolean
			  get: generic
			  set: setNumeric)
		  wrap(type: boolean
		       get: generic
		       set: setWrap)
		  updatePolicy(type: spinButtonUpdatePolicy
			       get: generic
			       set: setUpdatePolicy)
		  shadowType(type: shadowType
			     get: generic
			     set: setShadowType)
		  value(type: float
			get: getValueAsFloat
			set: setValue)]
	   new: [[adjustment climbRate digits]#new])

      statusbar:
	 o(api: gtk
	   isa: hBox
	   new: [nil#new])

      table:
	 o(%--** setRowSpacing, setColSpacing
	   api: gtk
	   isa: container
	   args: [nRows(type: int
			get: generic
			set: generic)
		  nColumns(type: int
			   get: generic
			   set: generic)
		  rowSpacing(type: int
			     get: generic
			     set: setRowSpacings)
		  columnSpacing(type: int
				get: generic
				set: setColSpacings)
		  homogeneous(type: boolean
			      default: false
			      get: generic
			      set: setHomogeneous)]
	   new: [[nRows nColumns homogeneous]#new]
	   set: [[nRows nColumns]#resize]
	   add: [attach#[left(type: int)
			 right(type: int default: unit)
			 top(type: int)
			 bottom(type: int default: unit)
			 xoptions(type: attachOptions
				  default: [expand fill])
			 yoptions(type: attachOptions
				  default: [expand fill])
			 xpadding(type: int
				  default: 0)
			 ypadding(type: int
				  default: 0)]#
		 proc {$ Me Child L R T B XO YO XP YP}
		    {Me attach(Child
			       L case R of unit then L + 1 else R end
			       T case B of unit then T + 1 else B end
			       XO YO XP YP)}
		 end])

      tearoffMenuItem:
	 o(api: gtk
	   isa: menuItem
	   new: [nil#new])

      text:
	 o(api: gtk
	   isa: editable
	   args: [hadjustment(type: option(object(adjustment))
			      default: unit
			      get: generic)
		  vadjustment(type: option(object(adjustment))
			      default: unit
			      get: generic)
		  lineWrap(type: boolean
			   get: generic
			   set: setLineWrap)
		  wordWrap(type: boolean
			   get: generic
			   set: setWordWrap)
		  editable(type: boolean
			   set: setEditable)
		  point(type: int
			get: getPoint
			set: setPoint)
		  length(type: int
			 get: getLength)
		  frozen(type: boolean
			 set: proc {$ O I}
				 if I == 1 then {O freeze()}
				 else {O thaw()}
				 end
			      end)]
	   new: [[hadjustment vadjustment]#new]
	   set: [[hadjustment vadjustment]#setAdjustments])

      tipsQuery:
	 o(api: gtk
	   isa: label
	   args: [emitAlways(type: boolean
			     get: generic
			     set: generic)
		  caller(type: object(widget)
			 get: generic
			 set: setCaller)
		  labelInactive(type: string
				get: generic
				set: generic)
		  labelNoTip(type: string
			     get: generic
			     set: generic)]
	   signals: [startQuery stopQuery widgetEntered widgetSelected]
	   new: [nil#new]
	   set: [[labelInactive labelNoTip]#setLabels])

      toggleButton:
	 o(api: gtk
	   isa: button
	   args: [active(type: boolean
			 get: getActive
			 set: setActive)
		  drawIndicator(type: boolean
				get: generic
				set: setMode)]
	   signals: [toggled]
	   new: [[label]#newWithLabel
		 nil#new])

      toolbar:
	 o(%--** children: appendItem, appendSpace, appendElement, appendWidget
	   api: gtk
	   isa: container
	   args: [orientation(type: orientation
			      set: setOrientation)
		  style(type: toolbarStyle
			set: setStyle)
		  spaceSize(type: int
			    set: setSpaceSize)
		  spaceStyle(type: toolbarSpaceStyle
			     set: setSpaceStyle)
		  tooltips(type: boolean
			   set: setTooltips)
		  buttonRelief(type: reliefStyle
			       get: getButtonRelief
			       set: setButtonRelief)]
	   signals: [orientationChanged styleChanged]
	   new: [[orientation]#new])

      tooltips:
	 o(api: gtk
	   isa: data
	   args: [enabled(type: boolean
			  set: proc {$ O I}
				  if I == 1 then {O enable()}
				  else {O disable()}
				  end
			       end)
		  delay(type: int
			set: setDelay)
		  widget(type: object(widget))
		  tipText(type: string)
		  tipPrivate(type: string)
		  background(type: color
			     get: tooltipsGetFieldBackground)
		  foreground(type: color
			     get: tooltipsGetFieldForeground)]
	   new: [nil#new]
	   set: [[background foreground]#setColors
		 [widget tipText tipPrivate]#setTip])

      tree:
	 o(api: gtk
	   isa: container
	   args: [selectionMode(type: selectionMode
				set: setSelectionMode)
		  viewMode(type: treeViewMode
			   set: setViewMode)
		  viewLines(type: boolean
			    set: setViewLines)]
	   signals: [selectionChanged selectChild unselectChild]
	   new: [nil#new]
	   add: [append#nil#append])

      treeItem:
	 o(api: gtk
	   isa: item
	   args: [label(type: string)
		  subtree(type: object(widget)
			  set: setSubtree)
		  selected(type: boolean
			   set: proc {$ O I}
				   if I == 1 then {O select()}
				   else {O deselect()}
				   end
				end)]
	   signals: [collapse expand]
	   new: [[label]#newWithLabel
		 nil#new])

      vButtonBox:
	 o(api: gtk
	   isa: buttonBox
	   new: [nil#new])

      vBox:
	 o(api: gtk
	   isa: box
	   new: [[homogeneous spacing]#new])

      viewport:
	 o(api: gtk
	   isa: bin
	   args: [hadjustment(type: option(object(adjustment))
			      default: unit
			      get: getHadjustment
			      set: setHadjustment)
		  vadjustment(type: option(object(adjustment))
			      default: unit
			      get: getVadjustment
			      set: setVadjustment)
		  shadowType(type: shadowType
			     get: generic
			     set: setShadowType)]
	   new: [[hadjustment vadjustment]#new])

      vPaned:
	 o(api: gtk
	   isa: paned
	   new: [nil#new])

      vRuler:
	 o(api: gtk
	   isa: ruler
	   new: [nil#new])

      vScale:
	 o(api: gtk
	   isa: scale
	   args: [adjustment(type: option(object(adjustment))
			     default: unit
			     get: generic
			     set: generic)]
	   new: [[adjustment]#new])

      vScrollbar:
	 o(api: gtk
	   isa: scrollbar
	   args: [adjustment(type: option(object(adjustment))
			     default: unit
			     get: generic
			     set: generic)]
	   new: [[adjustment]#new])

      vSeparator:
	 o(api: gtk
	   isa: separator
	   new: [nil#new])

      widget:
	 o(api: gtk
	   isa: object
	   args: [name(type: string
		       get: getName
		       set: setName)
		  sensitive(type: boolean
			    get: generic
			    set: setSensitive)
		  colormap(type: colormap
			   get: getColormap
			   set: setColormap)
		  usizeX(type: int
			 set: proc {$ O I} {O setUsize(I ~1)} end)
		  usizeY(type: int
			 set: proc {$ O I} {O setUsize(~1 I)} end)]
	   signals: [show hide map unmap realize unrealize draw drawFocus
		     drawDefault sizeRequest sizeAllocate stateChanged
		     parentSet addAccelerator removeAccelerator grabFocus event
		     buttonPressEvent buttonReleaseEvent motionNotifyEvent
		     deleteEvent destroyEvent exposeEvent keyPressEvent
		     keyReleaseEvent enterNotifyEvent leaveNotifyEvent
		     configureEvent focusInEvent focusOutEvent mapEvent
		     unmapEvent propertyNotifyEvent selectionClearEvent
		     selectionNotifyEvent selectionGet selectionReceived
		     proximityInEvent proximityOutEvent dragBegin dragEnd
		     dragDataDelete dragLeave dragMotion dragDrop dragDataGet
		     dragDataReceived clientEvent noExposeEvent
		     visibilityNotifyEvent debugMsg]
	   set: [[usizeX usizeY]#setUsize])

      window:
	 o(api: gtk
	   isa: bin
	   args: [type(type: windowType
		       default: toplevel
		       get: generic
		       set: generic)
		  title(type: string
			get: generic
			set: setTitle)
		  autoShrink(type: boolean
			     get: generic
			     set: generic)
		  allowShrink(type: boolean
			      get: generic
			      set: generic)
		  allowGrow(type: boolean
			    get: generic
			    set: generic)
		  modal(type: boolean
			get: generic
			set: setModal)
		  windowPosition(type: windowPosition
				 set: setPosition)
		  wmclassName(type: string
			      get: windowGetFieldWmclassName)
		  wmclassClass(type: string
			       get: windowGetFieldWmclassClass)
		  focus(type: object(widget)
			set: setFocus)
		  userResizable(type: boolean
				set: proc {$ O I}
					if I == 1 then {O setPolicy(0 1 0)}
					else {O setPolicy(0 0 1)}
					end
				     end)
		  accelGroup(type: accelGroup
			     set: addAccelGroup)]
	   new: [[type]#new]
	   set: [[wmclassName wmclassClass]#setWmclass])

      %% GTKCANVAS

      canvas:
	 %--** there's a get for more than one parameter: getScrollRegion
	 o(api: gtkcanvas
	   isa: layout
	   args: [imageSupport(type: bool
			       default: true)
		  scrollRegionLeftmost(type: float)
		  scrollRegionUpper(type: float)
		  scrollRegionRightmost(type: float)
		  scrollRegionLower(type: float)
		  pixelsPerUnit(type: float
				set: setPixelsPerUnit)
		  dither(type: rgbDither
			 set: setDither
			 get: getDither)]
	   new: [[imageSupport]#new]
	   set: [[scrollRegionLeftmost scrollRegionUpper
		  scrollRegionRightmost scrollRegionLower]#setScrollRegion]))
end
