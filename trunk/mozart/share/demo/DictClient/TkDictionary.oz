%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
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
   Tk(toplevel action frame label entry listbox scrollbar addYScrollbar
      button text send batch return returnInt isColor)
   TkTools(images menubar error dialog)
   Open(file)
   NetDictionary('class'
		 defaultServer: DEFAULT_SERVER
		 defaultPort: DEFAULT_PORT)
export
   'class': TkDictionary
require
   DemoUrls(image) at '../DemoUrls.ozf'
prepare
   FixedFont = '8x13'
   BoldFixedFont = '8x13bold'
   BoldFont = '-*-helvetica-bold-r-normal--*-120-*-*-*-*-*-*'
   NormalFont = '-*-helvetica-medium-r-normal--*-120-*-*-*-*-*-*'

   ServerWidth = 20
   IPad = 4
   TextBackground = c(239 239 239)
   WordWidth = 42
   ButtonPad = 10
   ListHeight = 8
   ScrollBorder = 1
   ScrollWidth = 12
   LogWidth = 40
   LogHeight = 4
   Pad = 0

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
define
   Images = {TkTools.images [DemoUrls.image#'dict-client/dict.gif']}

   proc {SetMinsize W}
      {Tk.send update(idletasks)}
      {Tk.send wm(minsize W
		  {Tk.returnInt winfo(reqwidth W)}
		  {Tk.returnInt winfo(reqheight W)})}
   end

   %%
   %% Dialog to Enter a Server Address and Port
   %%

   class ServerDialog
      meth init(Master Server Port Connect)
	 proc {DoConnect} Server Port in
	    {ServerEntry tkReturn(get ?Server)}
	    {PortEntry tkReturn(get ?Port)}
	    if {String.isInt Port} then
	       {Toplevel tkClose()}
	       {Connect Server {String.toInt Port}}
	    else
	       {New TkTools.error
		tkInit(master: Toplevel
		       text: 'The port must be given as a number.') _}
	    end
	 end
	 Toplevel = {New Tk.toplevel tkInit(parent: Master
					    title: 'Choose Server'
					    'class': 'OzTools'
					    highlightthickness: 0
					    withdraw: true)}
	 {Toplevel tkBind(event: '<Escape>'
			  action: Toplevel#tkClose())}
	 Frame1 = {New Tk.frame tkInit(parent: Toplevel
				       highlightthickness: 0
				       borderwidth: 1
				       relief: raised)}
	 ServerLabel = {New Tk.label tkInit(parent: Frame1
					    font: BoldFont
					    text: 'Server: ')}
	 ServerEntry = {New Tk.entry tkInit(parent: Frame1
					    font: FixedFont
					    width: ServerWidth
					    background: TextBackground
					    borderwidth: 1)}
	 {ServerEntry tkBind(event: '<Return>'
			     action: DoConnect)}
	 {ServerEntry tk(insert 'end' case Server of unit then DEFAULT_SERVER
				      else Server
				      end)}
	 PortLabel = {New Tk.label tkInit(parent: Frame1
					  font: BoldFont
					  text: 'Port: ')}
	 PortEntry = {New Tk.entry tkInit(parent: Frame1
					  font: FixedFont
					  width: ServerWidth
					  background: TextBackground
					  borderwidth: 1)}
	 {PortEntry tkBind(event: '<Return>'
			   action: DoConnect)}
	 {PortEntry tk(insert 'end' case Port of unit then DEFAULT_PORT
				    else Port
				    end)}
	 Frame2 = {New Tk.frame tkInit(parent: Toplevel
				       highlightthickness: 0
				       borderwidth: 1
				       relief: raised)}
	 ConnectButton = {New Tk.button tkInit(parent: Frame2
					       text: 'Connect'
					       action: DoConnect)}
	 CloseButton = {New Tk.button tkInit(parent: Frame2
					     text: 'Cancel'
					     action: Toplevel#tkClose())}
      in
	 {Tk.batch [pack(Frame1 side: top expand: true fill: both
			 ipadx: IPad ipady: IPad)
		    pack(Frame2 side: top expand: true fill: both
			 ipadx: IPad ipady: IPad)
		    grid(ServerLabel row: 0 column: 0 sticky: w)
		    grid(ServerEntry row: 0 column: 1)
		    grid(PortLabel row: 1 column: 0 sticky: w)
		    grid(PortEntry row: 1 column: 1)
		    pack(ConnectButton CloseButton side: left expand: true)
		    focus(ServerEntry)]}
	 {SetMinsize Toplevel}
	 {Tk.send wm(deiconify Toplevel)}
      end
   end

   %%
   %% A Simple Information Display Window
   %%

   class InformationWindow
      feat toplevel text status
      attr Withdrawn: true
      meth init(Master Title cursor: Cursor <= xterm status: Status <= false)
	 self.toplevel = {New Tk.toplevel tkInit(parent: Master
						 title: Title
						 'class': 'OzTools'
						 highlightthickness: 0
						 withdraw: true)}
	 Menu = {TkTools.menubar self.toplevel self.toplevel
		 [menubutton(text: 'File'
			     feature: file
			     menu: [command(label: 'Save as ...'
					    action: self#SaveAs())
				    separator
				    command(label: 'Close window'
					    key: ctrl(x)
					    action: self#close())])
		  menubutton(text: 'Edit'
			     feature: edit
			     menu: [command(label: 'Select all'
					    action: self#SelectAll())])]
		 nil}
	 self.text = {New Tk.text tkInit(parent: self.toplevel
					 cursor: Cursor
					 font: FixedFont
					 background: TextBackground
					 state: disabled)}
	 {self.text tk(tag configure titleTag font: BoldFixedFont)}
	 Scrollbar = {New Tk.scrollbar tkInit(parent: self.toplevel
					      borderwidth: ScrollBorder
					      width: ScrollWidth)}
	 {Tk.addYScrollbar self.text Scrollbar}
	 if Status then
	    self.status = {New Tk.text tkInit(parent: self.toplevel
					      cursor: left_ptr
					      border: 0
					      wrap: none
					      font: NormalFont
					      width: 0
					      height: 1
					      state: disabled)}
	 else
	    self.status = unit
	 end
      in
	 {Tk.batch
	  grid(columnconfigure self.toplevel 0 weight: 1)|
	  grid(rowconfigure self.toplevel 1 weight: 1)|
	  grid(Menu row: 0 column: 0 columnspan: 2 sticky: nsew)|
	  grid(self.text row: 1 column: 0 sticky: nsew)|
	  grid(Scrollbar row: 1 column: 1 sticky: nsew)|
	  if Status then
	     [grid(self.status row: 2 column: 0 columnspan: 2 sticky: nsew)]
	  else nil
	  end}
	 {SetMinsize self.toplevel}
      end
      meth append(VS Tag <= unit)
	 try
	    {Tk.batch [o(self.text configure state: normal)
		       case Tag of unit then o(self.text insert 'end' VS)
		       else o(self.text insert 'end' VS Tag)
		       end
		       o(self.text configure state: disabled)]}
	    if @Withdrawn then
	       {Tk.send wm(deiconify self.toplevel)}
	       Withdrawn <- false
	    end
	 catch _ then skip   % window already closed
	 end
      end
      meth status(VS)
	 if @Withdrawn then
	    {Tk.send wm(deiconify self.toplevel)}
	    Withdrawn <- false
	 end
	 {Tk.batch [o(self.status configure state: normal)
		    o(self.status delete p(1 0) 'end')
		    o(self.status insert 'end' VS)
		    o(self.status configure state: disabled)]}
      end
      meth close()
	 {self.toplevel tkClose()}
      end
      meth SaveAs() FileName in
	 FileName =
	 {Tk.return tk_getSaveFile(parent: self.toplevel
				   title: 'Save Text'
				   filetypes: q(q('All Files' '*')))}
	 if FileName == "" then skip
	 else File in
	    File = {New Open.file init(name: FileName
				       flags: [write create truncate])}
	    {File write(vs: {self.text tkReturn(get p(1 0) 'end' $)})}
	    {File close()}
	 end
      end
      meth SelectAll()
	 {self.text tk(tag add 'sel' p(1 0) 'end')}
      end
   end

   %%
   %% A Window to Display Definitions
   %%

   class DefinitionWindow from InformationWindow
      attr tagNumber inTag tagText
      feat action
      meth init(Master Action)
	 self.action = Action
	 tagNumber <- 0
	 inTag <- 0
	 tagText <- ""
	 InformationWindow, init(Master 'Definitions' status: true)
      end
      meth append(Definition)
	 case Definition
	 of definition(word: Word db: _ dbname: DBName body: Body) then
	    InformationWindow, append(Word#', '#DBName#'\n' titleTag)
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
	 if @inTag > 0 then
	    InformationWindow, append(S1 @tagNumber)
	    tagText <- @tagText#S1
	 else
	    InformationWindow, append(S1)
	 end
	 case S2 of C|Cr then
	    case C of &{ then
	       inTag <- @inTag + 1
	       {self.text tk(tag configure @tagNumber underline: true)}
	       if Tk.isColor then
		  {self.text tk(tag configure @tagNumber foreground: blue)}
	       end
	    [] &} then
	       inTag <- {Max @inTag - 1 0}
	       if @inTag == 0 then Text Action in
		  Text = {Clean {VirtualString.toString (tagText <- "")}}
		  Action = {New Tk.action
			    tkInit(parent: self.text
				   action: proc {$} {self.action Text} end)}
		  {self.text tk(tag bind @tagNumber '<1>' Action)}
		  tagText <- ""
		  tagNumber <- @tagNumber + 1
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
      feat action
      attr TagIndex: 0
      meth init(Master Action)
	 self.action = Action
	 InformationWindow, init(Master 'Matches'
				 cursor: left_ptr status: true)
      end
      meth append(Match Databases)
	 case Match of DB#Word then N Action in
	    N = @TagIndex + 1
	    TagIndex <- N
	    InformationWindow, append(Word#', '#{CondSelect Databases
						 {String.toAtom DB} DB}#'\n' N)
	    Action = {New Tk.action
		      tkInit(parent: self.text
			     action: proc {$} DBs in
					DBs = [{String.toAtom DB}]
					{self.action Word DBs}
				     end)}
	    {self.text tk(tag bind N '<1>' Action)}
	 end
      end
   end

   %%
   %% The Main Interaction Window
   %%

   class TkDictionary
      feat
	 closed
	 Toplevel WordEntry
	 DatabasesList DatabaseIndices StrategiesList StrategyIndices
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

	 self.Toplevel = {New Tk.toplevel
			  tkInit(title: 'Dictionary Client'
				 'class': 'OzTools'
				 delete: P#close()
				 withdraw: true)}

	 %% Menubar
	 Menu = {TkTools.menubar self.Toplevel self.Toplevel
		 [menubutton(text: 'Server'
			     menu: [command(label: 'Open ...'
					    key: ctrl(o)
					    action: P#ServerOpen(NetPort))
				    separator
				    command(label: 'Status'
					    key: ctrl(s)
					    action: P#ServerStatus(NetPort))
				    command(label: 'Information ...'
					    action: P#ServerInfo(NetPort))
				    separator
				    command(label: 'Close'
					    key: ctrl(x)
					    action: P#close())])
		  menubutton(text: 'Database'
			     menu: [command(label: 'Information ...'
					    action: P#ShowInfo(NetPort))])]
		 [menubutton(text: 'Help'
			     menu: [command(label: 'About ...'
					    action: P#About())])]}


	 %% Frames
	 Frame1 Frame2L Frame2R Frame3 Frame4
	 {ForAll [Frame1 Frame2L Frame2R Frame3 Frame4]
	  fun {$}
	     {New Tk.frame tkInit(parent: self.Toplevel
				  highlightthickness: 0
				  borderwidth: 1
				  relief: raised)}
	  end}

	 %% Contents of Frame1
	 WordLabel = {New Tk.label tkInit(parent: Frame1
					  font: BoldFont
					  text: 'Word')}
	 self.WordEntry = {New Tk.entry tkInit(parent: Frame1
					       font: FixedFont
					       width: WordWidth
					       background: TextBackground
					       borderwidth: 1)}
	 {self.WordEntry tkBind(event: '<Return>'
				action: P#GetDefinitions(NetPort))}
	 {self.WordEntry tkBind(event: '<Meta-Return>'
				action: P#GetMatches(NetPort))}
	 WordButtonsFrame = {New Tk.frame tkInit(parent: Frame1
						 highlightthickness: 0)}
	 DefineButton = {New Tk.button
			 tkInit(parent: WordButtonsFrame
				text: 'Lookup'
				action: P#GetDefinitions(NetPort))}
	 MatchButton = {New Tk.button
			tkInit(parent: WordButtonsFrame
			       text: 'Match'
			       action: P#GetMatches(NetPort))}

	 %% Contents of Frame2L
	 DatabasesLabel = {New Tk.label tkInit(parent: Frame2L
					       font: BoldFont
					       text: 'Databases')}
	 DatabasesListFrame = {New Tk.frame tkInit(parent: Frame2L
						   highlightthickness: 0)}
	 self.DatabasesList = {New Tk.listbox
			       tkInit(parent: DatabasesListFrame
				      selectmode: extended
				      exportselection: false
				      background: TextBackground
				      height: ListHeight
				      borderwidth: 1)}
	 self.DatabaseIndices = {NewDictionary}
	 DatabasesScrollbar = {New Tk.scrollbar
			       tkInit(parent: DatabasesListFrame
				      borderwidth: ScrollBorder
				      width: ScrollWidth)}
	 {Tk.addYScrollbar self.DatabasesList DatabasesScrollbar}
	 UpdateDatabasesButton = {New Tk.button
				  tkInit(parent: Frame2L
					 text: 'Update List'
					 action: P#UpdateDatabases(NetPort))}

	 %% Contents of Frame2R
	 StrategiesLabel = {New Tk.label tkInit(parent: Frame2R
						font: BoldFont
						text: 'Strategies')}
	 StrategiesListFrame = {New Tk.frame tkInit(parent: Frame2R
						    highlightthicknes: 0)}
	 self.StrategiesList = {New Tk.listbox
				tkInit(parent: StrategiesListFrame
				       selectmode: browse
				       exportselection: false
				       background: TextBackground
				       height: ListHeight
				       borderwidth: 1)}
	 self.StrategyIndices = {NewDictionary}
	 StrategiesScrollbar = {New Tk.scrollbar
				tkInit(parent: StrategiesListFrame
				       borderwidth: ScrollBorder
				       width: ScrollWidth)}
	 {Tk.addYScrollbar self.StrategiesList StrategiesScrollbar}
	 UpdateStrategiesButton = {New Tk.button
				   tkInit(parent: Frame2R
					  text: 'Update List'
					  action: P#UpdateStrategies(NetPort))}

	 %% Contents of Frame3
	 LogLabel = {New Tk.label tkInit(parent: Frame3
					 font: BoldFont
					 text: 'Log')}
	 LogTextFrame = {New Tk.frame tkInit(parent: Frame3
					     highlightthicknes: 0)}
	 self.LogText = {New Tk.text tkInit(parent: LogTextFrame
					    wrap: word
					    font: FixedFont
					    background: TextBackground
					    width: LogWidth
					    height: LogHeight
					    state: disabled)}
	 LogScrollbar = {New Tk.scrollbar
			 tkInit(parent: LogTextFrame
				borderwidth: ScrollBorder
				width: ScrollWidth)}
	 {Tk.addYScrollbar self.LogText LogScrollbar}

	 %% Contents of Frame4
	 self.StatusText = {New Tk.text tkInit(parent: Frame4
					       cursor: left_ptr
					       border: 0
					       wrap: none
					       font: NormalFont
					       width: 0
					       height: 1
					       state: disabled)}
      in
	 {Tk.batch [grid(columnconfigure self.Toplevel 0 weight: 1)
		    grid(columnconfigure self.Toplevel 1 weight: 1)
		    grid(rowconfigure self.Toplevel 2 weight: 1)
		    grid(rowconfigure self.Toplevel 3 weight: 1)
		    grid(Menu row: 0 column: 0 columnspan: 2 sticky: nsew)
		    grid(Frame1 row: 1 column: 0 columnspan: 2 sticky: nsew)
		    grid(Frame2L row: 2 column: 0 sticky: nsew)
		    grid(Frame2R row: 2 column: 1 sticky: nsew)
		    grid(Frame3 row: 3 column: 0 columnspan: 2 sticky: nsew)
		    grid(Frame4 row: 4 column: 0 columnspan: 2 sticky: nsew)
		    pack(WordLabel padx: Pad pady: Pad side: top)
		    pack(self.WordEntry padx: Pad pady: Pad side: top fill: x)
		    pack(WordButtonsFrame padx: Pad pady: Pad side: top)
		    pack(DefineButton MatchButton side: left padx: ButtonPad)
		    pack(DatabasesLabel padx: Pad pady: Pad side: top)
		    pack(DatabasesListFrame
			 padx: Pad pady: Pad side: top expand: true fill: both)
		    pack(UpdateDatabasesButton padx: Pad pady: Pad side: top)
		    pack(self.DatabasesList
			 side: left expand: true fill: both)
		    pack(DatabasesScrollbar side: left fill: y)
		    pack(StrategiesLabel padx: Pad pady: Pad side: top)
		    pack(StrategiesListFrame
			 padx: Pad pady: Pad side: top expand: true fill: both)
		    pack(UpdateStrategiesButton padx: Pad pady: Pad side: top)
		    pack(self.StrategiesList
			 side: left expand: true fill: both)
		    pack(StrategiesScrollbar side: left fill: y)
		    pack(LogLabel padx: Pad pady: Pad side: top)
		    pack(LogTextFrame
			 padx: Pad pady: Pad side: top expand: true fill: both)
		    pack(self.LogText side: left expand: true fill: both)
		    pack(LogScrollbar side: left fill: y)
		    pack(self.StatusText padx: Pad pady: Pad fill: x)
		    focus(self.WordEntry)]}
	 {SetMinsize self.Toplevel}
	 {Tk.send wm(deiconify self.Toplevel)}
	 self.NetDict = {New NetDictionary.'class' init()}
	 TkDictionary, Connect(NetPort Server Port)
	 thread
	    TkDictionary, NetServe(NetMessages)
	 end
	 thread
	    TkDictionary, Serve(Messages)
	 end
      end
      meth close()
	 try
	    {self.NetDict close()}
	 catch E then
	    TkDictionary, HandleException(E)
	 end
	 {self.Toplevel tkClose()}
	 self.closed = unit
      end

      meth Serve(Ms)
	 case Ms of M|Mr then
	    TkDictionary, M
	    TkDictionary, Serve(Mr)
	 end
      end
      meth ServerOpen(NetPort)
	 {New ServerDialog init(self.Toplevel @CurrentServer @CurrentPort
				proc {$ S P}
				   TkDictionary, Connect(NetPort S P)
				end) _}
      end
      meth Connect(NetPort Server Port)
	 TkDictionary, SetDatabases(DEFAULT_DATABASES)
	 TkDictionary, SetStrategies(DEFAULT_STRATEGIES)
	 TkDictionary, Log('Connect to '#Server#' on port '#Port)
	 {Send NetPort connect(Server Port)}
      end
      meth ServerStatus(NetPort)
	 TkDictionary, Log('Request server status')
	 {Send NetPort serverStatus()}
      end
      meth ServerInfo(NetPort)
	 TkDictionary, Log('Request server information')
	 {Send NetPort serverInfo()}
      end
      meth ShowInfo(NetPort) DBs in
	 DBs = {Filter TkDictionary, SelectedDatabases($)
		fun {$ DB} DB \= '!' andthen DB \= '*' end}
	 if DBs \= nil then
	    TkDictionary, Log('Request information on: '#
			      {FormatDBs DBs @Databases})
	    {Send NetPort showInfo(DBs)}
	 else
	    {New TkTools.error
	     tkInit(master: self.Toplevel
		    text: 'Select a non-generic database first.') _}
	 end
      end
      meth About()
	 Dialog = {New TkTools.dialog tkInit(master: self.Toplevel
					     root: pointer
					     title: 'About Dictionary Client'
					     buttons: ['Ok'#tkClose]
					     default: 1
					     focus: 1
					     pack: false)}
	 Icon = {New Tk.label tkInit(parent: Dialog
				     image: Images.dict)}
	 Title = {New Tk.label tkInit(parent: Dialog
				      text: 'Dictionary Client')}
	 Author = {New Tk.label tkInit(parent: Dialog
				       text: ('Programming Systems Lab\n'#
					      'Universität des Saarlandes\n'#
					      'Contact: Leif Kornstaedt\n'#
					      '<kornstae@ps.uni-sb.de>'))}
      in
	 {Tk.batch [grid(Icon row: 0 column: 0 padx: 4 pady: 4)
		    grid(Title row: 0 column: 1 padx: 4 pady: 4)
		    grid(Author row: 1 column: 0 columnspan: 2
			 padx: 8 pady: 8)]}
	 {Dialog tkPack()}
      end
      meth GetDefinitions(NetPort) Word DBs in
	 {self.WordEntry tkReturn(get ?Word)}
	 TkDictionary, SelectedDatabases(?DBs)
	 if Word \= "" andthen DBs \= nil then
	    TkDictionary, Log('Look up `'#Word#'\' in: '#
			      {FormatDBs DBs @Databases})
	    {Send NetPort getDefinitions(Word DBs NetPort)}
	 end
      end
      meth GetMatches(NetPort) Word DBs Strategy in
	 {self.WordEntry tkReturn(get ?Word)}
	 TkDictionary, SelectedDatabases(?DBs)
	 TkDictionary, SelectedStrategy(?Strategy)
	 if Word \= "" andthen DBs \= nil then
	    TkDictionary, Log('Match `'#Word#'\' in: '#
			      {FormatDBs DBs @Databases}#
			      ' using: '#@Strategies.Strategy)
	    {Send NetPort getMatches(Word DBs Strategy NetPort)}
	 end
      end
      meth Lookup(Word DB NetPort)
	 {Send NetPort getDefinitions(Word [DB] NetPort)}
      end
      meth UpdateDatabases(NetPort)
	 TkDictionary, Log('Update databases')
	 {Send NetPort updateDatabases()}
      end
      meth UpdateStrategies(NetPort)
	 TkDictionary, Log('Update strategies')
	 {Send NetPort updateStrategies()}
      end
      meth Log(VS)
	 {Tk.batch [o(self.LogText configure state: normal)
		    o(self.LogText insert 'end' VS#'\n')
		    o(self.LogText configure state: disabled)
		    o(self.LogText see 'end')]}
      end

      meth NetServe(Ms)
	 case Ms of M|Mr then
	    case M of connect(Server Port) then VS in
	       VS = 'Connecting to '#Server#' on port '#Port#' ...'
	       TkDictionary, Status(VS)
	       try
		  {self.NetDict connect(Server Port)}
		  CurrentServer <- Server
		  CurrentPort <- Port
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] serverStatus() then
	       TkDictionary, Status('Requesting server status ...')
	       try
		  TkDictionary, Status({self.NetDict status($)})
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] serverInfo() then VS in
	       VS = 'Requesting server information ...'
	       TkDictionary, Status(VS)
	       try W in
		  W = {New InformationWindow init(self.Toplevel
						  'Server Information')}
		  {W append({self.NetDict showServer($)})}
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] showInfo(DBs) then VS in
	       VS = ('Request information on: '#{FormatDBs DBs @Databases}#
		     ' ...')
	       TkDictionary, Status(VS)
	       try
		  {ForAll DBs
		   proc {$ DB} W in
		      W = {New InformationWindow init(self.Toplevel
						      'Database Information')}
		      {W append({self.NetDict showInfo(DB $)})}
		   end}
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] getDefinitions(Word DBs NetPort) then VS in
	       VS = ('Looking up `'#Word#'\' in: '#{FormatDBs DBs @Databases}#
		     ' ...')
	       TkDictionary, Status(VS)
	       try T Action W TotalCount in
		  T = {Thread.this}
		  proc {Action Word}
		     TkDictionary, Log('Look up `'#Word#'\' in: '#
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
		     {New TkTools.error
		      tkInit(master: self.Toplevel
			     text: 'No matches for `'#Word#'\' found.') _}
		     {W close()}
		  end
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] getMatches(Word DBs Strategy NetPort) then VS in
	       VS = ('Matching `'#Word#'\' in: '#{FormatDBs DBs @Databases}#
		     ' using: '#@Strategies.Strategy#' ...')
	       TkDictionary, Status(VS)
	       try Action W TotalCount in
		  proc {Action Word DBs}
		     TkDictionary, Log('Look up `'#Word#'\' in: '#
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
		     {New TkTools.error
		      tkInit(master: self.Toplevel
			     text: 'No matches for `'#Word#'\' found.') _}
		     {W close()}
		  end
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] updateDatabases() then VS in
	       VS = 'Requesting database information ...'
	       TkDictionary, Status(VS)
	       try
		  TkDictionary,
		  SetDatabases({Append DEFAULT_DATABASES
				{Map {self.NetDict showDatabases($)}
				 fun {$ DB#DBName}
				    {String.toAtom DB}#DBName
				 end}})
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    [] updateStrategies() then VS in
	       VS = 'Requesting strategy information ...'
	       TkDictionary, Status(VS)
	       try
		  TkDictionary,
		  SetStrategies('.'#'Default'|
				{Map {self.NetDict showStrategies($)}
				 fun {$ Strat#StrategyName}
				    {String.toAtom Strat}#StrategyName
				 end})
		  TkDictionary, Status(VS#' done')
	       catch E then
		  TkDictionary, HandleException(E)
	       end
	    end
	    TkDictionary, NetServe(Mr)
	 end
      end
      meth HandleException(E)
	 case E of system(os(os _ 110 ...) ...) then
	    TkDictionary, Status('Connection timed out')
	 elseof system(os(os _ 111 ...) ...) then
	    TkDictionary, Status('Connection refused')
	 elseof error(netdict(unexpectedResponse _ N Response) ...) then
	    if N == unit orelse N < 500 then
	       {Raise E}
	    else
	       TkDictionary, Status('Server error: '#Response)
	    end
	 elseof error(netdict(serverClosed Reason) ...) then
	    TkDictionary, Status('Connection closed'#
				 case Reason of unit then ""
				 else ': '#Reason
				 end)
	 elseof error(netdict(notConnected) ...) then
	    TkDictionary, Status('Not connected')
	 else
	    {Raise E}
	 end
      end
      meth Status(VS)
	 {Tk.batch [o(self.StatusText configure state: normal)
		    o(self.StatusText delete p(1 0) 'end')
		    o(self.StatusText insert 'end' VS)
		    o(self.StatusText configure state: disabled)]}
      end

      meth SetDatabases(Pairs)
	 Databases <- {List.toRecord databases Pairs}
	 {self.DatabasesList tk(delete 0 'end')}
	 {Dictionary.removeAll self.DatabaseIndices}
	 {List.forAllInd Pairs
	  proc {$ I DB#DatabaseName}
	     {self.DatabasesList tk(insert 'end' DatabaseName)}
	     {Dictionary.put self.DatabaseIndices I - 1 DB}
	  end}
	 {self.DatabasesList tk(selection set 0)}
      end
      meth SelectedDatabases($)
	 {Map {self.DatabasesList tkReturnListInt(curselection $)}
	  fun {$ I} {Dictionary.get self.DatabaseIndices I} end}
      end
      meth SetStrategies(Pairs)
	 Strategies <- {List.toRecord strategies Pairs}
	 {self.StrategiesList tk(delete 0 'end')}
	 {Dictionary.removeAll self.StrategyIndices}
	 {List.forAllInd Pairs
	  proc {$ I Strategy#StrategyName}
	     {self.StrategiesList tk(insert 'end' StrategyName)}
	     {Dictionary.put self.StrategyIndices I - 1 Strategy}
	  end}
	 {self.StrategiesList tk(selection set 0)}
      end
      meth SelectedStrategy($)
	 {Dictionary.get self.StrategyIndices
	  {self.StrategiesList tkReturnListInt(curselection $)}.1}
      end
   end
end
