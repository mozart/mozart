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
   in
      class Actions
	 from Tk.variable
	 feat menu dict
	 attr max:0 cur
	 meth init(menu: Menu)
	    <<Tk.variable tkInit(0)>>
	    self.menu = Menu
	    self.dict = {Dictionary.new}
	 end
	 
	 meth add(label:Label value:Value)
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
	    {Dictionary.put self.dict Num Label#Entry#Value}
	    <<Tk.variable tkSet(Num)>>
	 end

	 meth set(Num)
	    cur <- Num
	 end
	 
	 meth delete(value:Value)
	    Num = {FindKey {Dictionary.keys self.dict} self.dict Value}
	    _ # Entry # _ = {Dictionary.get self.dict Num}
	 in
	    {Dictionary.remove self.dict Num}
	    {Entry close}
	    case Num==@cur then
	       cur <- {FindNext Num-1 self.dict}
	    else true end
	 end

	 meth get($)
	    {Dictionary.get self.dict @cur}.3
	 end
      end
   end
   
   proc {DoEntries M Es W}
      case Es of nil then true
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
		      menu: [command(label:   'About'
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
			     command(label:   'Export Postscript'
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
		      menu: [command(label:   'Search ...'
				     action:  self # searchOptions
				     feature: search)
			     command(label:   'Information ...'
				     action:  self # infoOptions
				     feature: info)
			     command(label:   'Drawing ...'
				     action:  self # layoutOptions
				     feature: layout)
			     command(label:   'Postscript ...'
				     action:  self # postscriptOptions
				     feature:postscript)]
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
				     event:   minus
				     key:     '-'
				     feature: leftMost)
			     command(label:   'Rightmost'
				     action:  self # moveFrom(rightMost)
				     state:   disabled
				     event :  plus
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
				     event:   less
				     key:     '<'
				     feature: prevSol)
			     command(label:   'Next Solution'
				     action:  self # moveFrom(nextSol)
				     state:   disabled
				     event:   greater
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
				     key:     '='
				     event:   '<KeyPress-equal>'
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
			     command(label:   'Unhide All'
				     action:  self # nodes(unhideTree)
				     state:   disabled
				     key:     u
				     feature: all)
			     command(label:   'Hide Failed Subtrees'
				     action:  self # nodes(hideFailed)
				     state:   disabled
				     key:     f
				     feature: failed)
			    ]
		      feature: hide)]
	  nil}
	 self.infoAction = {New Actions init(menu:Menu.nodes.infoAction.menu)}
	 self.cmpAction  = {New Actions init(menu:Menu.nodes.cmpAction.menu)}
	 self.statAction = {New Actions init(menu:Menu.nodes.statAction.menu)}
      end
      
      meth clear
	 {DoEntries self.menu [explorer([clear postscript halt reset])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step])
			       nodes([cmp info selCmp deselCmp stat])
			       hide([toggle all failed])]
	  tk(entryconf o(state:disabled))}
      end

      meth busy
	 {DoEntries self.menu [explorer([about postscript])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step])
			       nodes([cmp info selCmp deselCmp stat])
			       hide([toggle all failed])
			       options([search layout info postscript])]
	  tk(entryconf o(state:disabled))}
      end

      meth idle
	 {DoEntries self.menu [explorer([about postscript])
			       options([search layout info postscript])]
	  tk(entryconf o(state:normal))}
      end
      
      meth normal(Es)
	 {DoEntries self.menu Es tk(entryconf o(state:normal))}
      end
   
      meth disable(Es)
	 {DoEntries self.menu Es tk(entryconf o(state:disabled))}
      end
      
      meth state(B Es)
	 case B then {DoEntries self.menu Es tk(entryconf o(state:normal))}
	 else {DoEntries self.menu Es tk(entryconf o(state:disabled))}
	 end
      end

   end

end
