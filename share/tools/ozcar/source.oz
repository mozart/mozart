%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
local

   %% emacs Oz mode only writes tabs at the beginning of
   %% each line, so this algorithm is correct...
   fun {FixTabs L}
      case L == nil then nil
      else
	 case L.1 == 9 then
	    32 | 32 | 32 | 32 | 32 | 32 | 32 | 32 | {FixTabs L.2}
	 else
	    L.1 | {FixTabs L.2}
	 end
      end
   end
   
   class SourceWindow from Tk.text
      feat
	 filename
      attr
	 CurrentLine
	 Time
      
      meth init(parent:P file:F
		width: Width <=SourceWindowTextSize.1
		height:Height<=SourceWindowTextSize.2)
	 Tk.text,tkInit(parent: P         bg: SourceTextBackground
			font:   SmallFont bd: BorderSize cursor: TextCursor
			width:  Width     height:Height)
	 self.filename = F
	 CurrentLine <- line(appl:undef stack:undef)
	 {self Load(file:F)}
      end

      meth Load(file:F)
	 File = {New class $ from Open.file Open.text end
		 init(name:F flags:[read])}
      in
	 Time <- {OS.stat F}.mtime
	 {ForAll [tk(conf state:normal)
		  tk(delete '0.0' 'end')
		  DoLoad(File 1)
		  tk(conf state:disabled)] self}
	 {File close}
      end
      
      meth DoLoad(F L)
	 Line = {F getS($)}
      in
	 case Line == false then
	    skip
	 else
	    FL   = {StripPath self.filename} # ', line ' # L
	    D1   = 'Deleted b'
	    D2   = 'delete b'
	    Set  = 'set b'
	    Br   = 'reakpoint at '
	    Err  = 'Failed to '
	    AcSet  = {New Tk.action
		      tkInit(parent: self
			     action:
				proc{$}
				   Ok = {Debug.breakpointAt
					 self.filename L true}
				in
				   case Ok then
				      {Ozcar rawStatus('B'#Br#FL)}
				      {self tk(tag conf q(L)
					       font:SmallBoldFont)}
				   else
				      {Ozcar rawStatus(Err#Set#Br#FL)}
				   end
				end)}
	    AcDelete = {New Tk.action
			tkInit(parent: self
			       action: proc{$}
					  Ok = {Debug.breakpointAt
						self.filename L false}
				       in
					  case Ok then
					     {Ozcar rawStatus(D1#Br#FL)}
					     {self tk(tag conf q(L)
						   font:SmallFont)}
					  else
					     {Ozcar rawStatus(Err#D2#Br#FL)}
					  end
				       end)}
	 in
	    {ForAll [tk(insert 'end' {PrintF L 5} # {FixTabs Line} # NL q(L))
		     tk(tag bind q(L) '<1>' AcSet)
		     tk(tag bind q(L) '<3>' AcDelete)] self}
	    {self DoLoad(F L+1)}
	 end
      end

      meth Update(What Where)
	 CurrentLine <- {Record.adjoinAt @CurrentLine What Where}
      end

      meth mtime($)
	 @Time
      end
	 
      meth highlight(line:L color:C what:What<=appl)
	 case {OS.stat self.filename}.mtime > @Time then
	    {OzcarMessage 'Reloading file ' # self.filename}
	    SourceWindow,Load(file:self.filename)
	 else skip end
	 case @CurrentLine.What \= undef then
	    Other         = case What == appl then stack else appl end
	    NewColors     = case @CurrentLine.Other == @CurrentLine.What then
			       SourceTextInvForeground #
			       ScrollbarColors.Other
			    else
			       SourceTextForeground #
			       SourceTextBackground
			    end
	 in
	    {self tk(tag conf q(@CurrentLine.What)
		     foreground:NewColors.1
		     background:NewColors.2)}
	 else skip end
	 case L \= undef then
	    {ForAll [tk(tag conf q(L)
			foreground: SourceTextInvForeground
			background: C)
		     tk(see L#'.0')] self}
	    SourceWindow,Update(What L)
	 else
	    SourceWindow,Update(What undef)
	 end
      end
   end
   
in
   
   class SourceManager from Tk.toplevel
      feat
	 NoteBook
      attr
	 WindowList    : nil
	 CurrentWindow : window(appl:undef stack:undef)
	 WithDrawn     : true
      
      meth init
	 Tk.toplevel,tkInit(parent:self.toplevel
			    title:SourceWindowTitle withdraw:true)
	 self.NoteBook = {New TkTools.notebook tkInit(parent:self)}
	 {Tk.batch [pack(self.NoteBook expand:yes fill:both)
		    wm(iconname   self SourceWindowIcon)
		    wm(iconbitmap self BitMap)
		   ]}
      end
      
      meth lookup(file:F entry:?E)
         {ForAll @WindowList
          proc{$ X}
             case X.1.filename == F then
                E = X   % there is only one matching object... (hope so :-))
             else skip end
          end}
      end

      meth Update(What Which)
	 CurrentWindow <- {Record.adjoinAt @CurrentWindow What Which}
      end

      meth EraseScrollbar(What)
	 case What == both then
	    {ForAll [appl stack]
	     proc{$ W}
		case @CurrentWindow.W \= undef then
		   {@CurrentWindow.W highlight(line:undef
					       color:undef what:W)}
		   SourceManager,Update(W undef)
		else skip end
	     end}
	 else
	    case @CurrentWindow.What \= undef then
	       {@CurrentWindow.What highlight(line:undef
					      color:undef what:What)}
	       SourceManager,Update(What undef)
	    else skip end
	 end
      end
	 
      meth scrollbar(file:F line:L color:C what:What<=appl ack:Ack<=unit)
	 case F == undef orelse F == '' orelse F == noDebugInfo then
	    SourceManager,EraseScrollbar(What)
	 else
	    %% heuristic: if F begins with '.?.' then it's a prelude file
	    FS    = {Atom.toString F}
	    RealF = case FS.1 == &. andthen FS.2.2.1 == &. then
		       {VS2A {System.get home} # '/lib/' # F}
		    else
		       F
		    end
	    E = {self lookup(file:RealF entry:$)}
	 in
	    case {IsDet E} then
	       {self ToTop(entry:E line:L color:C what:What)}
	    else
	       {self NewFile(file:RealF line:L color:C what:What)}
	    end
	    case {IsDet Ack} then skip else Ack = unit end
	    case @WithDrawn then
	       {Tk.batch [wm(geometry  self SourceWindowGeometry)
			  wm(resizable self false false)
			  wm(deiconify self)
			 ]}
	       WithDrawn <- false
	    else skip end
	 end
      end
      
      meth NewFile(file:F line:L color:C what:What)
	 N = {New TkTools.note tkInit(parent: self.NoteBook
				      text:   {StripPath F})}
	 W = {New SourceWindow init(parent:N file:F)}
      in
	 WindowList <- W#N | @WindowList
	 {Tk.send pack(W expand:yes fill:both)}
	 {self.NoteBook add(N)}
	 {self ToTop(entry:W#N line:L color:C what:What)}
      end
      
      meth ToTop(entry:E line:L color:C what:What)
	 case @CurrentWindow.What \= undef then
	    {@CurrentWindow.What highlight(line:undef color:undef what:What)}
	 else skip end
	 SourceManager,Update(What E.1)
	 {self.NoteBook toTop(E.2)}
	 {E.1 highlight(line:L color:C what:What)}
      end

      meth isUpToDate(Time $)
	 case @CurrentWindow.appl == undef then
	    true
	 else
	    {@CurrentWindow.appl mtime($)} =< Time
	 end
      end
      
      meth close
	 Tk.toplevel,tkClose
      end
   end
end
