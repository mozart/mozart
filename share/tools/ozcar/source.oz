%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
local
   class SourceWindow from Tk.text
      feat
	 filename
      attr
	 CurrentLine : undef
      
      meth init(parent:P file:F
		width: Width <=SourceWindowTextSize.1
		height:Height<=SourceWindowTextSize.2)
	 Tk.text,tkInit(parent: P         bg: SourceTextBackground
			font:   SmallFont bd: BorderSize
			width:  Width     height:Height)
	 {self Load(file:F)}
      end
      
      meth Load(file:F)
	 File = {New class $ from Open.file Open.text end
		 init(name:F flags:[read])}
      in
	 self.filename = F
	 {self DoLoad(File 1)}
      end
      
      meth DoLoad(F L)
	 Line = {F getS($)}
      in
	 case Line == false then
	    skip
	 else
	    {ForAll [tk(conf state:normal)
		     tk(insert 'end' Line#[10] q(L))
		     tk(conf state:disabled)] self}
	    {self DoLoad(F L+1)}
	 end
      end
      
      meth highlight(line:L color:C)
	 {Show highlight#L#@CurrentLine}
	 case @CurrentLine \= undef then
	    {self tk(tag conf q(@CurrentLine)
		     foreground:black
		     background:white)}
	 else skip end
	 case L \= undef then
	    {ForAll [tk(tag conf q(L) foreground:white background:C)
		     tk(see L#'.0')] self}
	    CurrentLine <- L
	 else
	    CurrentLine <- undef
	 end
      end
   end
   
in
   
   class SourceManager from Tk.toplevel
      feat
	 NoteBook
      attr
	 WindowList : nil
	 Current    : undef
	 WithDrawn  : true
      
      meth init
	 Tk.toplevel,tkInit(title:SourceWindowTitle withdraw:true)
	 self.NoteBook = {New TkTools.notebook tkInit(parent:self)}
	 {Tk.batch [pack(self.NoteBook expand:yes fill:both)
		    wm(iconname   self SourceWindowIcon)
		    wm(iconbitmap self BitMap)
		    wm(geometry   self SourceWindowGeometry)
		    wm(resizable  self false false)
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
      
      meth scrollbar(file:F line:L color:C)
	 {Show scrollbar#F}
	 case F == undef orelse F == '' orelse L == undef then
	    case @Current \= undef then
	       {@Current highlight(line:undef color:undef)}
	    else skip end
	 else
	    E = {self lookup(file:F entry:$)}
	 in
	    case {IsDet E} then
	       {self ToTop(entry:E line:L color:C)}
	    else
	       {self NewFile(file:F line:L color:C)}
	    end
	    case @WithDrawn then
	       {Tk.send wm(deiconify self)}
	       WithDrawn <- false
	    else skip end
	 end
      end
      
      meth NewFile(file:F line:L color:C)
	 N = {New TkTools.note tkInit(parent:self.NoteBook
				      text: {Str.rchr {Atom.toString F} &/}.2)}
	 W = {New SourceWindow init(parent:N file:F)}
      in
	 WindowList <- W#N | @WindowList
	 {Tk.send pack(W expand:yes fill:both)}
	 {self.NoteBook add(N)}
	 {self ToTop(entry:W#N line:L color:C)}
      end
      
      meth ToTop(entry:E line:L color:C)
	 case @Current == E.1 then
	    skip
	 else
	    Current <- E.1
	    {self.NoteBook toTop(E.2)}
	 end
	 {E.1 highlight(line:L color:C)}
      end
      
      meth close
	 Tk.toplevel,tkClose
      end
   end
end
