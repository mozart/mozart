%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

%declare
local
   class SourceWindow from Tk.text
      feat
	 StatusLine
      attr
	 CurrentFile : undef
	 CurrentLine : undef
      
      meth init(parent:P width:W<=80 height:H<=50)
	 self.StatusLine =
	 {New Tk.label tkInit(parent:P font:BoldFont
			      text:"No file loaded")}
	 Tk.text,tkInit(parent:P width:W height:H
			font:SmallFont bd:BorderSize)
	 {ForAll [pack(self.StatusLine fill:x expand:no)
		  pack(self fill:both expand:yes)] Tk.send}
	 {self tk(conf state:disabled)}
      end
      
      meth load(file:F)
	 File = {New class $ from Open.file Open.text end
		 init(name:F flags:[read])}
      in
	 CurrentFile <- F
	 {self.StatusLine tk(conf text:F)}
	 {self DoLoad(File 1)}
      end
      
      meth current(file:F line:L)
	 F = @CurrentFile
	 L = @CurrentLine
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
	 case @CurrentLine \= undef then
	    {self tk(tag conf q(@CurrentLine)
		     foreground:black
		     background:white)}
	 else skip end
	 {self tk(tag conf q(L) foreground:white background:C)}
	 CurrentLine <- L
      end
   end
   
in
   
   class SourceManager
      attr
	 Windows
      
      meth init
	 Windows <- nil
      end
      
      meth lookup(file:F window:?W)
	 {ForAll @Windows
	  proc{$ X}
	     case {X.1 current(file:$ line:_)} == F then
		W = X.1   % there is only one matching object... (hope so :-))
	     else skip end
	  end}
      end
      
      meth highlight(file:F line:L color:C<=black)
	 W = {self lookup(file:F window:$)}
      in
	 case {IsDet W} then
	    {W highlight(line:L color:C)}
	 else
	    T = {New Tk.toplevel tkInit(title:'Oz Source')}
	    NewW = {New SourceWindow init(parent:T)}
	 in
	    Windows <- NewW # T | @Windows
	    {ForAll [load(file:F) highlight(line:L color:C)] NewW}
	 end
      end

      meth close
	 {ForAll @Windows
	  proc {$ W}
	     {W.2 tkClose}
	  end}
      end
   end
end


/*

declare
SM = {New SourceManager init}

{SM display(file:"/etc/passwd" line:9)}
{SM display(file:"/etc/hosts" line:8)}

*/

