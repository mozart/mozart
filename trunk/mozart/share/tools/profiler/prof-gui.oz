%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

local

   local
      fun {DoWhere Xs Y N}
	 X|Xr = Xs in
	 case Y == X then N else
	    {DoWhere Xr Y N+1}
	 end
      end
   in
      fun {Where Xs Y}
	 {DoWhere Xs Y 0}
      end
   end

   fun {CheckName N}
      case N == '' then '$' else N end
   end

   local
      C = 10000
      D =  1024
   in
      fun {FormatSize B}
	 case     B < C   then B           # 'b'
	 elsecase B < C*D then B div D     # 'k'
	 else                  B div (D*D) # 'M'
	 end
      end
   end

   fun {FormatTime T}
      case T < 60 then
	 T # 's'
      elsecase T < 3600 then
	 T div 60 # 'm' # T mod 60 # 's'
      else H = T mod 3600 in
	 T div 3600 # 'h' # H div 60 # 'm' # H mod 60 # 's'
      end
   end
   
in

   class Gui from Menu Dialog Help Time.repeat

      prop
	 locking
      
      feat
	 toplevel
	 menuBar
      
	 ButtonFrame
	 SortBox
	 SortMenu

	 BarCanvas
	 ProcText
	 SumText
      
	 StatusFrame
	 StatusText

      attr
	 Stats          : nil
	 StatsCount     : 0
	 ResetTime      : {OS.time}
      
	 TagList    : nil
	 SortList   : [calls closures samples heap]
	 SortBy     : calls

	 StatusSync : _

      meth init
	 %% create the main window, but delay showing it
	 self.toplevel = {New Tk.toplevel tkInit(title:    TitleName
						 delete:   self # off
						 withdraw: true)}
	 {Tk.batch [wm(iconname   self.toplevel IconName)
		    wm(iconbitmap self.toplevel BitMap)
		    wm(geometry   self.toplevel ToplevelGeometry)]}
	 
	 Menu,init
	 Dialog,init
	 Help,init

	 Time.repeat,setRepAll(action:update delay:ConfigUpdate)
	 
	 {ForAll [self.ButtonFrame self.StatusFrame]
	  proc{$ F}
	     F = {New Tk.frame tkInit(parent: self.toplevel
				      bd:     SmallBorderSize
				      relief: ridge)}
	  end}
      
	 {Tk.batch [grid(self.menuBar       row:0 column:0
			 sticky:we columnspan:2)
		    grid(self.ButtonFrame   row:1 column:0
			 sticky:we columnspan:2)
		    grid(self.StatusFrame   row:6 column:0
			 sticky:we columnspan:2)
		   ]}
      
	 %% the buttons
	 local
	    Bs = {Map [UpdateButtonText ResetButtonText]
		  fun {$ S}
		     B = {New Tk.button
			  tkInit(parent: self.ButtonFrame
				 text:   S
				 padx:   PadXButton
				 pady:   PadYButton
				 font:   ButtonFont
				 borderwidth: SmallBorderSize
				 action: self # action(S))}
		  in
		     {B tkBind(event:  HelpEvent
			       action: self # help(S))}
		     B
		  end}
	    SortLabel = {New Tk.label tkInit(parent: self.ButtonFrame
					     text:   SortButtonText
					     pady:   PadYButton
					     font:   ButtonFont
					     borderwidth: SmallBorderSize)}
	    self.SortBox = {New Tk.label tkInit(parent: self.ButtonFrame
						text:   @SortBy
						relief: raised
						padx:   PadXButton
						pady:   2
						font:   ButtonFont
						width:  7
						%bg:     DefaultBackground
						borderwidth: SmallBorderSize
					       )}
	 in
	    {ForAll [tkBind(event:  '<1>'
			    action: self # PrintSortMenu)
		     tkBind(event:  HelpEvent
			    action: self # help(SortButtonText))] self.SortBox}
	    {Tk.batch [pack(b(Bs) side:left  padx:1)
		       pack(self.SortBox SortLabel side:right padx:3)
		      ]}
	 end

	 %% sort menu
	 self.SortMenu =
	 {New Tk.menu tkInit(parent:      self.toplevel
			     font:        ButtonFont
			     borderwidth: SmallBorderSize
			     activeborderwidth: SmallBorderSize
			     transient:   yes
			     tearoff:     no)}
         {ForAll @SortList
          proc {$ S}
	     {New Tk.menuentry.command tkInit(parent: self.SortMenu
					      label:  S
					      action: self # DoSortBy(S)) _}
          end}
	 
	 %% border line
         local
	    F = {New Tk.frame tkInit(parent: self.toplevel
				     height: SmallBorderSize
				     bd:     NoBorderSize
				     relief: flat)}
	 in
	    {Tk.send grid(F row:2 column:0 sticky:we columnspan:3)}
	 end
 
	 %% status line
	 self.StatusText =
	 {New Tk.text tkInit(parent: self.StatusFrame
			     state:  disabled
			     height: 1
			     width:  0
			     bd:     NoBorderSize
			     cursor: TextCursor
			     font:   StatusFont)}
	 {self.StatusText tkBind(event:  HelpEvent
				 action: self # help(StatusHelp))}
	 {Tk.send pack(self.StatusText side:left padx:2 fill:x expand:yes)}

	 %% create the bar canvas...
	 self.BarCanvas =
	 {New YScrolledTitleCanvas tkInit(parent: self.toplevel
					  title:  BarCanvasTitle
					  bd:     SmallBorderSize
					  relief: sunken
					  width:  BarCanvasWidth
					  bg:     DefaultBackground)}
	 {self.BarCanvas tkBind(event:  HelpEvent
				action: self # help(BarCanvasTitle))}
	 
	 %% ...the text widget for detailed output...
	 self.ProcText =
	 {New TitleText tkInit(parent: self.toplevel
			       title:  ProcTextTitle
			       wrap:   none
			       state:  disabled
			       width:  ProcTextWidth
			       height: ProcTextHeight
			       bd:     SmallBorderSize
			       cursor: TextCursor
			       font:   DefaultFont
			       bg:     DefaultBackground)}
	 {self.ProcText tkBind(event:  HelpEvent
			      action: self # help(ProcTextTitle))}
	 
	 %% ...and the text widget for general output
	 self.SumText =
	 {New TitleText tkInit(parent: self.toplevel
			       title:  SumTextTitle
			       wrap:   none
			       state:  disabled
			       width:  SumTextWidth
			       height: SumTextHeight
			       bd:     SmallBorderSize
			       cursor: TextCursor
			       font:   DefaultFont
			       bg:     DefaultBackground)}
	 {self.SumText tkBind(event:  HelpEvent
			      action: self # help(SumTextTitle))}
	 
	 {Tk.batch [grid(self.BarCanvas  row:3 column:0 sticky:nswe rowspan:2)
		    grid(self.ProcText    row:3 column:1 sticky:nswe)
		    grid(self.SumText    row:4 column:1 sticky:nswe)
		    grid(rowconfigure    self.toplevel 3 weight:1)
		    grid(rowconfigure    self.toplevel 4 weight:1)
		    grid(columnconfigure self.toplevel 0 weight:5)
		    grid(columnconfigure self.toplevel 1 weight:1)
		   ]}
      end

      meth PrintSortMenu
         X = {Tk.returnInt winfo(pointerx '.')}
         Y = {Tk.returnInt winfo(pointery '.')}
	 N = {Where @SortList @SortBy}
      in
	 {Tk.send tk_popup(self.SortMenu X Y N)}
      end

      meth DoSortBy(S)
	 SortBy <- S
	 {self.SortBox tk(conf text:S)}

	 Gui,doStatus('Sorting by ' # S # '...')
	 Gui,UpdateBars
	 Gui,doStatus(' done' append)
      end
      
      meth UpdateBars
	 case @Stats == nil then
	    Gui,DeleteBars(false)
	    Gui,UpdateSumInfo
	 else
	    RawData    = {Filter @Stats fun {$ X}
					   X.@SortBy >= ConfigThreshold.@SortBy
					end}
	    SortedData = {Sort RawData fun {$ X Y} X.@SortBy > Y.@SortBy end}
	    Max        = case SortedData == nil then 0.1 else
			    {Int.toFloat SortedData.1.@SortBy} + 0.1 end
	    XStretch   = 207.0
	 
	    fun {YStretch I}
	       (I-1)*32
	    end
	 in
	    Gui,DeleteBars(false)
	    {List.forAllInd SortedData
	     proc {$ I S}
		Length = {Int.toFloat S.@SortBy}
		CTag = {New Tk.canvasTag tkInit(parent:{self.BarCanvas w($)})}
	     in
		{ForAll [tk(crea rectangle
			    7                             {YStretch I}+18
			    7.0+(Length/Max)*XStretch     {YStretch I}+32
			    fill: SelectedBackground
			    tags: CTag)
			 tk(crea text
			    7.0+(Length/Max)*XStretch+5.0 {YStretch I}+19
			    text:   case @SortBy
				    of heap then {FormatSize S.heap}
				    [] X    then S.X
				    end
			    anchor: nw
			    tags:   CTag
			    font:   BoldFont)
			 tk(crea text
			    8                             {YStretch I}+4
			    text:   {CheckName S.name}
			    anchor: nw
			    tags:   CTag
			    font:   DefaultFont)
			] self.BarCanvas}
		{CTag tkBind(event:  '<1>'
			     action: self # UpdateProcInfo(S))}
		TagList <- CTag | @TagList
		StatsCount <- I
	     end}
	    {self.BarCanvas
	     tk(conf scrollregion: q(7 3 XStretch {YStretch @StatsCount}+40))}
	    Gui,UpdateProcInfo({CondSelect SortedData 1 nil})
	    Gui,UpdateSumInfo
	 end
      end

      meth DeleteBars(RemoveEmacsBar<=true)
	 case RemoveEmacsBar then
	    {self.BarCanvas tk(conf scrollregion: q(7 3 7 3))}
	    case {Cget emacs} then
	       SourceManager,removeBar
	    else skip end
	 else skip end
	 {ForAll @TagList
	  proc {$ T}
	     {self.BarCanvas tk(delete T)}
	  end}
	 TagList    <- nil
	 StatsCount <- 0
      end
      
      meth DeleteProcInfo
	 Gui,Clear(self.ProcText)
	 Gui,Disable(self.ProcText)
      end
      
      meth DeleteSummary
	 Gui,Clear(self.SumText)
	 Gui,Disable(self.SumText)
      end
      
      meth UpdateProcInfo(S)
	 Gui,Clear(self.ProcText)
	 case S == nil then skip else
	    Gui,Append(self.ProcText
		       ' Name: ' # {CheckName S.name} # '\n' #
		       ' File: ' # {StripPath S.file} # '\n' #
		       ' Line: ' # S.line # '\n' #
		       ' Call: ' # S.calls # '\n' #
		       ' Clos: ' # S.closures # '\n' #
		       ' Smpl: ' # S.samples # '\n' #
		       ' Heap: ' # {FormatSize S.heap})
	    Gui,Disable(self.ProcText)
	    case {Cget emacs} then
	       SourceManager,bar(file:S.file line:S.line state:runnable)
	    else skip end
	 end
      end
      
      meth UpdateSumInfo
	 case @Stats == nil then
	    Gui,Clear(self.SumText)
	    Gui,Append(self.SumText ' No info available')
	    Gui,Disable(self.SumText)
	 else
	    Procs    = {Length @Stats}
	    Calls    = {FoldL @Stats fun {$ A S} A+S.calls end 0}
	    Closures = {FoldL @Stats fun {$ A S} A+S.closures end 0}
	    Heap     = {FoldL @Stats fun {$ A S} A+S.heap end 0}
	    TimeDiff = {OS.time} - @ResetTime
	 in
	    Gui,Clear(self.SumText)
	    Gui,Append(self.SumText
		       ' Procs:    ' # Procs             # '\n' #
		       ' Calls:    ' # Calls             # '\n' #
		       ' Closures: ' # Closures          # '\n' #
		       ' Heap:     ' # {FormatSize Heap} # '\n' #
		       ' Time:     ' # {FormatTime TimeDiff})
	    Gui,Disable(self.SumText)
	 end
      end
	 
      meth status(S M<=clear C<=DefaultForeground)
	 New in
         StatusSync <- New = unit
         thread
            {WaitOr New {Alarm TimeoutToStatus}}
            case {IsDet New} then skip else
	       Gui,doStatus(S M C)
            end
         end
      end

      meth doStatus(S M<=clear C<=DefaultForeground)
	 W = self.StatusText
      in
	 case M == clear then
	    Gui,Clear(W)
	 else
	    Gui,Enable(W)
	 end
	 Gui,Append(W S C)
	 Gui,Disable(W)
      end

      meth toggleEmacs
	 case {Cget emacs} then
	    Gui,doStatus('Not using Emacs Bar')
	    SourceManager,removeBar
	 else
	    Gui,doStatus('Using Emacs Bar')
	 end
	 {Ctoggle emacs}
      end

      meth update
	 Stats <- {Filter {Profile.getInfo}
		   fun {$ S}
		      P = {StripPath S.file}
		   in
		      %% make sure profiling is not reflexive :-)
		      P \= 'prof-gui.oz'     andthen
		      P \= 'prof-dialog.oz'  andthen
		      P \= 'prof-string.oz'  andthen
		      P \= 'prof-tk.oz'      andthen
		      P \= 'prof-prelude.oz' andthen
		      P \= 'prof-source.oz'  andthen
		      P \= 'prof-config.oz'  andthen
		      P \= 'TkTools.oz'      andthen
		      P \= 'Time.oz'
		   end}
	 Gui,UpdateBars
      end

      meth reset
	 {Profile.reset}
	 Stats      <- nil
	 ResetTime  <- {OS.time}
	 
	 Gui,DeleteBars
	 Gui,DeleteProcInfo
	 Gui,DeleteSummary
      end
      
      meth action(A)
	 lock
	    case A
	    of !UpdateButtonText then
	       Gui,doStatus('Updating...')
	       Gui,update
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(' done' append)
	       
	    [] !ResetButtonText then
	       Gui,doStatus('Resetting...')
	       Gui,reset
	       {Delay 200} %% just to look nice... ;)
	       Gui,doStatus(' done' append)
	    end
	 end
      end
      
      meth Clear(Widget)
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')] Widget}
      end
      
      meth Enable(Widget)
	 {Widget tk(conf state:normal)}
      end
      
      meth Append(Widget Text Color<=DefaultForeground)
	 {ForAll [tk(insert 'end' Text)
		  tk(conf fg:Color)] Widget}
      end
      
      meth Disable(Widget)
	 {Widget tk(conf state:disabled)}
      end
      
      meth DeleteLine(Widget Nr)
	 {Widget tk(delete Nr#'.0' Nr#DotEnd)}
      end

      meth DeleteToEnd(Widget Nr)
	 {Widget tk(delete Nr#'.0' 'end')}
      end
   end
end
