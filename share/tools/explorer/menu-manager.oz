%%%  Programming Systems Lab, DFKI Saarbruecken,
%%%  Stuhlsatzenhausweg 3, D-66123 Saarbruecken, Phone (+49) 681 302-5312
%%%  Author: Christian Schulte
%%%  Email: schulte@dfki.uni-sb.de
%%%  Last modified: $Date$ by $Author$
%%%  Version: $Revision$

local

   proc {DoEntries M Es W}
      case Es of nil then true
      [] E|Er then {DoEntries M E W} {DoEntries M Er W}
      elsecase {IsAtom Es} then {M.Es W}
      else {DoEntries M.{Label Es} Es.1 W}
      end
   end

in

   class MenuManager
      feat menu

      meth init
	 self.menu =
      {Menues.make self.toplevel
       [explorer(label: 'Explorer'
		 menu:
		    [about(command(label:  'About'
				   action: self#about))
		     separator
		     break(command(label:  'Break'
				   action: proc {$}
					      {self.status break}
					   end
				   key:    'b'
				   ctrl:   True))
		     separator
		     reset(command(label:  'Reset'
				   action: proc {$}
					      {self.status kill}
					      {self reset}
					   end
				   normal: False
				   key:    'r'
				   ctrl:   True))
		     clear(command(label:  'Clear'
				   action: proc {$}
					      {self.status kill}
					      {self clear}
					   end
				   normal: False))
		     separator
		     postscript(command(label:  'Export Postscript'
					action: self#postscript
					normal: False))
		     separator
		     quit(command(label:  'Quit'
				  action: proc {$}
					     {self.status kill}
					     {self close}
					  end))
		    ])
	options(label:'Options'
		menu:
		   [search(command(label:  'Search ...'
				   action: self#searchOptions))
		    info(command(label:  'Information ...'
				 action: self#infoOptions))
		    layout(command(label:  'Drawing ...'
				   action: self#layoutOptions))
		    postscript(command(label:  'Postscript ...'
				       action: self#postscriptOptions))])
	move(label:'Move'
	     menu:
		[cur(command(label:  'Current Node'
			     action: self#moveCurrent
			     normal: False
			     key:    'c'))
		 top(command(label:  'Top Node'
			     action: self#moveTop
			     normal: False
			     key:    't'))
		 separator
		 leftMost(command(label:  'Leftmost'
				  action: self#moveFrom(leftMost)
				  normal: False
				  key:    '-'))
		 rightMost(command(label:  'Rightmost'
				  action: self#moveFrom(rightMost)
				  normal: False
				  key:    '+'))
		 separator
		 back(command(label:  'Backtrack'
			      action: self#moveFrom(back)
			      normal: False
			      key:    'b'))
		 separator
		 prevSol(command(label:  'Previous Solution'
				 action:  self#moveFrom(prevSol)
				 normal:  False
				 key:    '<'))
		 nextSol(command(label:  'Next Solution'
				 action: self#moveFrom(nextSol)
				 normal: False
				 key:    '>'))
		])
	search(label:'Search'
	       menu: [next(command(label:  'Next Solution'
				   action: self#next
				   normal: False
				   key:    'n'))
		      all(command(label:  'All Solutions'
				  action: self#all
				  normal: False
				  key:    'a'))
		      separator
		      step(command(label:  'One Step'
				   action: self#step
				   normal: False
				   key:    's'))
		      separator
		      halt(command(label:  'Halt'
				   action: proc {$}
					      {self.status halt}
					   end
				   normal: False
				   key:    'h'))
		      separator
		      depth(number(label:    'Maximal Depth'
				   numbers:  DefDepthNumbers))
		      nodes(number(label:    'Maximal Nodes'
				   numbers:  DefNodesNumbers))
		     ])
	nodes(label:'Nodes'
	      menu:
		 [chooseInfo(cascade(label:'Information Action'
				     menu:[group(group(menu:nil))]))
		  info(command(label:  'Information'
			       action: self#nodesInfo
			       normal: False
			       key:    'i'
			       ctrl:   True))
		  separator
		  chooseCmp(cascade(label:'Compare Action'
				     menu:[group(group(menu:nil))]))
		  selCmp(command(label:  'Select Compare'
				 action: self#nodesSelCmp
				 normal: False
				 key:    'c'
				 ctrl:   True))
		  deselCmp(command(label:  'Deselect Compare'
				   action: self#nodesDeselCmp
				   normal: False
				   key:    'd'
				   ctrl:   True))
		  cmp(command(label:  'Compare'
			      action: self#nodesCmp
			      normal: False
			      key:    'e'
			      ctrl:   True))
		  separator
		  chooseStat(cascade(label:'Statistics Action'
				     menu:[group(group(menu:nil))]))
		  stat(command(label:  'Statistics'
			       action: self#stat
			       normal: False
			       key:    's'
			       ctrl:   True))
		  separator
		  clear(command(label:  'Clear Numbers'
				action: self#clearNumbers
				normal: False
				key:    'n'
				ctrl:   True))
		  separator
		  hide(command(label:  'Hide/Unhide'
			       action: self#nodes(hide)
			       normal: False
			       key:    'h'
			       ctrl:   True))
		  hideFailed(command(label:  'Hide Failed Subtrees'
				     action: self#nodes(hideFailed)
				     normal: False
				     key:    'f'
				     ctrl:   True))
		  unhide(command(label:  'Unhide Subtrees'
				 action: self#nodes(unhideTree)
				 normal: False
				 key:    'u'
				 ctrl:   True))
		 ])
       ]}
      end
      
      meth clear
	 {DoEntries self.menu [explorer([clear postscript reset])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step halt])
			       nodes([cmp hide hideFailed info selCmp
				      deselCmp stat clear unhide])] disable}
      end

      meth busy
	 {DoEntries self.menu [explorer([about postscript])
			       move([cur leftMost rightMost
				     nextSol prevSol top back])
			       search([all next step halt])
			       nodes([cmp hide hideFailed info selCmp
				      deselCmp stat clear unhide])
			       options([search layout info postscript])]
	  disable}
      end

      meth idle
	 {DoEntries self.menu [explorer([about postscript])
			       options([search layout info postscript])]
	  normal}
      end
      
      meth normal(Es)
	 {DoEntries self.menu Es normal}
      end
   
      meth disable(Es)
	 {DoEntries self.menu Es disable}
      end
      
      meth state(B Es)
	 case B then {DoEntries self.menu Es normal}
	 else {DoEntries self.menu Es normal}
	 end
      end

   end

end
