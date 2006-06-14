%%%
%%% Authors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Joerg Wuertz <wuertz@de.ibm.com>
%%%
%%% Copyright:
%%%   Tobias Mueller, 1998
%%%   Joerg Wuert, 1997
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

require

   DemoUrls(image) at '../DemoUrls.ozf' 

prepare
   ImageNames = [DemoUrls.image#'college/title.xbm']

import

   Tk
   TkTools
   Application

   Scheduler(timeTable: TimeTable)
   
export

   Controller

   TopWindow

define

   TopWindow = {New Tk.toplevel tkInit(title:'College Time-Tabling'
				       delete: proc{$}
						  {TopWindow tkClose}
						  {Application.exit 0}
					       end)}
   
   
   Images = {TkTools.images ImageNames}

   fun {Controller}
      
      Menu = {New Tk.frame
	      tkInit(parent:TopWindow relief:raised borderwidth:2)}
      ControllerLabel = {New Tk.label tkInit(parent: Menu text: "")}
      
      {Tk.send pack(Menu side:top fill:x)}
      
      MB1 MB2 MB3
      M1 M2 M3
      AAbout AExit
      
      ALoadSolution
      
\ifndef ALONEDEMO
      ASaveSolution 
      AEditProblem
      ALoadProblem
\endif
   
      AFirstSol AOptimize
      AIntrOpt ADispGraphical ADispInspecting
      ADispTextual
   
      fun {TkAction What Parent P1 P2}
	 {New Tk.action tkInit(parent: Parent
			       action: proc {$}
					  if P2 == none then
					     {What P1}
					  else
					     {What P1 P2}
					  end
				       end)}
      end

      proc {DoFileOp MessageName Flag}
	 File
      in
	 if Flag == read then
	    {Tk.return tk_getOpenFile(title: 'Select file'
				      filetypes:  q(q('Data files' q('.ozt'))
						    q('Oz Files  ' q('.oz'))))
	     File}
	 else
	    {Tk.return tk_getSaveFile(title: 'Select file'
				      filetypes:  q(q('Data files' q('.ozt'))
						    q('Oz Files  ' q('.oz'))))
	     File}
	 end
      
	 {Wait File}

	 if File == nil then skip
	 else       
	    {ControllerLabel tk(configure text:"File "#Flag)}
	    {TimeTable MessageName(File)}
	 end
      end

\ifdef ALONEDEMO  
      proc {DoFileOpAlone MessageName Flag}
	 {TimeTable MessageName(_)}
      end
\endif
   
   % -------------------------------------------------
   
      proc {About}
	 AboutWindow = {New Tk.toplevel tkInit(parent:TopWindow)}
	 OK = {New Tk.button
	       tkInit(parent:AboutWindow text:'OK' relief:raised
		      borderwidth:2
		      action: AboutWindow # tkClose)}
	 Message = {New Tk.message
		    tkInit(parent:AboutWindow
			   aspect:300 
			   relief:raised
			   borderwidth:1
			   font:'-misc-fixed-bold-*-*-*-*-*-*-*-*-*-*'
			   text:
			      "This propgram allows to solve a time tabling problem. When started, a problem is already read in. Hence, just start solving. The search may be interrupted (if a solution was already computed) and resumed.\n")}
      in
	 {Tk.batch [wm(title AboutWindow "What is it?")
		    wm(iconname AboutWindow "About")
		    pack(Message side:top)
		    pack(OK side:bottom fill:x)]}
      end

      % -------------------------------------------------

      {Tk.send pack(ControllerLabel side: right)}

      proc {Compute Message Display}
	 {ControllerLabel tk(configure text: "Computing...")}
	 {TimeTable Message} 
	 thread
	    {Wait {TimeTable get($)}}
	    {ControllerLabel tk(configure text: "done")}
	    {TimeTable Display}
	 end
      end
      
      % -------------------------------------------------

      TitleCanvas = {New Tk.canvas tkInit(parent: TopWindow
					  height: 130 width: 340)}
      {TitleCanvas tk(crea image 0 0
		      image: Images.'title'
		      anchor:nw)}
      {TitleCanvas tk(crea text 240 40
		      text:"TimeTable" anchor:nw
		      font:'-adobe-helvetica-bold-o-*-*-18-*')}
      {TitleCanvas tk(crea text 240 65
		      text:"Manager" anchor:nw
		      font:'-adobe-helvetica-bold-o-*-*-18-*')}
      
      {Tk.send pack(TitleCanvas side:top)}

   in

      {ForAll [ [MB1 " File"] [MB2 " Compute"] [MB3 " Output"] ]
       proc {$ MB}
	  MB.1 = {New Tk.menubutton tkInit(parent:Menu text:MB.2.1)}
       end}
      
      {ForAll [ [M1 MB1] [M2 MB2] [M3 MB3] ]
       proc {$ M} M.1 = {New Tk.menu tkInit(parent:M.2.1)} end}

      AAbout = {New Tk.action tkInit(parent: Menu action: About)}
      AExit = {New Tk.action tkInit(parent: Menu
				    action: proc{$}
					       {TopWindow tkClose}
					       {Application.exit 0}
					    end)}
      
      {ForAll [
\ifndef ALONEDEMO
	       [ALoadProblem DoFileOp readProblem read]
	       [AEditProblem DoFileOp edit read]
	       [ALoadSolution DoFileOp read read]
	       [ASaveSolution DoFileOp save written]
\else
	       [_             DoFileOp readProblem read]
	       [_             DoFileOp edit read]
	       [ALoadSolution DoFileOpAlone read read]
\endif
	       [AFirstSol Compute solveProblem graphic]
	       [AOptimize TimeTable optimizeProblem none]
	       [AIntrOpt TimeTable anyTime none]
	       [ADispGraphical TimeTable graphic none]
	       [ADispInspecting TimeTable inspect none]
	       [ADispTextual TimeTable text none]
	      ]
       proc {$ Action}
	  Action.1 = {TkAction Action.2.1 Menu Action.2.2.1 Action.2.2.2.1}
       end}
   
\ifndef ALONEDEMO
      {ForAll [ ["  Load problem " ALoadProblem]
		["  Edit problem " AEditProblem]
		["--------------"]
		["  Load solution " ALoadSolution]
		["  Save Solution " ASaveSolution]
		["--------------"]
		["  Exit " AExit] ]
\else
       {ForAll [ ["  Load solution " ALoadSolution]
		 ["--------------"]
		 ["  About this program " AAbout]
		 ["--------------"]
		 ["  Exit " AExit] ]
\endif       
	proc{$ C}
	   case C.1
	   of "--------------" then {M1 tk(add sep)}
	   else {M1 tk(add command label: C.1 command: C.2.1)}
	   end
	end}
       {ForAll [ ["  First Solution " AFirstSol]
		 ["  Optimize Current Solution " AOptimize]
		 ["--------------"]
		 ["  Interrupt Solving " AIntrOpt] ]
	proc{$ C}
	   case C.1
	   of "--------------" then {M2 tk(add(sep))}
	   else {M2 tk(add(command(label: C.1 command: C.2.1)))}
	   end
	end}
       {ForAll [ ["  Display graphically " ADispGraphical]
		 ["  Display with Inspector " ADispInspecting]
		 ["  Display textually " ADispTextual]
	       ]
	proc{$ C}
	   case C.1
	   of "--------------" then {M3 tk(add(sep))}
	   else {M3 tk(add command label: C.1 command: C.2.1)}
	   end
	end}
      
       {MB1 tk(conf menu: M1)}
       {MB2 tk(conf menu: M2)}
       {MB3 tk(conf menu: M3)}

       {Tk.send pack(MB1 MB2 MB3 side: left)}
       {Tk.send tk_menuBar(Menu MB1 MB2 MB3)}

       ControllerLabel      
   end
      
end
