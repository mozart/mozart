%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   local
      fun {FindKey K|Kr D V}
	 case {Dictionary.get D K}.3==V then K else {FindKey Kr D V} end
      end
      fun {FindNext I D}
	 case {Dictionary.member D I} then I else {FindNext I-1 D} end
      end
      proc {DeleteAll Ks D}
	 case Ks of nil then skip
	 [] K|Kr then
	    case K<2 then skip else
	       {{Dictionary.get D K}.2 tkClose}
	       {Dictionary.remove D K}
	    end
	    {DeleteAll Kr D}
	 end
      end
   in
      class Actions
	 from Tk.variable
	 feat menu dict
	 attr separators:nil max:0 cur:0
	 meth init(menu: Menu)
	    Actions,tkInit(0)
	    self.menu = Menu
	    self.dict = {Dictionary.new}
	 end
	 
	 meth add(label:Label value:Value type:Type)
	    Num   = @max
	    Entry = {New Tk.menuentry.radiobutton
		     tkInit(parent:   self.menu
			    variable: self
			    value:    Num
			    action:   self # set(Num)
			    label:    Label)}
	 in
	    max <- Num + 1
	    cur <- Num
	    {Dictionary.put self.dict Num Label#Entry#Value#Type}
	    Actions,tkSet(Num)
	 end

	 meth addSeparator
	    separators <- {New Tk.menuentry.separator
			   tkInit(parent:self.menu)}|@separators
	 end

	 meth set(Num)
	    cur <- Num
	 end
	 
	 meth delete(Value)
	    Num   = {FindKey {Dictionary.keys self.dict} self.dict Value}
	    Entry = {Dictionary.get self.dict Num}.2
	 in
	    {Dictionary.remove self.dict Num}
	    {Entry tkClose}
	    case Num==@cur then
	       cur <- {FindNext Num-1 self.dict}
	       Actions,tkSet(@cur)
	    else skip end
	 end

	 meth deleteAll
	    {DeleteAll {Dictionary.keys self.dict} self.dict}
	    cur <- 1 % The second of the default entries
	    Actions,tkSet(@cur)
	    {ForAll @separators proc {$ O} {O tkClose} end}
	    separators <- nil
	 end
	 
	 meth get($)
	    {Dictionary.get self.dict @cur}
	 end
      end
   end
   
   proc {DoEntries M Es W}
      case Es of nil then skip
      [] E|Er then {DoEntries M E W} {DoEntries M Er W}
      elsecase {IsAtom Es} then {M.Es W}
      else {DoEntries M.{Label Es} Es.1 W}
      end
   end

in

   class MenuManager
      feat
	 menu
	 infoAction
	 cmpAction
	 statAction

      meth init
	 Menu = self.menu
      in
	 Menu =
	 {TkTools.menubar self.toplevel self.toplevel
	  [menubutton(text: 'Explorer'
		      menu: [command(label:   'About...'
				     action:  self # about
				     feature: about)
			     separator
			     command(label:   'Halt'
				     action:  self.status # halt
				     state:   disabled
				     key:     ctrl(g)
				     feature: halt)
			     command(label:   'Break'
				     action:  self.status # break
				     key:     ctrl(c)
				     feature: break)
			     separator
			     command(label:   'Reset'
				     action:  proc {$}
						 {self.status kill}
						 {self reset}
					      end
				     state:   disabled
				     key:     ctrl(r)
				     feature: reset)
			     command(label:   'Clear'
				     action:  proc {$}
						 {self.status kill}
						 {self clear}
					      end
				     state:   disabled
				     feature: clear)
			     separator
			     command(label:   'Export Postscript...'
				     action:  self # postscript
				     state:   disabled
				     feature: postscript)
			     separator
			     command(label:   'Quit'
				     action:  proc {$}
						 {self.status kill}
						 {self close}
					      end
				     feature: quit)]
		      feature: explorer)
	   menubutton(text: 'Options'
		      menu: [command(label:   'Search...'
				     action:  self # guiOptions(search)
				     feature: search)
			     command(label:   'Drawing...'
				     action:  self # guiOptions(drawing)
				     feature: drawing)
			     command(label:   'Postscript...'
				     action:  self # guiOptions(postscript)
				     feature: postscript)]
		      feature: options)
	   menubutton(text: 'Move'
		      menu: [command(label:   'Center'
				     action:  self # moveCurrent
				     state:   disabled
				     key:    'c'
				     feature: cur)
			     command(label:   'Top Node'
				     action:  self # moveTop
				     state:   disabled
				     key:     't'
				     feature: top)
			     separator
			     command(label:   'Leftmost'
				     action:  self # moveFrom(leftMost)
				     state:   disabled
				     event:   '<minus>'
				     key:     '-'
				     feature: leftMost)
			     command(label:   'Rightmost'
				     action:  self # moveFrom(rightMost)
				     state:   disabled
				     event :  '<plus>'
				     key:     '+'
				     feature: rightMost)
			     separator
			     command(label:   'Backtrack'
				     action:  self # moveFrom(back)
				     state:   disabled
				     key:     'b'
				     feature: back)
			     separator
			     command(label:   'Previous Solution'
				     action:  self # moveFrom(prevSol)
				     state:   disabled
				     event:   '<less>'
				     key:     '<'
				     feature: prevSol)
			     command(label:   'Next Solution'
				     action:  self # moveFrom(nextSol)
				     state:   disabled
				     event:   '<greater>'
				     key:     '>'
				     feature: nextSol)]
		      feature:move)
	   menubutton(text: 'Search'
		      menu: [command(label:   'Next Solution'
				     action:  self # next
				     state:   disabled
				     key:     'n'
				     feature: next)
			     command(label:   'All Solutions'
				     action:  self # all
				     state:   disabled
				     key:     'a'
				     feature: all)
			     separator
			     command(label:   'One Step'
				     action:  self # step
				     state:   disabled
				     key:     'o'
				     feature: step)]
		      feature: search)
	   menubutton(text: 'Nodes'
		      menu: [cascade(label:   'Information Action'
				     menu:    nil
				     feature: infoAction)
			     command(label:  'Information'
				     action:  self # nodesInfo
				     state:   disabled
				     key:     i
				     feature: info)
			     separator
			     cascade(label:   'Compare Action'
				     menu:    nil
				     feature: cmpAction)
			     command(label:   'Select Compare'
				     action:  self # nodesSelCmp
				     state:   disabled
				     key:     1
				     feature: selCmp)
			     command(label:   'Deselect Compare'
				     action:  self # nodesDeselCmp
				     state:   disabled
				     key:     0
				     feature: deselCmp)
			     command(label:   'Compare'
				     action:  self # nodesCmp
				     state:   disabled
				     key:     2
				     feature: cmp)
			     separator
			     cascade(label:   'Statistics Action'
				     menu:    nil
				     feature: statAction)
			     command(label:   'Statistics'
				     action:  self # stat
				     state:   disabled
				     key:     s
				     feature: stat)]
		      feature: nodes)
	   menubutton(text: 'Hide'
		      menu: [command(label:   'Hide/Unhide'
				     action:  self # nodes(hide)
				     state:   disabled
				     key:     h
				     feature: toggle)
			     separator
			     command(label:   'Hide Failed'
				     action:  self # nodes(hideFailed)
				     state:   disabled
				     key:     f
				     feature: failed)
			     separator
			     command(label:   'Unhide But Failed'
				     action:  self # nodes(unhideButFailed)
				     state:   disabled
				     key:     u
				     feature: butfailed)
			     command(label:   'Unhide All'
				     action:  self # nodes(unhideTree)
				     state:   disabled
				     key:     ctrl(u)
				     feature: all)
			    ]
		      feature: hide)]
	  nil}
	 {Menu.explorer.menu tk(conf tearoff:false)}
	 {Menu.options.menu  tk(conf tearoff:false)}
	 {Menu.move.menu     tk(conf tearoff:false)}
	 {Menu.search.menu   tk(conf tearoff:false)}
	 {Menu.nodes.menu    tk(conf tearoff:false)}
	 {Menu.nodes.statAction.menu tk(conf tearoff:false)}
	 {Menu.nodes.infoAction.menu tk(conf tearoff:false)}
	 {Menu.nodes.cmpAction.menu  tk(conf tearoff:false)}
	 {Menu.hide.menu     tk(conf tearoff:false)}
	 self.infoAction = {New Actions init(menu:Menu.nodes.infoAction.menu)}
	 self.cmpAction  = {New Actions init(menu:Menu.nodes.cmpAction.menu)}
	 self.statAction = {New Actions init(menu:Menu.nodes.statAction.menu)}
      end
      
      meth clear
	 {DoEntries self.menu [explorer([clear postscript break halt reset])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step])
			       nodes([cmp info selCmp deselCmp stat])
			       hide([toggle all failed butfailed])]
	  tk(entryconf state:disabled)}
      end

      meth busy
	 {DoEntries self.menu [explorer([about postscript])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step])
			       nodes([cmp info selCmp deselCmp stat])
			       hide([toggle all failed butfailed])
			       options([search drawing postscript])]
	  tk(entryconf state:disabled)}
      end

      meth idle
	 {DoEntries self.menu [explorer([about postscript])
			       options([search drawing postscript])]
	  tk(entryconf state:normal)}
	 {DoEntries self.menu explorer([break halt])
	  tk(entryconf state:disabled)}
      end
      
      meth normal(Es)
	 {DoEntries self.menu Es tk(entryconf state:normal)}
      end
   
      meth disable(Es)
	 {DoEntries self.menu Es tk(entryconf state:disabled)}
      end
      
      meth state(B Es)
	 case B then {DoEntries self.menu Es tk(entryconf state:normal)}
	 else {DoEntries self.menu Es tk(entryconf state:disabled)}
	 end
      end

   end

end

