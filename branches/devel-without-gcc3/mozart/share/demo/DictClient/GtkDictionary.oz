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
   GBuilder(create: Create keyPress: KeyPress)
   Open(file)
   NetDictionary('class'
		 defaultServer: DEFAULT_SERVER
		 defaultPort: DEFAULT_PORT)
export
   'class': GtkDictionary
require
   DemoUrls(image) at '../DemoUrls.ozf'
prepare
   %% The following databases and strategies are always available.
   DEFAULT_DATABASES = ['*'#'All'
			'!'#'First with matches']
   DEFAULT_STRATEGIES = ['.'#'Default'
			 'exact'#'Match words exactly'
			 'prefix'#'Match prefixes']

   fun {FormatDBs DBs DatabaseNames}
      {FoldR DBs
       fun {$ DB In}
	  {CondSelect DatabaseNames DB DB}#
	  case In of unit then ""
	  else ', '#In
	  end
       end unit}
   end

   fun {Clean S}
      {FoldR S
       fun {$ C In}
	  if C =< &  then
	     case In of & |_ then In
	     else & |In
	     end
	  else C|In
	  end
       end nil}
   end

   class RegionManager
      prop locking
      attr Dict Count
      meth init()
	 Dict <- {NewDictionary}
	 Count <- 0
      end
      meth append(Min Max Entry)
	 lock N in
	    N = @Count
	    @Dict.N := Min#Max#Entry
	    Count <- N + 1
	 end
      end
      meth lookup(Key $)
	 lock
	    RegionManager, Lookup(0 @Count - 1 Key $)
	 end
      end
      meth Lookup(Low High Key $)
	 if Low > High then unit
	 else Mid in
	    Mid = (High + Low) div 2
	    case {Dictionary.get @Dict Mid} of Min#Max#Entry then
	       if Key < Min then
		  RegionManager, Lookup(Low Mid - 1 Key $)
	       elseif Key > Max then
		  RegionManager, Lookup(Mid + 1 High Key $)
	       else Entry
	       end
	    end
	 end
      end
   end
define
   MonospaceFont = {Create font(weight: medium
				slant: r
				pointSize: 120
				spacing: m
				registry: iso8859
				encoding: 1)}

   BoldMonospaceFont = {Create font(weight: bold
				    slant: r
				    pointSize: 120
				    spacing: m
				    registry: iso8859
				    encoding: 1)}

   Black = {Create color(red: 0 green: 0 blue: 0)}
   White = {Create color(red: 0xFFFF green: 0xFFFF blue: 0xFFFF)}
   Blue = {Create color(red: 0 green: 0 blue: 0xFFFF)}

   %--** Images = {TkTools.images [DemoUrls.image#'dict-client/dict.gif']}

   proc {ErrorDialog VS} DialogDesc Dialog in
      DialogDesc =
      dialog(title: 'DictClient Error'
	     modal: true
	     userResizable: false
	     handle: Dialog
	     vBox(label(label: VS))
	     actionArea(button(label: 'Close'
			       clicked: Dialog#destroy())))
      {{Create DialogDesc} showAll()}
   end

   %%
   %% Dialog to Enter a Server Address and Port
   %%

   class ServerDialog
      meth init(Master Server Port Connect)
	 ServerEntry PortEntry

	 proc {DoConnect _}
	    Server = {ServerEntry getChars(0 ~1 $)}
	    Port = {PortEntry getChars(0 ~1 $)}
	 in
	    if {String.isInt Port} then
	       {Dialog destroy()}
	       {Connect Server {String.toInt Port}}
	    else
	       {ErrorDialog 'The port must be given as a number.'}
	    end
	 end

	 %% Contents of the vbox:
	 VBoxDesc =
	 table(nRows: 2 nColumns: 2
	       borderWidth: 5
	       attach(left: 0 top: 0
		      label(xalign: 0.0
			    label: 'Server: '))
	       attach(left: 1 top: 0
		      entry(handle: ServerEntry
			    text:
			       case Server of unit then DEFAULT_SERVER
			       else Server
			       end
			    activate: DoConnect))
	       attach(left: 0 top: 1
		      label(xalign: 0.0
			    label: 'Port:'))
	       attach(left: 1 top: 1
		      entry(handle: PortEntry
			    text:
			       case Port of unit then DEFAULT_PORT
			       else Port
			       end
			    activate: DoConnect)))

	 %% Dialog window:
	 DialogDesc =
	 dialog(title: 'Choose Server'
		modal: true
		userResizable: false
		vBox(VBoxDesc)
		actionArea(button(label: 'Connect'
				  clicked: DoConnect))
		actionArea(button(label: 'Cancel'
				  clicked: Dialog#destroy()))
		keyPressEvent:
		   {KeyPress [nil#'\033'#proc {$ _} {Dialog destroy()} end]})

	 Dialog = {Create DialogDesc}
      in
	 {Dialog showAll()}
      end
   end

   %%
   %% A Simple Information Display Window
   %%

   class InformationWindow from RegionManager
      prop locking
      feat toplevel text status Closed
      attr Withdrawn: true
      meth init(Master Title status: Status <= false)
	 RegionManager, init()

	 Table

	 MenuBarDesc =
	 menuBar(
	    append(
	       menuItem(
		  label: 'File'
		  submenu(
		     menu(append(menuItem(label: 'Save as ...'
					  activate: self#SaveAs()))
			  append(menuItem())
			  append(menuItem(label: 'Close window'
					  activate: self#close()))))))
	    append(
	       menuItem(
		  label: 'Edit'
		  submenu(
		     menu(append(menuItem(label: 'Select all'
					  activate: self#SelectAll())))))))

	 proc {ButtonPress _}
	    case RegionManager, lookup({self.text return(textPosition: $)} $)
	    of unit then skip
	    elseof Action then {Action}
	    end
	 end

	 ScrolledWindowDesc =
	 scrolledWindow(hscrollbarPolicy: never
			vscrollbarPolicy: automatic
			add(text(handle: self.text
				 buttonPressEvent: ButtonPress)))

	 WindowDesc =
	 window(type: dialog
		title: Title
		add(table(nRows: 3 nColumns: 1
			  handle: Table
			  attach(left: 0 top: 0
				 yoptions: nil
				 MenuBarDesc)
			  attach(left: 0 top: 1
				 ScrolledWindowDesc))))

	 self.toplevel = {Create WindowDesc}
      in
	 if Status then
	    {Table conf(attach(left: 0 top: 2
			       yoptions: nil
			       label(xalign: 0.0
				     handle: self.status)))}
	 else
	    self.status = unit
	 end
      end
      meth append(VS Style <= normal)
	 lock
	    if {IsFree self.Closed} then
	       {self.text insert(case Style of title then BoldMonospaceFont
				 else MonospaceFont
				 end
				 case Style of link then Blue
				 else Black
				 end
				 White VS ~1)}
	       if @Withdrawn then
		  {self.toplevel showAll()}
		  Withdrawn <- false
	       end
	    end
	 end
      end
      meth status(VS)
	 lock
	    if {IsFree self.Closed} then
	       if @Withdrawn then
		  {self.toplevel showAll()}
		  Withdrawn <- false
	       end
	       {self.status setText(VS)}
	    end
	 end
      end
      meth close()
	 lock
	    if {IsFree self.Closed} then
	       self.Closed = true
	       {self.toplevel destroy()}
	    end
	 end
      end
      meth SaveAs()
	 Dialog = {Create fileSelection(title: 'Save Text')}
      in
	 {{Dialog fileSelectionGetFieldOkButton($)}
	  signalConnect(clicked
			proc {$ _} FileName File in
			   FileName = {Dialog return(filename: $)}
			   {Dialog destroy()}
			   File = {New Open.file
				   init(name: FileName
					flags: [write create truncate])}
			   {File write(vs: {self.text getChars(0 ~1 $)})}
			   {File close()}
			end _)}
	 {{Dialog fileSelectionGetFieldCancelButton($)}
	  signalConnect(clicked proc {$ _} {Dialog destroy()} end _)}
	 {Dialog showAll()}
      end
      meth SelectAll()
	 {self.text selectRegion(0 ~1)}
	 {self.text copyClipboard()}
      end
   end

   %%
   %% A Window to Display Definitions
   %%

   class DefinitionWindow from InformationWindow
      attr LinkDepth LinkStart LinkText
      feat Action
      meth init(Master TheAction)
	 self.Action = TheAction
	 LinkDepth <- 0
	 LinkStart <- 0
	 LinkText <- ""
	 InformationWindow, init(Master 'Definitions' status: true)
      end
      meth append(Definition)
	 case Definition
	 of definition(word: Word db: _ dbname: DBName body: Body) then
	    InformationWindow, append(Word#', '#DBName#'\n' title)
	    DefinitionWindow, AppendLines(nil#'\n'#Body)
	    InformationWindow, append('\n\n')
	 end
      end
      meth AppendLines(VS)
	 case VS of V#'\n'#Rest then V2 in
	    V2 = V#'\n'
	    DefinitionWindow, AppendWithHyperLinks({VirtualString.toString V2})
	    DefinitionWindow, AppendLines(Rest)
	 else
	    DefinitionWindow, AppendWithHyperLinks({VirtualString.toString VS})
	 end
      end
      meth AppendWithHyperLinks(S) S1 S2 in
	 {List.takeDropWhile S fun {$ C} C \= &{ andthen C \= &} end ?S1 ?S2}
	 if @LinkDepth > 0 then
	    InformationWindow, append(S1 link)
	    LinkText <- @LinkText#S1
	 else
	    InformationWindow, append(S1)
	 end
	 case S2 of C|Cr then
	    case C of &{ then
	       if @LinkDepth == 0 then
		  LinkStart <- {self.text return(point: $)}
	       end
	       LinkDepth <- @LinkDepth + 1
	    [] &} then
	       LinkDepth <- {Max @LinkDepth - 1 0}
	       if @LinkDepth == 0 then Text in
		  Text = {Clean {VirtualString.toString (LinkText <- "")}}
		  RegionManager, append(@LinkStart {self.text return(point: $)}
					proc {$} {self.Action Text} end)
	       end
	    end
	    DefinitionWindow, AppendWithHyperLinks(Cr)
	 [] nil then skip
	 end
      end
   end

   %%
   %% A Window to Display Matches
   %%

   class MatchWindow from InformationWindow
      feat Action
      attr TagIndex: 0
      meth init(Master TheAction)
	 self.Action = TheAction
	 InformationWindow, init(Master 'Matches' status: true)
      end
      meth append(Match Databases)
	 case Match of DB#Word then N DBName LinkStart in
	    N = @TagIndex + 1
	    TagIndex <- N
	    DBName = {CondSelect Databases {String.toAtom DB} DB}
	    LinkStart = {self.text return(point: $)}
	    InformationWindow, append(Word#', '#DBName#'\n' link)
	    RegionManager, append(LinkStart {self.text return(point: $)}
				  proc {$}
				     {self.Action Word [{String.toAtom DB}]}
				  end)
	 end
      end
   end

   %%
   %% The Main Interaction Window
   %%

   class GtkDictionary
      feat
	 closed
	 Toplevel WordEntry
	 DatabasesList DatabaseIndices
	 StrategiesList StrategyIndices
	 StatusText LogText
	 NetDict
      attr
	 CurrentServer: unit CurrentPort: unit
	 Databases: unit Strategies: unit
      meth init(Server <= DEFAULT_SERVER Port <= DEFAULT_PORT)
	 NetMessages
	 NetPort = {NewPort NetMessages}
	 Messages
	 P = {NewPort Messages}

	 %% Menu bar
	 MenuBarDesc =
	 menuBar(
	    append(
	       menuItem(
		  label: 'Server'
		  submenu(
		     menu(append(menuItem(label: 'Open ...'
					  %--** key: ctrl(o)
					  activate: P#ServerOpen(NetPort)))
			  append(menuItem())
			  append(menuItem(label: 'Status'
					  %--** key: ctrl(s)
					  activate: P#ServerStatus(NetPort)))
			  append(menuItem(label: 'Information ...'
					  activate: P#ServerInfo(NetPort)))
			  append(menuItem())
			  append(menuItem(label: 'Close'
					  %--** key: ctrl(x)
					  activate: P#close()))))))
	    append(
	       menuItem(
		  label: 'Databases'
		  submenu(
		     menu(append(menuItem(label: 'Information ...'
					  activate: P#ShowInfo(NetPort)))))))
	    append(
	       menuItem(
		  label: 'Help'
		  submenu(
		     menu(append(menuItem(label: 'About ...'
					  activate: P#About())))))))

	 %% Contents of `Word' frame
	 LookupShortcut = {KeyPress [[mod1]#'\r'#(P#GetMatches(NetPort))
				     nil#'\r'#(P#GetDefinitions(NetPort))]}

	 WordFrameDesc =
	 frame(label: 'Word'
	       add(table(nRows: 2 nColumns: 2
			 attach(left: 0 right: 2 top: 0
				yoptions: nil
				entry(handle: self.WordEntry
				      keyPressEvent: LookupShortcut))
			 attach(left: 0 top: 1
				xoptions: nil yoptions: nil
				button(label: 'Lookup'
				       clicked: P#GetDefinitions(NetPort)))
			 attach(left: 1 top: 1
				xoptions: nil yoptions: nil
				button(label: 'Match'
				       clicked: P#GetMatches(NetPort))))))

	 %% Contents of `Databases' frame
	 DatabasesFrameDesc =
	 frame(label: 'Databases'
	       add(table(nRows: 2 nColumns: 1
			 attach(left: 0 top: 0
				scrolledWindow(
				   hscrollbarPolicy: never
				   vscrollbarPolicy: always
				   addWithViewport(
				      list(selectionMode: extended
					   handle: self.DatabasesList))))
			 attach(left: 0 top: 1
				xoptions: nil yoptions: nil
				button(label: 'Update List'
				       clicked: P#UpdateDatabases(NetPort))))))

	 self.DatabaseIndices = {NewDictionary}

	 %% Contents of `Strategies' frame
	 StrategiesFrameDesc =
	 frame(label: 'Strategies'
	       add(table(nRows: 2 nColumns: 1
			 attach(left: 0 top: 0
				scrolledWindow(
				   hscrollbarPolicy: never
				   vscrollbarPolicy: always
				   addWithViewport(
				      list(selectionMode: browse
					   handle: self.StrategiesList))))
			 attach(left: 0 top: 1
				xoptions: nil yoptions: nil
				button(label: 'Update List'
				       clicked:
					  P#UpdateStrategies(NetPort))))))

	 self.StrategyIndices = {NewDictionary}

	 %% Contents of `Log' frame
	 LogFrameDesc =
	 frame(label: 'Log'
	       add(scrolledWindow(vscrollbarPolicy: always
				  hscrollbarPolicy: never
				  add(text(handle: self.LogText)))))

	 %% Contents of `Status' frame
	 StatusLabelDesc = label(xalign: 0.0
				 handle: self.StatusText)

	 %% Toplevel window
	 WindowDesc =
	 window(type: toplevel
		title: 'Dictionary Client'
		destroy: P#close()
		add(table(nRows: 5 nColumns: 2
			  columnSpacing: 5 rowSpacing: 5
			  attach(left: 0 right: 2 top: 0
				 yoptions: nil
				 MenuBarDesc)
			  attach(left: 0 right: 2 top: 1
				 yoptions: nil
				 WordFrameDesc)
			  attach(left: 0 top: 2
				 DatabasesFrameDesc)
			  attach(left: 1 top: 2
				 StrategiesFrameDesc)
			  attach(left: 0 right: 2 top: 3
				 LogFrameDesc)
			  attach(left: 0 right: 2 top: 4
				 yoptions: nil
				 StatusLabelDesc))))

	 self.Toplevel = {Create WindowDesc}
      in
	 {self.Toplevel showAll()}
	 self.NetDict = {New NetDictionary.'class' init()}
	 GtkDictionary, Connect(NetPort Server Port)
	 thread
	    GtkDictionary, NetServe(NetMessages)
	 end
	 thread
	    GtkDictionary, Serve(Messages)
	 end
      end
      meth close()
	 try
	    {self.NetDict close()}
	 catch E then
	    GtkDictionary, HandleException(E)
	 end
	 {self.Toplevel hideAll()}
	 self.closed = unit
      end

      meth Serve(Ms)
	 case Ms of M|Mr then
	    GtkDictionary, M
	    GtkDictionary, Serve(Mr)
	 end
      end
      meth ServerOpen(NetPort)
	 {New ServerDialog init(self.Toplevel @CurrentServer @CurrentPort
				proc {$ S P}
				   GtkDictionary, Connect(NetPort S P)
				end) _}
      end
      meth Connect(NetPort Server Port)
	 GtkDictionary, SetDatabases(DEFAULT_DATABASES)
	 GtkDictionary, SetStrategies(DEFAULT_STRATEGIES)
	 GtkDictionary, Log('Connect to '#Server#' on port '#Port)
	 {Send NetPort connect(Server Port)}
      end
      meth ServerStatus(NetPort)
	 GtkDictionary, Log('Request server status')
	 {Send NetPort serverStatus()}
      end
      meth ServerInfo(NetPort)
	 GtkDictionary, Log('Request server information')
	 {Send NetPort serverInfo()}
      end
      meth ShowInfo(NetPort) DBs in
	 DBs = {Filter GtkDictionary, SelectedDatabases($)
		fun {$ DB} DB \= '!' andthen DB \= '*' end}
	 if DBs \= nil then
	    GtkDictionary, Log('Request information on: '#
			       {FormatDBs DBs @Databases})
	    {Send NetPort showInfo(DBs)}
	 else
	    {ErrorDialog 'Select a non-generic database first.'}
	 end
      end
      meth About()
	 Blurb =
	 vBox(pack(label(label: 'Programming Systems Lab'))
	      pack(label(label: 'Universität des Saarlandes')) %--** ä
	      pack(label(label: 'Contact: Leif Kornstaedt'))
	      pack(label(label: '<kornstae@ps.uni-sb.de>')))

	 %--** Image = {{New GDK.imlib noop()} loadImage('dict.gif' $)}

	 DialogDesc =
	 dialog(title: 'About Dictionary Client'
		modal: true
		userResizable: false
		vBox(table(nRows: 3 nColumns: 2
			   borderWidth: 5 rowSpacing: 5 columnSpacing: 5
			   %--** attach(left: 0 top: 0 Image)
			   attach(left: 1 top: 0
				  label(label: 'Dictionary Client'))
			   attach(left: 0 right: 2 top: 1
				  Blurb)))
		actionArea(button(label: 'Ok'
				  clicked: Dialog#destroy())))
	 Dialog = {Create DialogDesc}
      in
	 {Dialog showAll()}
      end
      meth GetDefinitions(NetPort) Word DBs in
	 {self.WordEntry getChars(0 ~1 ?Word)}
	 GtkDictionary, SelectedDatabases(?DBs)
	 if Word \= "" andthen DBs \= nil then
	    GtkDictionary, Log('Look up `'#Word#'\' in: '#
			       {FormatDBs DBs @Databases})
	    {Send NetPort getDefinitions(Word DBs NetPort)}
	 end
      end
      meth GetMatches(NetPort) Word DBs Strategy in
	 {self.WordEntry getChars(0 ~1 ?Word)}
	 GtkDictionary, SelectedDatabases(?DBs)
	 GtkDictionary, SelectedStrategy(?Strategy)
	 if Word \= "" andthen DBs \= nil then
	    GtkDictionary, Log('Match `'#Word#'\' in: '#
			       {FormatDBs DBs @Databases}#
			       ' using: '#@Strategies.Strategy)
	    {Send NetPort getMatches(Word DBs Strategy NetPort)}
	 end
      end
      meth Lookup(Word DB NetPort)
	 {Send NetPort getDefinitions(Word [DB] NetPort)}
      end
      meth UpdateDatabases(NetPort)
	 GtkDictionary, Log('Update databases')
	 {Send NetPort updateDatabases()}
      end
      meth UpdateStrategies(NetPort)
	 GtkDictionary, Log('Update strategies')
	 {Send NetPort updateStrategies()}
      end
      meth Log(VS)
	 {self.LogText insert(unit unit unit VS#'\n' ~1)}
      end

      meth NetServe(Ms)
	 case Ms of M|Mr then
	    case M of connect(Server Port) then VS in
	       VS = 'Connecting to '#Server#' on port '#Port#' ...'
	       GtkDictionary, Status(VS)
	       try
		  {self.NetDict connect(Server Port)}
		  CurrentServer <- Server
		  CurrentPort <- Port
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] serverStatus() then
	       GtkDictionary, Status('Requesting server status ...')
	       try
		  GtkDictionary, Status({self.NetDict status($)})
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] serverInfo() then VS in
	       VS = 'Requesting server information ...'
	       GtkDictionary, Status(VS)
	       try W in
		  W = {New InformationWindow init(self.Toplevel
						  'Server Information')}
		  {W append({self.NetDict showServer($)})}
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] showInfo(DBs) then VS in
	       VS = ('Request information on: '#{FormatDBs DBs @Databases}#
		     ' ...')
	       GtkDictionary, Status(VS)
	       try
		  {ForAll DBs
		   proc {$ DB} W in
		      W = {New InformationWindow init(self.Toplevel
						      'Database Information')}
		      {W append({self.NetDict showInfo(DB $)})}
		   end}
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] getDefinitions(Word DBs NetPort) then VS in
	       VS = ('Looking up `'#Word#'\' in: '#{FormatDBs DBs @Databases}#
		     ' ...')
	       GtkDictionary, Status(VS)
	       try T Action W TotalCount in
		  T = {Thread.this}
		  proc {Action Word}
		     GtkDictionary, Log('Look up `'#Word#'\' in: '#
					{FormatDBs DBs @Databases})
		     {Send NetPort getDefinitions(Word DBs NetPort)}
		  end
		  W = {New DefinitionWindow init(self.Toplevel Action)}
		  TotalCount = {NewCell 0}
		  {ForAll DBs
		   proc {$ DB} Count Res in
		      thread
			 try
			    {self.NetDict 'define'(Word db: DB
						   count: ?Count ?Res)}
			 catch E then
			    {Thread.injectException T E}
			 end
		      end
		      if Count > 0 then Got ToGet in
			 Got = {Access TotalCount}
			 ToGet = Got + Count
			 {Assign TotalCount ToGet}
			 {W status('Retrieved '#Got#'; found '#ToGet)}
			 {List.forAllInd Res
			  proc {$ I Definition}
			     {W status('Retrieved '#Got + I#'; found '#ToGet)}
			     {W append(Definition)}
			  end}
		      end
		   end}
		  {W status('Total: '#{Access TotalCount})}
		  if {Access TotalCount} == 0 then
		     {ErrorDialog 'No matches for `'#Word#'\' found.'}
		     {W close()}
		  end
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] getMatches(Word DBs Strategy NetPort) then VS in
	       VS = ('Matching `'#Word#'\' in: '#{FormatDBs DBs @Databases}#
		     ' using: '#@Strategies.Strategy#' ...')
	       GtkDictionary, Status(VS)
	       try Action W TotalCount in
		  proc {Action Word DBs}
		     GtkDictionary, Log('Look up `'#Word#'\' in: '#
					{FormatDBs DBs @Databases})
		     {Send NetPort getDefinitions(Word DBs NetPort)}
		  end
		  W = {New MatchWindow init(self.Toplevel Action)}
		  TotalCount = {NewCell 0}
		  {ForAll DBs
		   proc {$ DB} Count Res in
		      {self.NetDict match(Word db: DB strategy: Strategy
					  count: ?Count ?Res)}
		      if Count > 0 then Got ToGet in
			 Got = {Access TotalCount}
			 ToGet = Got + Count
			 {Assign TotalCount ToGet}
			 {W status('Retrieving '#Got#'; found '#ToGet)}
			 {List.forAllInd Res
			  proc {$ I Match}
			     {W status('Retrieved '#Got + I#'; found '#ToGet)}
			     {W append(Match @Databases)}
			  end}
		      end
		   end}
		  {W status('Total: '#{Access TotalCount})}
		  if {Access TotalCount} == 0 then
		     {ErrorDialog 'No matches for `'#Word#'\' found.'}
		     {W close()}
		  end
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] updateDatabases() then VS in
	       VS = 'Requesting database information ...'
	       GtkDictionary, Status(VS)
	       try
		  GtkDictionary,
		  SetDatabases({Append DEFAULT_DATABASES
				{Map {self.NetDict showDatabases($)}
				 fun {$ DB#DBName}
				    {String.toAtom DB}#DBName
				 end}})
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    [] updateStrategies() then VS in
	       VS = 'Requesting strategy information ...'
	       GtkDictionary, Status(VS)
	       try
		  GtkDictionary,
		  SetStrategies('.'#'Default'|
				{Map {self.NetDict showStrategies($)}
				 fun {$ Strat#StrategyName}
				    {String.toAtom Strat}#StrategyName
				 end})
		  GtkDictionary, Status(VS#' done')
	       catch E then
		  GtkDictionary, HandleException(E)
	       end
	    end
	    GtkDictionary, NetServe(Mr)
	 end
      end
      meth HandleException(E)
	 case E of system(os(os _ 110 ...) ...) then
	    GtkDictionary, Status('Connection timed out')
	 elseof system(os(os _ 111 ...) ...) then
	    GtkDictionary, Status('Connection refused')
	 elseof error(netdict(unexpectedResponse _ N Response) ...) then
	    if N == unit orelse N < 500 then
	       {Raise E}
	    else
	       GtkDictionary, Status('Server error: '#Response)
	    end
	 elseof error(netdict(serverClosed Reason) ...) then
	    GtkDictionary, Status('Connection closed'#
				  case Reason of unit then ""
				  else ': '#Reason
				  end)
	 elseof error(netdict(notConnected) ...) then
	    GtkDictionary, Status('Not connected')
	 else
	    {Raise E}
	 end
      end
      meth Status(VS)
	 {self.StatusText setText(VS)}
      end

      meth SetDatabases(Pairs)
	 Databases <- {List.toRecord databases Pairs}
	 {self.DatabasesList clearItems(0 ~1)}
	 {Dictionary.removeAll self.DatabaseIndices}
	 {self.DatabasesList
	  appendItems({List.mapInd Pairs
		       fun {$ I DB#DatabaseName}
			  {Dictionary.put self.DatabaseIndices I - 1 DB}
			  {Create listItem(label: DatabaseName)}
		       end})}
	 {self.DatabasesList selectItem(0)}
	 {self.DatabasesList showAll()}
      end
      meth SelectedDatabases($)
	 {Map {self.DatabasesList return(selection: $)}
	  fun {$ Item}
	     {Dictionary.get self.DatabaseIndices
	      {self.DatabasesList childPosition(Item $)}}
	  end}
      end
      meth SetStrategies(Pairs)
	 Strategies <- {List.toRecord strategies Pairs}
	 {self.StrategiesList unselectAll()}
	 {self.StrategiesList clearItems(0 ~1)}
	 {Dictionary.removeAll self.StrategyIndices}
	 {self.StrategiesList
	  appendItems({List.mapInd Pairs
		       fun {$ I Strategy#StrategyName}
			  {Dictionary.put self.StrategyIndices I - 1 Strategy}
			  {Create listItem(label: StrategyName)}
		       end})}
	 {self.StrategiesList selectItem(0)}
	 {self.StrategiesList showAll()}
      end
      meth SelectedStrategy($)
	 case {self.StrategiesList return(selection: $)} of [Item] then
	    {Dictionary.get self.StrategyIndices
	     {self.StrategiesList childPosition(Item $)}}
	 end
      end
   end
end
