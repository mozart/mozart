%%%
%%% Authors:
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Christian Schulte, 1997
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

functor

import
   Application(exit getCmdArgs)
   Explorer(object)

   Tk(frame toplevel batch addXScrollbar addYScrollbar scrollbar)
   TkTools(notebook)
   
   TaskBoard('class')
   Scheduler('class')
   Tools('class')
   Configure(text)
   
require
   Examples
   
prepare
   ArgSpec = record(example(single type:atom default:no))
   {Wait Examples}
      
define

   Args = {Application.getCmdArgs ArgSpec}

   class Frontend
      from Tk.frame
      feat Board Sched
	 
      meth tkInit(parent:P spec:Spec)
	 Tk.frame, tkInit(parent:P)
	 NB = {New TkTools.notebook  tkInit(parent:self font:Configure.text)}
	 T  = {New Tools.'class'     tkInit(parent:NB board:B)}
	 S  = {New Scheduler.'class' tkInit(parent:NB board:B)}
	 F  = {New Tk.frame          tkInit(parent:self)}
	 B  = {New TaskBoard.'class' tkInit(parent:F tools:T spec:Spec)}
	 H  = {New Tk.scrollbar      tkInit(parent:F orient:horizontal
					    width:13)}
	 V  = {New Tk.scrollbar      tkInit(parent:F orient:vertical
					    width:13)}
      in
	 {Tk.addXScrollbar B H}
	 {Tk.addYScrollbar B V}
	 {Tk.batch [grid(columnconfigure F    0 weight:1)
		    grid(rowconfigure    F    0 weight:1)
		    grid(B row:0 column:0 sticky:nsew)
		    grid(H row:1 column:0 sticky:we)
		    grid(V row:0 column:1 sticky:ns)
		    grid(columnconfigure self 0 weight:1)
		    grid(rowconfigure    self 0 weight:1)
		    grid(F  row:0 column:1 sticky:nsew padx:4 pady:4)
		    grid(NB row:0 column:0 sticky:n    padx:4 pady:4)]}
	 {NB add(T)} {NB add(S)}
	 self.Board = B
	 self.Sched = S
      end
      
      meth displaySol(Sol)
	 if {Record.all Sol IsDet} then
	    {self.Board displaySol(Sol)}
	    {self.Sched setSpan(Sol)}
	 end
      end
      
   end
   
   Top = {New Tk.toplevel tkInit(title:    'Job Shop Scheduler'
				 delete:   Application.exit # 0
				 withdraw: true)}
   
   JSS = {New Frontend
	  tkInit(parent: Top
		 spec:   if {HasFeature Examples Args.example} then
			    Examples.(Args.example)
			 else nil
			 end)}
   
   {Explorer.object add(information
			proc {$ N X}
			   {JSS displaySol(X)}
			end
			label: 'Display Job Shop Schedule')}
   
   
   {Tk.batch [pack(JSS)
	      update
	      wm(deiconify Top)
	      wm(resizable Top false false)]}
	      
   
end


 




