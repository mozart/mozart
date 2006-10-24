%%%
%%% Authors:
%%%   Christian Schulte (schulte@dfki.de)
%%%
%%% Copyright:
%%%   Christian Schulte, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

local
   An = 'announce: '
   Sp = '          '
in
			     
   class History from Tk.toplevel
      attr mapped:false
      feat text entry var
	 
      meth init(master:M suffix:U)
	 History, tkInit(parent:M.toplevel title:'History: '#U withdraw:true
			 delete: self # unmap)
	 F = {New TkTools.textframe tkInit(parent:self text:'History: '#U)}
	 T = {New Tk.text tkInit(parent:F.inner
				 width:HistoryWidth height:HistoryHeight
				 bg:HistoryBg font:HistoryFont
				 highlightthickness: 0)}
	 S = {New Tk.scrollbar tkInit(parent:F.inner)}
	 V = {New Tk.variable tkInit(false)}
	 E = {New Tk.menuentry.checkbutton tkInit(parent:   M.menu.windows.menu
						  label:    U
						  variable: V
						  action:   self # toggle)}
      in
	 {T tkBind(event:'<Map>'   action: self # map)}
	 {T tkBind(event:'<Unmap>' action: self # unmap)}
	 {Tk.addYScrollbar T S}
	 {Tk.batch [pack(T S side:left fill:y) pack(F)]}
	 self.text  = T
	 self.entry = E
	 self.var   = V
      end

      meth setMapped(B)
	 mapped <- B
	 {self.var tkSet(B)}
      end
      
      meth toggle
	 {Tk.send wm(if @mapped then iconify else deiconify end self)}
	 History, setMapped({Not @mapped})
      end
      
      meth map
	 if @mapped then skip else {Tk.send wm(deiconify self)}
	 end
	 History, setMapped(true)
      end

      meth unmap
	 if @mapped then {Tk.send wm(iconify self)}
	 end
	 History, setMapped(false)
      end
      
      meth print(V)
	 Text = self.text
      in
	 {Text tk(insert 'end' V#'\n')}
	 {Text tk(yview pickplace:'end')}
      end
      
      meth announce(what:W weight:N src:S dst:D reply:R<=unit ...)
	 History,print(An#W#', '#N#'\n'#
		       Sp#'from: '#S#' to: '#D#
		       case R
		       of grant then '\n'#Sp#'granted'
		       [] reject then '\n'#Sp#'rejected'
		       else ''
		       end)
      end

      meth tkClose
	 {self.entry tkClose}
	 Tk.toplevel, tkClose
      end
   end
   
end

local
   proc {DefAction _}
      skip
   end
in
   class EntryChooser
      from Tk.frame
      feat entry button toplevel action
      attr entries

      meth tkInit(parent:P toplevel:T entries:Es action:A<=DefAction)
	 Tk.frame, tkInit(parent:P highlightthickness:2)
	 Entry  = {New Tk.entry tkInit(parent:self width:BigTextWidth bg:TextBg
				       font: TextFont
				       highlightthickness:0)}
	 Button = {New Tk.button tkInit(parent: self
					image: Images.down
					highlightthickness:0
					state:  if Es==nil then disabled
						else normal
						end
					action: self # OpenChooser)}
      in
	 self.entry    = Entry
	 self.button   = Button
	 self.toplevel = T
	 self.action   = A
	 EntryChooser,entries(Es)
	 {Tk.send pack(self.entry self.button side:left fill:y)}
      end

      meth entries(Es)
	 entries <- Es
	 if Es\=nil then
	    {self.entry tk(delete 0 'end')}
	    {self.entry tk(insert 0 Es.1)}
	 end
      end
      
      meth OpenChooser
	 [X Y H] = {Map [rootx rooty height]
		    fun {$ WI} {Tk.returnInt winfo(WI self)} end}
	 T = {New Tk.toplevel tkInit(withdraw: true
				     parent:   self
				     cursor:   top_left_arrow)}
	 F = {New Tk.frame tkInit(parent:T bg:black bd:2)}
	 L = {New Tk.listbox   tkInit(parent:F height:TextHeight
				      width:BigTextWidth
				      bg:white
				      exportselection:false)}
	 S = {New Tk.scrollbar tkInit(parent:F width:10)}
      in
	 {L tk(insert 0 b(@entries))}
	 {Tk.addYScrollbar L S}
	 {self.toplevel tkBind(event:'<1>' action:T#tkClose)}
	 {L tkBind(event:'<1>'
		   action: proc {$}
			      A={L tkReturnAtom(get
						l(L curselection) $)}
			   in
			      {self.entry tk(delete 0 'end')}
			      {self.entry tk(insert 0 A)}
			      {self.action A}
			      {T tkClose}
			   end)}
	 {L tkBind(event:'<2>'
		   action: T # tkClose)}
	 {Tk.batch [wm(overrideredirect T true)
		    wm(geometry T '+'#X#'+'#Y+H)
		    pack(L S side:left fill:both)
		    pack(F)
		    wm(deiconify T)]}
      end
      
   end
end

