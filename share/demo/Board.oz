%%%
%%% Authors:
%%%   Michael Mehl (mehl@dfki.de)
%%%   Ralf Scheidhauer (scheidhr@dfki.de)
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%   Gert Smolka (smolka@dfki.de)
%%%
%%% Copyright:
%%%   Michael Mehl, 1997
%%%   Ralf Scheidhauer, 1997
%%%   Christian Schulte, 1997, 1998
%%%   Gert Smolka, 1997
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

   PickleCompressionLevel = 9
   
   local
      InitServer = {NewName}
   in
      class Server
	 attr
	    port close serve
	    
	 meth !InitServer(?Port)
	    proc {Serve X|Xs}
	       {@serve X}
	       {Serve Xs}
	    end
	    Stream
	    CloseException = {NewName}
	 in
	    proc {@close}
	       raise CloseException end
	    end
	    {NewPort Stream Port}
	    @port  = Port
	    @serve = self
	    thread
	       try
		  {Serve Stream}
	       catch
		  !CloseException then skip
	       end
	    end
	 end
      end
      
      fun {NewServer Class Init}
	 Port
	 Object = {New Class InitServer(Port)}
      in
	 {Object Init}
	 Port
      end
      
   end
   
   proc {NewAgenda ?Port ?Connect}
      Stream
   in
      {NewPort Stream Port}
      proc {Connect P}
	 thread
	    {ForAll Stream proc {$ M} {Send P M} end}
	 end
      end
   end
   
   fun {NewBoardFunctor IsMaster Ticket User}
      
      ArgSpec = record(master(single type:bool default:IsMaster)
		       ticket(single type:atom default:Ticket)
		       user(single type:atom   default:User))

   in
      
      functor
      
      import
	 Tk
	 TkTools
	 Connection
	 OS
	 Application
	 Pickle
      
      define
   
	 Args = {Application.getCmdArgs ArgSpec}

	 proc {SendApplet FileName Subject To}
	    if
	       {OS.system ('metasend -b -e base64 -f '#FileName#
			   ' -m application/x-oz-application'#
			   ' -s "'#Subject#'" -t '#To)}\=0
	    then
	       raise failed(mail) end
	    end
	 end

	 class Board from Server
	    attr
	       agenda connect
	    
	    meth init
	       {NewAgenda @agenda @connect}
	    end
	 
	    meth newWindow($)
	       Connect = @connect % Don't pass reference to object
	    in
	       proc {$ Tk Desc}
		  {NewServer Window init(Tk
					 {AdjoinAt Desc
					  agenda @agenda}
					 Connect) _}
	       end
	    end
	 
	    meth newTool(Label BClass WClass)
	       T = {NewServer Tool init(BClass @agenda)}
	    in
	       {Send @agenda newTool(Label T WClass)}
	    end
	 
	    meth newUser(UserId Name)
	       {Send @agenda newUser(UserId Name)}
	    end
	 
	    meth deleteUser(UserId)
	       {Send @agenda deleteUser(UserId)}
	    end
	 
	    meth close
	       {Send @agenda shutdown}
	       {@close}
	    end
	 end
      
	 class Tool from Server
	    attr
	       bClass  agenda
	    
	    meth init(BClass Agenda) % invoked by Board
	       @bClass  = BClass
	       @agenda  = Agenda
	    end
	 
	    meth newBObj(X Y) % invoked by Window
	       {NewServer @bClass init(@agenda @port X Y) _}
	    end
	 end
      
	 class BObject
	    from Server
	       %% virtual, needs attributes kind, props, dx, dy
	    attr
	       agenda
	       updating: false
	    
	    meth init(Agenda Tool X Y) % invoked by Tool
	       Message = create(@kind X Y X+@dx Y+@dy
				{List.toRecord o {self GetProps($)}})
	    in
	       @agenda = Agenda
	       {Send @agenda newWObj(@port Tool Message)}
	    end
	 
	    meth requestUpdate(Window) % invoked by Window
	       if @updating then {Send Window rejectRequest}
	       else
		  updating <- true
		  {Send Window grantUpdate({self GetProps($)} @port)}
	       end
	    end
	 
	    meth requestClose(Window) % invoked by Window
	       if @updating then {Send Window rejectRequest}
	       else
		  {Send @agenda closeWObj(@port Window)}
		  {@close}
	       end
	    end
	 
	    meth update(NewProps) % invoked by Dialog
	       {ForAll NewProps proc {$ A#V} A <- V end}
	       {Send @agenda updateWObj(@port NewProps)}
	       updating <- false
	    end
	 
	    meth GetProps($)
	       {Map @props fun {$ A} A#@A end}
	    end
	 end
      
	 class Window from Server
	    attr
	       tk  top  bar  radio  canvas
	       tool page1 page2
	       BObj2WObj:   nil
	       Tool2WClass: nil
	       Name2Label:  nil
	       BGColor:     ivory
	       Busy:        false
	       MyDialog:    nil
	    
	    
	    meth init(Tk Desc ConnectToAgenda) % invoked by Board
	       proc {ActionNewBObj X Y}
		  {Send @port NewBObj(X Y)}
	       end
	       proc {ActionDelete}
		  {Send Desc.agenda deleteUser(Desc.user)} 
		  {Send @port   Close}
		  if {HasFeature Desc close} then
		     {Desc.close}
		  end
	       end
	       Title   = {CondSelect Desc title 'Drawing Board'}
	       TkTools = Tk.tools
	    in
	       @tool   = {NewPort _}
	       @tk     = Tk
	       @top    = {New Tk.toplevel
			  tkInit(title:Title delete:ActionDelete
				 withdraw:true)}
	       @page1  = {New TkTools.textframe tkInit(parent:@top text:'Tools')}
	       @radio  = {New Tk.variable tkInit}
	       @page2  = {New TkTools.textframe tkInit(parent:@top text:'Users')}
	       @canvas = {New Tk.canvas
			  tkInit(parent:@top bd:2 relief:sunken
				 width:300 height:300 bg:@BGColor)}
	       {@canvas tkBind(event:  '<1>'
			       args:   [int(x) int(y)]
			       action: ActionNewBObj)}
	       {Tk.batch [grid(@page1  row:0 column:0 pady:4 sticky:ew)
			  grid(@page2  row:1 column:0 pady:4 sticky:ew)
			  grid(@canvas row:0 column:1 sticky:sn padx:2 pady:2
			       rowspan:4)
			  grid({New Tk.button
				tkInit(parent: @top
				       text:  'Mail Applet'
				       action: Desc.mail)}
			       sticky:ew pady:4
			       row:2 column:0)
			  grid({New Tk.button
				tkInit(parent: @top
				       text:  'Save Applet'
				       action: Desc.save)}
			       sticky:ew pady:4
			       row:3 column:0)
			  update(idletasks)
			  wm(deiconify @top)]}
	       {ConnectToAgenda @port}
	    end
	 
	    meth newTool(Label Tool WClass) % invoked by Board
	       proc {ActionSelectTool}
		  {Send @port SelectTool(Tool)}
	       end
	       C|S    = {VirtualString.toString Label}
	    
	       Button = {New @tk.radiobutton
			 tkInit(parent: @page1.inner
				text:   {Char.toUpper C}|S
				anchor: w
				bg:'#b9b9b9'
				var:    @radio
				value:  Label
				relief: groove
				bd:     2
				action: ActionSelectTool)}
	    in
	       {@tk.send grid(Button sticky:sew padx:2 pady:2)}
	       {self Put(Tool2WClass Tool WClass)}
	    end
	 
	    meth newUser(UserId Name) % invoked by Board
	       Label = {New @tk.label
			tkInit(parent: @page2.inner
			       text:   Name
			       anchor: w
			       bg:     white
			       bd:     2)}
	    in
	       {@tk.send grid(Label sticky:sew padx:2 pady:2)}
	       {self Put(Name2Label UserId Label)}
	    end
	 
	    meth deleteUser(UserId)
	       {{self Get(Name2Label UserId $)} tkClose}
	       {self Delete(Name2Label UserId)}
	    end
	 
	    meth shutdown % invoked by Board
	       if @MyDialog \= nil then {Send @MyDialog close} end
	       {@top tkClose}
	       {@close}
	    end 
	 
	    meth NewBObj(X Y) % invoked by User <1>
	       {Send @tool newBObj(X Y)}
	    end
	 
	    meth updateBObj(BObj) % invoked by User <2>
	       if @Busy then {self Flash(black)} else
		  {self MkBusy}
		  {Send BObj requestUpdate(@port)}
	       end
	    end	 
	 
	    meth closeBObj(BObj) % invoked by User <3>
	       if @Busy then  {self Flash(black)} else
		  {self MkBusy}
		  {Send BObj requestClose(@port)}
	       end
	    end
	 
	    meth SelectTool(Tool) % invoked by User <1>
	       tool <- Tool
	    end
	 
	    meth Close % invoked by User <1>
	       if @Busy then  {self Flash(black)}  else
		  {@top tkClose}
		  {@close}
	       end
	    end
	 
	    meth newWObj(BObj Tool Message) % invoked by BObject through agenda
	       WObj = {New {self Get(Tool2WClass Tool $)}
		       init(@tk @port @canvas BObj Message)}
	    in
	       {self Put(BObj2WObj BObj WObj)}
	    end
	 
	    meth updateWObj(BObj Props) % invoked by BObject through agenda
	       {{self Get(BObj2WObj BObj $)} update(Props)}
	    end
	 
	    meth closeWObj(BObj Window) % invoked by BObject through agenda
	       if Window == @port then {self MkIdle} end
	       {{self Get(BObj2WObj BObj $)} close}
	    end
	 
	    meth grantUpdate(Old BObj) % invoked by BObject
	       MyDialog <- {NewServer Dialog init(@tk @port Old BObj)}
	    end
	 
	    meth rejectRequest % invoked by BObject
	       {self Flash(red)}
	       {self MkIdle}
	    end
	 
	    meth dialogClosed % invoked by Dialog
	       MyDialog <- nil
	       {self MkIdle}
	    end
	 
	    meth Put(A K V)
	       A <- K#V | @A
	    end
	 
	    meth Get(A K $)
	       {LookUp @A K}
	    end
	 
	    meth Delete(A K)
	       A <- {Remove @A K}
	    end
	 
	    meth MkBusy
	       Busy <- true
	       {self ChangeColor(thistle)}
	    end
	 
	    meth MkIdle
	       Busy <- false
	       {self ChangeColor(ivory)}
	    end
	 
	    meth ChangeColor(Color)
	       {@canvas tk(configure(bg:Color))}
	       BGColor <- Color
	    end
	 
	    meth Flash(Color)
	       OldColor = @BGColor
	    in
	       {self ChangeColor(Color)}
	       {Delay 200}
	       {self ChangeColor(OldColor)}
	    end
	 end
      
	 fun {LookUp K#I|KIr GK}
	    if K==GK then I else {LookUp KIr GK} end
	 end
      
	 fun {Remove KIs DK}
	    case KIs of nil then nil
	    [] KI|KIr then K#_=KI in
	       if K==DK then KIr else KI|{Remove KIr DK} end
	    end
	 end
      
	 fun {GetHostName}
	    UTS = {OS.uName}
	 in
	    UTS.nodename
	 end
      
	 class WObject
	    attr
	       canvas tag
	    
	    meth init(Tk Window Canvas BObj Message) % invoked by Window
	       proc {ActionUpdate} {Send Window updateBObj(BObj)} end
	       proc {ActionClose} {Send Window closeBObj(BObj)} end
	    in
	       @canvas = Canvas
	       @tag  = {New Tk.canvasTag tkInit(parent:Canvas)}
	       {Canvas tk(Message o(tag:@tag))}
	       {@tag tkBind(event:'<2>' action: ActionUpdate)}
	       {@tag tkBind(event:'<3>' action: ActionClose)}
	    end
	 
	    meth update(Props) % invoked by Window on request of BObject
	       {@canvas tk({List.toRecord itemconfigure (1#@tag|Props)})}
	    end
	 
	    meth close % invoked by Window on request of BObject
	       {@tag tk(delete)}
	    end
	 end
      
	 class Dialog from Server
	    attr
	       top window entries oldProps bObj
	    
	    meth init(Tk Window OldProps BObj) % invoked by Window
	       proc {ActionOk}
		  {Send @port Ok}
	       end
	       proc {ActionCancel}
		  {Send @port Cancel}
	       end
	       @window  = Window
	       @oldProps= OldProps
	       @bObj    = BObj
	       @top     = {New Tk.toplevel
			   tkInit(title:'Object Attributes' delete:ActionCancel
				  withdraw:true)}
	       {@top tkWM(geometry '+'#{Tk.returnInt winfo(pointerx @top)}+5#
			  '+'#{Tk.returnInt winfo(pointery @top)}+10)}
	       @entries = {List.mapInd @oldProps
			   fun {$ I A#V}
			      L = {New Tk.label
				   tkInit(parent:@top text:A#':' anchor:w)}
			      E = {New Tk.entry
				   tkInit(parent:@top width:10 bg:white)}
			   in
			      {E tk(insert 0 V)}
			      {Tk.send grid(row:I L E sticky:w padx:4 pady:4)}
			      A#E
			   end}
	       OkButton     = {New Tk.button
			       tkInit(parent:@top  text:'Okay' action: ActionOk)}
	    in
	       {Tk.send grid(row:{Length @entries}+1 columnspan:2 pady:10 OkButton)}
	       {@top tkWM(deiconify)}
	    end
	 
	    meth close  % invoked by Window
	       {@top tkClose}
	       {@close}
	    end 
	 
	    meth Ok  % invoked by User <1>
	       NewProps = {Map @entries fun {$ A#E} A # {E tkReturn(get $)} end}
	    in
	       {Send @bObj update(NewProps)}
	       {Send @window dialogClosed}
	       {self close}
	    end
	 
	    meth Cancel  % invoked by User <1>
	       {Send @bObj update(@oldProps)}
	       {Send @window dialogClosed}
	       {self close}
	    end
	 end
      
	 class Circle from BObject
	    attr
	       kind:  oval
	       props: [fill width]
	       dx:    20
	       dy:    20
	       fill:   red
	       width:  1
	 end
      
	 class Square from Circle
	    attr
	       kind: rectangle
	       fill: blue
	 end
      
	 class HBar from Square
	    attr
	       dy:   4
	       fill: orange
	 end
      
	 class VBar from Square
	    attr
	       dx:   4
	       fill: green
	 end
      
	 class Arc from Circle
	    attr
	       kind:   arc
	       props:  [fill start extent]
	       fill:   yellow
	       start:  22.5
	       extent: 315
	 end
      
	 proc {MailBoard}
	    FN = {OS.tmpnam}
	 
	    To
	 
	    T = {New Tk.toplevel tkInit}
	    L = {New Tk.label  tkInit(parent:T text:'To: ')}
	    E = {New Tk.entry  tkInit(parent:T width:20)}
	    B = {New Tk.button tkInit(parent:T text:'Send'
				      action: proc {$}
						 To = {E tkReturnAtom(get $)}
						 {T tkClose}
					      end)}
	    {Tk.batch [pack(L E side:left pady:1#m padx:1#m)
		       pack(B side:bottom pady:2#m)
		       focus(E)]}
	 
	 in

	    {Wait To}
	 
	    {Pickle.saveCompressed
	     {NewBoardFunctor false Ticket To} FN PickleCompressionLevel}
	    
	    try
	       {SendApplet FN 'Oz Drawing Board' To}
	    catch _ then
	       D={New TkTools.error
		  tkInit(text: ('Could not send mail. Please check whether '#
				'metamail package is installed properly.'))}
	    in
	       {Wait D.tkClosed}
	    end
	    
	    {OS.unlink FN}
	 end

	 proc {SaveBoard}
	    case {Tk.return
		  tk_getSaveFile(filetypes: q(q('Oz Applications'
						'.oza')
					      q('All files'
						'*')))}
	    of nil then skip 
	    [] S then 
	       {Pickle.saveCompressed
		{NewBoardFunctor false Ticket 'Saved'}
		S PickleCompressionLevel}
	    end
	 end

	 Ticket B

	 UserId = {NewName}
      
	 if Args.master then
	    B      = {NewServer Board init}
	    Ticket = {New Connection.gate init(B $) _}
	 else
	    Ticket = Args.ticket
	    B      = {Connection.take Ticket}
	 end

	 {Wait Tk}
	 {Wait TkTools}
   
	 {{Send B newWindow($)} {AdjoinAt Tk tools TkTools}
	  d(title: 'Drawing Board'#'@'#{GetHostName}
	    close: proc {$}
		      {Application.exit 0}
		   end
	    mail:  MailBoard
	    save:  SaveBoard
	    user:  UserId)}
   
	 {Send B newUser(UserId Args.user)}
   
	 if Args.master then
	    {Send B newTool(circle Circle WObject)}
	    {Send B newTool(square Square WObject)}
	    {Send B newTool(hbar HBar WObject)}
	    {Send B newTool(vbar VBar WObject)}
	    {Send B newTool(arc Arc WObject)}
	 end

      end

   end

in

   {NewBoardFunctor true '' master}

end
