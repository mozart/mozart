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
   Search
   Tk
   TkTools
   Explorer
   Compiler(compile: Compile) at 'Compiler.ozf'
   Configure(text:Helv textBold:HelvBold)
   

export
   'class': Scheduler
   
define

   class Scheduler
      from TkTools.note
      feat
	 Span Reset Next Best Stop Exp
	 Board Solver
      attr
	 Stamp:   0
	 Problem: proc {$ _} skip end

      meth tkInit(parent:P board:B)
	 TkTools.note,tkInit(parent:P text:'Schedule')
	 SpanL = {New Tk.label  tkInit(parent:self text:'Make span:'
				       font:Helv)} 
	 ValL  = {New Tk.label  tkInit(parent:self text:0
				       font:HelvBold
				       width:5 bg:wheat relief:sunken bd:1
				       anchor:e)}
	 ResetB= {New Tk.button tkInit(parent:self text:'Reset' width:9
				       font:Helv
				       action:self#reset)}
	 NextB = {New Tk.button tkInit(parent:self text:'Next'  width:9
				       font:Helv
				       action:self#next
				       state:disabled)}
	 BestB = {New Tk.button tkInit(parent:self text:'Best'  width:9
				       font:Helv
				       action:self#best
				       state:disabled)}
	 StopB = {New Tk.button tkInit(parent:self text:'Stop'  width:9
				       font:Helv
				       action:self#stop
				       state:disabled)}
	 ExpB  = {New Tk.button tkInit(parent:self text:'Search Tree' width:9
				       font:Helv
				       action:self#explore)}
      in
	 {Tk.batch [grid(SpanL  column:0 row:0 pady:6 padx:4)
		    grid(ValL   column:1 row:0 pady:6 padx:4)
		    grid({New Tk.frame  tkInit(parent:self height:5)}
			 column:0 row:1 columnspan:2)
		    grid(ResetB column:0 row:2 columnspan:2)
		    grid(NextB  column:0 row:3 columnspan:2)
		    grid(BestB  column:0 row:4 columnspan:2)
		    grid(StopB  column:0 row:5 columnspan:2)
		    grid({New Tk.frame  tkInit(parent:self height:5)}
			 column:0 row:6 columnspan:2)
		    grid(ExpB   column:0 row:7 columnspan:2 pady:3)
		   ]}
	 self.Board  = B
	 self.Solver = {New Search.object script(proc {$ _} skip end)}
	 self.Span   = ValL
	 self.Reset  = ResetB
	 self.Next   = NextB
	 self.Best   = BestB
	 self.Stop   = StopB
	 self.Exp    = ExpB
      end

      meth setSol(I S)
	 if I==@Stamp then
	    {self.Stop tk(configure state:disabled)}
	    case S of nil then
	       {self.Next tk(configure state:disabled)}
	       {self.Best tk(configure state:disabled)}
	       {self.Span tk(configure bg:cyan4)}
	    [] [S] then
	       {self.Next tk(configure state:normal)}
	       {self.Best tk(configure state:normal)}
	       {self.Board setSol(S)}
	       {self.Span tk(configure text:S.pe)}
	    else
	       {self.Next tk(configure state:normal)}
	       {self.Best tk(configure state:normal)}
	    end
	 end
      end

      meth reset
	 NS = @Stamp+1
      in
	 Stamp <- NS
	 {self.Solver stop}
	 {self.Span   tk(configure text:'' bg:wheat)}
	 {self.Next   tk(configure state:normal)}
	 {self.Best   tk(configure state:normal)}
	 {self.Stop   tk(configure state:disabled)}
	 {self.Board  setEdit}
	 Problem <- {Compile {self.Board getSpec($)}}
	 {self.Solver script(@Problem
			     proc {$ O N}
				O.pe >: N.pe
			     end)}	 
      end

      meth next
	 {self.Next tk(configure state:disabled)}
	 {self.Best tk(configure state:disabled)}
	 {self.Stop tk(configure state:normal)}
	 {self setSol(@Stamp {self.Solver next($)})}
      end

      meth SearchBest(S)
	 case {self.Solver next($)}
	 of [T] then
	    {self.Span tk(configure text:T.pe)}
	    {self.Board setSol(T)}
	    {self SearchBest(S)}
	 elseof T then
	    {self setSol(S T)}
	 end
      end
      
      meth best
	 {self.Next tk(configure state:disabled)}
	 {self.Best tk(configure state:disabled)}
	 {self.Stop tk(configure state:normal)}
	 {self SearchBest(@Stamp)}
      end

      meth stop
	 {self.Solver stop}
      end

      meth explore
	 {Explorer.object all(@Problem proc {$ O N}
					  O.pe >: N.pe
				       end)}
      end

      meth setSpan(S)
	 {self.Span tk(configure text:S.pe)}
      end
      
      meth toTop
	 Scheduler, reset
      end
      
   end

end
